// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_ComboManagerComponent.h"
#include <AbilitySystemComponent.h>
#include <AbilitySystemInterface.h>
#include <GameplayTagContainer.h>
#include "DataTableManager.h"
#include "ComboInfoTable.h"
#include <AT/AbilityTask_ComboMontage.h>
#include <SL_StaminaComponent.h>

USL_ComboManagerComponent::USL_ComboManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void USL_ComboManagerComponent::HandleInputPressed(EComboInputActionType InputType)
{
	// 获取当前角色的激活窗口状态(连击激活窗口状态同一时间只会有一个,并且永远会有一个)
	FGameplayTagContainer currentTags;
	if (UAbilitySystemComponent* ASC = GetCachedASC())
	{
		ASC->GetOwnedGameplayTags(currentTags);
		if (UDataTableManager* tableManager = UDataTableManager::Get(this))
		{
			if (UComboInfoTable* comboInfoTable = Cast<UComboInfoTable>(tableManager->GetDataTable(EDataTableType::DT_ComboInfo)))
			{
				
				// 检查是否有任何State.Window.* Tag存在
				const FGameplayTag ComboWindowRoot = FGameplayTag::RequestGameplayTag(TEXT("State.Window"));
				bool bInComboWindow = currentTags.HasTag(ComboWindowRoot);

				if (bInComboWindow)
				{
					if (comboInfoTable->FindNextComboInfo(currentTags, InputType, nextComboInfo))
					{
						// ===== 体力检查：委托给StaminaComponent =====
						if (CachedStaminaComp.IsValid() && !CachedStaminaComp->CanAffordCost(nextComboInfo.StaminaCost))
						{
							UE_LOG(LogTemp, Verbose, TEXT("Stamina: 体力不足，无法继续连击"));
							return;
						}
						
						UAbilityTask_ComboMontage* ComboTask = ActiveComboTask.Get();
						if (ComboTask && IsValid(ComboTask))
						{
							EComboInputHandledResult AcceptType = ActiveComboTask->OnInputReceived(InputType);
							if (AcceptType == EComboInputHandledResult::Accepted)
							{
								// 将行为委托给AT
								ComboTask->OnInterrupted.RemoveAll(this);
								ComboTask->OnInterrupted.AddUniqueDynamic(this, &USL_ComboManagerComponent::OnMontageBlendOut);
							}
							else if (AcceptType == EComboInputHandledResult::AcceptedAndBlended)
							{
								// 已经混合好了,直接执行
								OnMontageBlendOut();
							}
							else
							{}
						}
					}
				}
				else
				{
					// 没有正在执行的TASK,执行初始连段
					FGameplayTagContainer firstAttackTags;
					firstAttackTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Window.None")));

					if (comboInfoTable->FindNextComboInfo(firstAttackTags, InputType, nextComboInfo))
					{
						// 执行初始连段
						ASC->TryActivateAbilityByClass(nextComboInfo.NextAbilityClass);

						// 暂停恢复
						if (USL_StaminaComponent* StaminaComp = GetCachedStaminaComp())
						{
							StaminaComp->OnComboStarted();
							StaminaComp->ConsumeStamina(nextComboInfo.StaminaCost);
						}
						return;
					}
				}
				
			}
				
		}
	}
}

void USL_ComboManagerComponent::OnMontageBlendOut()
{
	if (UAbilitySystemComponent* ASC = GetCachedASC())
	{
		ASC->TryActivateAbilityByClass(nextComboInfo.NextAbilityClass);
	}
	if (USL_StaminaComponent* StaminaComp = GetCachedStaminaComp())
	{
		CachedStaminaComp->ConsumeStamina(nextComboInfo.StaminaCost);
	}
}

void USL_ComboManagerComponent::OnMontageFinished()
{
	if (ActiveComboTask.IsValid())
	{
		ActiveComboTask->OnInterrupted.RemoveAll(this);
	}
	ActiveComboTask.Reset();

	// 通知StaminaComponent：连击结束
	if (USL_StaminaComponent* StaminaComp = GetCachedStaminaComp())
	{
		CachedStaminaComp->OnComboEnded();
	}

}

void USL_ComboManagerComponent::RegisterActiveComboTask(class UAbilityTask_ComboMontage* InTask)
{
	ActiveComboTask = InTask;
}

void USL_ComboManagerComponent::UnregisterActiveComboTask()
{
	ActiveComboTask.Reset();
}

void USL_ComboManagerComponent::SetNeedClearTag(FGameplayTag WindowTag)
{
	oldWindowTag = WindowTag;
	UE_LOG(LogTemp, Warning, TEXT("WindowTag_SetNeedClearTag: %s"), *oldWindowTag.ToString());
}

void USL_ComboManagerComponent::ClearTargetWindowTag()
{
	if (UAbilitySystemComponent* ASC = GetCachedASC())
	{
		ASC->RemoveLooseGameplayTag(oldWindowTag);
		UE_LOG(LogTemp, Warning, TEXT("WindowTag_ClearTargetWindowTag: %s"), *oldWindowTag.ToString());
	}
}

UAbilitySystemComponent* USL_ComboManagerComponent::GetCachedASC() const
{
	// 使用缓存
	if (CachedASC.IsValid())
	{
		return CachedASC.Get();
	}

	// 从当前控制的 Pawn 上查找
	if (IAbilitySystemInterface* ASC_IF = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		// 从接口获取目标的 AbilitySystemComponent
		if (UAbilitySystemComponent* ASC = ASC_IF->GetAbilitySystemComponent())
		{
			CachedASC = ASC;
			return CachedASC.Get();
		}
	}

	return nullptr;

}

USL_StaminaComponent* USL_ComboManagerComponent::GetCachedStaminaComp() const
{
	// 使用缓存
	if (CachedStaminaComp.IsValid())
	{
		return CachedStaminaComp.Get();
	}

	// 从当前控制的 Pawn 上查找

	if (AActor* OwnActor = GetOwner())
	{
		if (USL_StaminaComponent* StaminaComp = OwnActor->FindComponentByClass<USL_StaminaComponent>())
		{
			CachedStaminaComp = StaminaComp;
			return CachedStaminaComp.Get();
		}
		
	}

	return nullptr;
}
