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
					ASC->TryActivateAbilityByClass(nextComboInfo.NextAbilityClass);
					return;
				}
			}
		}
	}
}


