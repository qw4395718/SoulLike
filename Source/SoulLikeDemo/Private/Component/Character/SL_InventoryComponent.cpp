// Private/Component/Character/SL_InventoryComponent.cpp
#include "SL_InventoryComponent.h"
#include "SL_CharacterBase.h"
#include "SL_AbilitySystemComponent.h"
#include "SL_GameplayAbilityUseItem.h"
#include "GlobalDelegatesManager.h"
#include "DataTableManager.h"
#include "ItemDataTable.h"

USL_InventoryComponent::USL_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAbilityGranted = false;
}

/************************************************************************/
/*                               继承实现                                */
/************************************************************************/

void USL_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 延迟一帧初始化（确保所有子系统已就绪）
	FTimerHandle DummyHandle;
	GetWorld()->GetTimerManager()->SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		InitializeInventory();
	}));
}

void USL_InventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理委托绑定
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		if (OnItemUsedHandle.IsValid())
		{
			DelegateMgr->OnItemUsed.Remove(OnItemUsedHandle);
			OnItemUsedHandle.Reset();
		}
	}

	Super::EndPlay(EndPlayReason);
}

/************************************************************************/
/*                               外部调用                                */
/************************************************************************/

void USL_InventoryComponent::InitializeInventory()
{
	// 1. 授予通用道具使用能力（角色一生只做一次）
	if (!bAbilityGranted)
	{
		GrantUseItemAbility();
	}

	// 2. 绑定道具使用事件（监听从GAS能力广播回来的事件）
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		if (!OnItemUsedHandle.IsValid())
		{
			OnItemUsedHandle = DelegateMgr->OnItemUsed.AddUObject(
				this, &USL_InventoryComponent::OnItemUsedCallback);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("USL_InventoryComponent::InitializeInventory - Initialized"));
}

bool USL_InventoryComponent::AddItemByID(FName InItemID, int32 InCount)
{
	if (InItemID.IsNone() || InCount <= 0) return false;

	// 检查道具是否存在（通过DataTable验证）
	UDataTableManager* TableManager = UDataTableManager::Get(this);
	if (!TableManager) return false;

	UItemDataTable* ItemTable = Cast<UItemDataTable>(
		TableManager->GetDataTable(EDataTableType::DT_ItemConfigInfo));
	if (!ItemTable) return false;

	FItemDataRow ItemData;
	if (!ItemTable->GetItemData(InItemID, ItemData))
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_InventoryComponent::AddItemByID - ItemID %s not found in table"), *InItemID.ToString());
		return false;
	}

	// 添加到库存
	int32& CurrentCount = ItemInventory.FindOrAdd(InItemID);
	CurrentCount += InCount;

	// 广播数量变更
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		DelegateMgr->BroadcastItemCountChanged(GetOwner(), InItemID, CurrentCount);
	}

	UE_LOG(LogTemp, Log, TEXT("USL_InventoryComponent::AddItemByID - Added %s x%d (total: %d)"),
		*InItemID.ToString(), InCount, CurrentCount);
	return true;
}

bool USL_InventoryComponent::RemoveItemByID(FName InItemID, int32 InCount)
{
	if (InItemID.IsNone() || InCount <= 0) return false;

	int32* CurrentCount = ItemInventory.Find(InItemID);
	if (!CurrentCount || *CurrentCount < InCount) return false;

	*CurrentCount -= InCount;

	if (*CurrentCount <= 0)
	{
		// 数量归零，移除条目
		ItemInventory.Remove(InItemID);

		// 如果是选中的道具，清空选择
		if (SelectedItemID == InItemID)
		{
			SelectedItemID = NAME_None;
		}
	}

	// 广播数量变更
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		DelegateMgr->BroadcastItemCountChanged(GetOwner(), InItemID, FMath::Max(0, *CurrentCount));
	}

	UE_LOG(LogTemp, Verbose, TEXT("USL_InventoryComponent::RemoveItemByID - Removed %s x%d"), *InItemID.ToString(), InCount);
	return true;
}

int32 USL_InventoryComponent::GetItemCount(FName InItemID) const
{
	if (const int32* Found = ItemInventory.Find(InItemID))
	{
		return *Found;
	}
	return 0;
}

TArray<FName> USL_InventoryComponent::GetAllItemIDs() const
{
	TArray<FName> IDs;
	ItemInventory.GetKeys(IDs);
	return IDs;
}

bool USL_InventoryComponent::CanUseItem(FName InItemID) const
{
	// 1. 检查道具是否存在且数量 > 0
	if (GetItemCount(InItemID) <= 0) return false;

	// 2. 检查玩家是否存活
	const ASL_CharacterBase* Character = GetOwningCharacter();
	if (!Character || !Character->IsAlive()) return false;

	// 3. 检查道具在数据表中是否标记为可用
	UDataTableManager* TableManager = UDataTableManager::Get(
		const_cast<USL_InventoryComponent*>(this));
	if (!TableManager) return false;

	UItemDataTable* ItemTable = Cast<UItemDataTable>(
		TableManager->GetDataTable(EDataTableType::DT_ItemConfigInfo));
	if (!ItemTable) return false;

	FItemDataRow ItemData;
	if (!ItemTable->GetItemData(InItemID, ItemData)) return false;

	// 4. 道具必须标记为可用
	return ItemData.bCanUse;
}

