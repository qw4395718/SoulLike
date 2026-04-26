// Fill out your copyright notice in the Description page of Project Settings.


#include <SL_ComboManagerComponent.h>
#include <AbilitySystemComponent.h>
#include <AbilitySystemInterface.h>
#include <GameplayTagContainer.h>
#include <DataTableManager.h>
#include <ComboInfoTable.h>

USL_ComboManagerComponent::USL_ComboManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void USL_ComboManagerComponent::HandleInputPressed(EComboInputActionType InputType)
{
	// 获取当前角色的激活窗口状态(连击激活窗口状态同一时间只会有一个,并且永远会有一个)
	FGameplayTagContainer currentTags;
	if (IAbilitySystemInterface* ASC_IF = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		// 从接口获取目标的 AbilitySystemComponent
		UAbilitySystemComponent* ASC = ASC_IF->GetAbilitySystemComponent();
		RETURN_IF_TRUE(ASC == nullptr);

		ASC->GetOwnedGameplayTags(currentTags);

		if (UDataTableManager* tableManager = UDataTableManager::Get(this))
		{
			if (UComboInfoTable* comboInfoTable = Cast<UComboInfoTable>(tableManager->GetDataTable(EDataTableType::DT_ComboInfo)))
			{
				FComboInfo nextComboInfo{};
				if (comboInfoTable->FindNextComboInfo(currentTags, InputType, nextComboInfo))
				{
					// ASC->TryActivateAbilityByClass(nextComboInfo.NextAbilityClass);
					// 不直接激活GA，而是发送一个预定义Event
					SendComboEvent(
						FGameplayTag::RequestGameplayTag(TEXT("Event.ComboSystem.InputReceived")),
						nextComboInfo.NextAbilityClass);
					return;
				}
			}
		}
	}
}

void USL_ComboManagerComponent::SendComboEvent(const FGameplayTag& EventTag, TSubclassOf<UGameplayAbility> NextAbility)
{
	if (IAbilitySystemInterface* ASC_IF = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		// 从接口获取目标的 AbilitySystemComponent
		if(UAbilitySystemComponent* ASC = ASC_IF->GetAbilitySystemComponent())
		{
			FGameplayEventData EventData;
			EventData.Instigator = GetOwner();
			EventData.Target = GetOwner();
			// 用OptionalObject携带下一招的GA类
			EventData.OptionalObject = NextAbility.Get();

			ASC->HandleGameplayEvent(EventTag, &EventData);

			UE_LOG(LogTemp, Verbose, TEXT("[ComboManager] Sent event: %s with NextGA: %s"),
				*EventTag.ToString(),
				*NextAbility->GetName());
		}
	}
}

