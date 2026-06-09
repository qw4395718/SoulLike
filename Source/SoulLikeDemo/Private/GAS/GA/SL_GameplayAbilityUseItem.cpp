// Private/GAS/GA/SL_GameplayAbilityUseItem.cpp
#include "SL_GameplayAbilityUseItem.h"
#include "SL_AbilitySystemComponent.h"
#include "SL_CharacterBase.h"
#include "GlobalDelegatesManager.h"
#include "DataTableManager.h"
#include "ItemDataTable.h"
#include <SL_InventoryComponent.h>
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

USL_GameplayAbilityUseItem::USL_GameplayAbilityUseItem()
{
	// 默认需要在服务器上执行
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

/************************************************************************/
/*                               继承实现                                */
/************************************************************************/

void USL_GameplayAbilityUseItem::ActivateAbility(const FGameplayAbilitySpecHandle InHandle,
	const FGameplayAbilityActorInfo* InActorInfo,
	const FGameplayAbilityActivationInfo InActivationInfo,
	const FGameplayEventData* InTriggerEventData)
{
	if (!CommitAbility(InHandle, InActorInfo, InActivationInfo))
	{
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, false);
		return;
	}

	CachedActorInfo = InActorInfo;

	// 1. 从持有者的背包组件获取当前选中的道具ID
	FName ItemID = GetItemIDFromInventory(InActorInfo);
	if (ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_GameplayAbilityUseItem::ActivateAbility - Failed to parse ItemID from event data"));
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, false);
		return;
	}

	CurrentItemID = ItemID;

	// 2. 查表获取道具配置
	UDataTableManager* TableManager = UDataTableManager::Get(InActorInfo->OwnerActor.Get());
	if (!TableManager)
	{
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, false);
		return;
	}

	UItemDataTable* ItemTable = Cast<UItemDataTable>(
		TableManager->GetDataTable(EDataTableType::DT_ItemConfigInfo));
	if (!ItemTable)
	{
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, false);
		return;
	}

	FItemDataRow ItemData;
	if (!ItemTable->GetItemData(ItemID, ItemData))
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_GameplayAbilityUseItem::ActivateAbility - Item %s not found in table"), *ItemID.ToString());
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, false);
		return;
	}

	// 3. 广播道具使用事件（InventoryComponent会监听这个来消耗道具）
	BroadcastItemUsed(InActorInfo, ItemID);

	// 4. 播放使用道具蒙太奇
	if (UseItemMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this,
				NAME_None,
				UseItemMontage,
				1.0f,
				NAME_None,
				false
			);

		if (MontageTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &USL_GameplayAbilityUseItem::OnUseMontageCompleted);
			MontageTask->OnInterrupted.AddDynamic(this, &USL_GameplayAbilityUseItem::OnUseMontageInterrupted);
			MontageTask->ReadyForActivation();
		}
	}
	else
	{
		// 没有蒙太奇，直接应用效果
		ApplyItemEffectByID(InActorInfo, ItemID);
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, false);
	}
}

/************************************************************************/
/*                               内部调用                                */
/************************************************************************/

FName USL_GameplayAbilityUseItem::GetItemIDFromInventory(const FGameplayAbilityActorInfo* InActorInfo) const
{
	if (!InActorInfo || !InActorInfo->OwnerActor.IsValid()) return NAME_None;

	AActor* OwnerActor = InActorInfo->OwnerActor.Get();
	USL_InventoryComponent* Inventory = Cast<USL_InventoryComponent>(
		OwnerActor->GetComponentByClass(USL_InventoryComponent::StaticClass()));

	if (Inventory)
	{
		FName SelectedID = Inventory->GetSelectedItemID();
		if (!SelectedID.IsNone())
		{
			return SelectedID;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("USL_GameplayAbilityUseItem::GetItemIDFromInventory - No selected item in inventory"));
	return NAME_None;
}

void USL_GameplayAbilityUseItem::ApplyItemEffectByID(const FGameplayAbilityActorInfo* InActorInfo, FName InItemID)
{
	UDataTableManager* TableManager = UDataTableManager::Get(InActorInfo->OwnerActor.Get());
	if (!TableManager) return;

	UItemDataTable* ItemTable = Cast<UItemDataTable>(
		TableManager->GetDataTable(EDataTableType::DT_ItemConfigInfo));
	if (!ItemTable) return;

	FItemDataRow ItemData;
	if (!ItemTable->GetItemData(InItemID, ItemData)) return;

	ApplyItemEffects(InActorInfo, ItemData);
}

void USL_GameplayAbilityUseItem::ApplyItemEffects(const FGameplayAbilityActorInfo* InActorInfo, const FItemDataRow& InItemData)
{
	UAbilitySystemComponent* ASC = InActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return;

	// 1. 应用对自己（使用者）的效果
	if (!InItemData.SelfEffectClass.IsNull())
	{
		TSubclassOf<UGameplayEffect> SelfEffectClass = InItemData.SelfEffectClass.LoadSynchronous();
		if (SelfEffectClass)
		{
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SelfEffectClass, 1, ASC->MakeEffectContext());
			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
				UE_LOG(LogTemp, Verbose, TEXT("USL_GameplayAbilityUseItem::ApplyItemEffects - Applied self effect for item %s"), *InItemData.ItemID.ToString());
			}
		}
	}

	// 2. 广播道具效果触发事件
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(InActorInfo->OwnerActor.Get()))
	{
		DelegateMgr->BroadcastItemEffectTriggered(InActorInfo->OwnerActor.Get(), InItemData.ItemID, InActorInfo->OwnerActor.Get());
	}
}

void USL_GameplayAbilityUseItem::BroadcastItemUsed(const FGameplayAbilityActorInfo* InActorInfo, FName InItemID)
{
	if (!InActorInfo) return;

	AActor* OwnerActor = InActorInfo->OwnerActor.Get();
	if (!OwnerActor) return;

	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(OwnerActor))
	{
		DelegateMgr->BroadcastItemUsed(OwnerActor, InItemID);

		UE_LOG(LogTemp, Log, TEXT("USL_GameplayAbilityUseItem::BroadcastItemUsed - Item %s used by %s"),
			*InItemID.ToString(), *OwnerActor->GetName());
	}
}

void USL_GameplayAbilityUseItem::OnUseMontageCompleted()
{
	// 蒙太奇播放完成，应用效果
	ApplyItemEffectByID(CachedActorInfo, CurrentItemID);

	// 结束能力
	EndAbility(CurrentHandle, CachedActorInfo, CurrentActivationInfo, true, false);
}

void USL_GameplayAbilityUseItem::OnUseMontageInterrupted()
{
	// 蒙太奇被打断，取消能力
	EndAbility(CurrentHandle, CachedActorInfo, CurrentActivationInfo, true, true);
}