bool USL_InventoryComponent::UseItemByID(FName InItemID)
{
	if (!CanUseItem(InItemID)) return false;

	// 激活通用道具使用能力，传递道具ID
	return ActivateItemAbility(InItemID);
}

bool USL_InventoryComponent::UseSelectedItem()
{
	if (SelectedItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_InventoryComponent::UseSelectedItem - No item selected"));
		return false;
	}

	return UseItemByID(SelectedItemID);
}

/************************************************************************/
/*                               内部调用                                */
/************************************************************************/

void USL_InventoryComponent::GrantUseItemAbility()
{
	if (bAbilityGranted) return;

	USL_AbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_InventoryComponent::GrantUseItemAbility - ASC not found"));
		return;
	}

	// 检查是否已配置能力类
	if (!UseItemAbilityClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_InventoryComponent::GrantUseItemAbility - UseItemAbilityClass is not set, using default"));

		// 尝试从默认路径加载
		UseItemAbilityClass = USL_GameplayAbilityUseItem::StaticClass();
		if (!UseItemAbilityClass)
		{
			UE_LOG(LogTemp, Error, TEXT("USL_InventoryComponent::GrantUseItemAbility - Failed to get default ability class"));
			return;
		}
	}

	// 授予通用道具使用能力
	FGameplayAbilitySpec Spec(UseItemAbilityClass, 1, INDEX_NONE, this);
	ASC->GiveAbility(Spec);

	bAbilityGranted = true;

	UE_LOG(LogTemp, Log, TEXT("USL_InventoryComponent::GrantUseItemAbility - Granted GA_UseItem to %s"), *GetOwner()->GetName());
}

bool USL_InventoryComponent::ActivateItemAbility(FName InItemID)
{
	USL_AbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return false;

	// 查表获取道具的GameplayTag
	UDataTableManager* TableManager = UDataTableManager::Get(this);
	if (!TableManager) return false;

	UItemDataTable* ItemTable = Cast<UItemDataTable>(
		TableManager->GetDataTable(EDataTableType::DT_ItemConfigInfo));
	if (!ItemTable) return false;

	FItemDataRow ItemData;
	if (!ItemTable->GetItemData(InItemID, ItemData)) return false;

	if (!ItemData.UseAbilityTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_InventoryComponent::ActivateItemAbility - Item %s has no UseAbilityTag"), *InItemID.ToString());
		return false;
	}

	// 查找通用道具使用能力
	FGameplayAbilitySpec* FoundSpec = ASC->FindAbilitySpecFromTag(ItemData.UseAbilityTag);
	if (!FoundSpec)
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_InventoryComponent::ActivateItemAbility - UseItem ability not found, try to activate by class"));

		// 通过能力类查找
		if (UseItemAbilityClass)
		{
			FoundSpec = ASC->FindAbilitySpecFromClass(UseItemAbilityClass);
		}

		if (!FoundSpec)
		{
			UE_LOG(LogTemp, Error, TEXT("USL_InventoryComponent::ActivateItemAbility - Failed to find any use item ability"));
			return false;
		}
	}

	// 通过事件数据传递道具ID
	FGameplayEventData EventData;
	EventData.Instigator = GetOwner();
	EventData.Target = GetOwner();
	EventData.EventTag = ItemData.UseAbilityTag;
	EventData.OptionalObject = nullptr;
	EventData.ContextString = InItemID.ToString();

	// 通过Event激活能力
	bool bActivated = ASC->TriggerAbilityFromGameplayEvent(
		FoundSpec->Handle,
		ASC->AbilityActorInfo.Get(),
		ItemData.UseAbilityTag,
		&EventData,
		*ASC
	);

	if (bActivated)
	{
		UE_LOG(LogTemp, Log, TEXT("USL_InventoryComponent::ActivateItemAbility - Activated use item ability for %s"), *InItemID.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_InventoryComponent::ActivateItemAbility - Failed to activate ability for %s"), *InItemID.ToString());
	}

	return bActivated;
}

void USL_InventoryComponent::OnItemUsedCallback(AActor* InUserActor, FName InItemID)
{
	// 检查是否是自己用的道具
	if (InUserActor != GetOwner()) return;

	// 消耗道具数量
	RemoveItemByID(InItemID, 1);
}

ASL_CharacterBase* USL_InventoryComponent::GetOwningCharacter() const
{
	return Cast<ASL_CharacterBase>(GetOwner());
}

USL_AbilitySystemComponent* USL_InventoryComponent::GetAbilitySystemComponent() const
{
	ASL_CharacterBase* Character = GetOwningCharacter();
	if (!Character) return nullptr;

	return Cast<USL_AbilitySystemComponent>(Character->GetAbilitySystemComponent());
}