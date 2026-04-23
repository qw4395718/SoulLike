// Fill out your copyright notice in the Description page of Project Settings.


#include <SL_ComboManagerComponent.h>
#include <AbilitySystemComponent.h>
#include <AbilitySystemInterface.h>
#include <GameplayTagContainer.h>

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
		if (!ASC)
			return;
		ASC->GetOwnedGameplayTags(currentTags);

		// 遍历ComboInfoMap
		for (const auto& pair : ComboInfoMap)
		{
			if (currentTags.HasTag(pair.Key))
			{
				const auto& subPair = pair.Value;
				if (subPair.Contains(InputType))
				{
					// 执行对应的GA
					ASC->TryActivateAbilityByClass(subPair[InputType].NextAbilityClass);
					return;
				}
			}
		}
	}
}


