// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_ComboManagerComponent.h"
#include <AbilitySystemComponent.h>
#include <AbilitySystemInterface.h>
#include <GameplayTagContainer.h>
#include "DataTableManager.h"
#include "ComboInfoTable.h"
#include <WeaponAccessory_IF.h>
#include <SL_WeaponBase.h>
#include <SL_WeaponAnimSet.h>
#include <AT/AbilityTask_ComboMontage.h>
#include <SL_StaminaComponent.h>

USL_ComboManagerComponent::USL_ComboManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;  // 仅蓄力时启用

	m_bIsCharging = false;
	m_chargeStartTime = 0.0f;
	m_chargeLevel = 0.0f;
	m_bChargeHoldAbilityActivated = false;
}

void USL_ComboManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_bIsCharging)
	{
		UpdateCharge(DeltaTime);
	}
}

/************************************************************************/
/*                              武器连招表查询                                  */
/************************************************************************/

UComboInfoTable* USL_ComboManagerComponent::GetCurrentWeaponComboTable() const
{
	IWeaponAccessory_IF* WeaponAccessory = Cast<IWeaponAccessory_IF>(GetOwner());
	if (!WeaponAccessory) return nullptr;

	ASL_WeaponBase* Weapon = WeaponAccessory->GetRightHandWeapon();
	USL_WeaponAnimSet* AnimSet = Weapon ? Weapon->GetWeaponAnimSet() : nullptr;
	if (!AnimSet || AnimSet->ComboTableType == EDataTableType::DT_None) return nullptr;

	UDataTableManager* TM = UDataTableManager::Get(this);
	return TM ? Cast<UComboInfoTable>(TM->GetDataTable(AnimSet->ComboTableType)) : nullptr;
}

UAnimMontage* USL_ComboManagerComponent::ResolveCurrentMontage() const
{
	if (!CurrentComboInfo.OutputMontageTag.IsValid())
		return nullptr;

	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	IWeaponAccessory_IF* WeaponAccessory = Cast<IWeaponAccessory_IF>(Owner);
	if (!WeaponAccessory) return nullptr;

	ASL_WeaponBase* Weapon = WeaponAccessory->GetRightHandWeapon();
	if (!Weapon) return nullptr;

	USL_WeaponAnimSet* AnimSet = Weapon->GetWeaponAnimSet();
	if (!AnimSet) return nullptr;

	return AnimSet->GetComboMontageByTag(CurrentComboInfo.OutputMontageTag);
}

/************************************************************************/
/*                              外部调用                                        */
/************************************************************************/

void USL_ComboManagerComponent::HandleInputPressed(EComboInputActionType InputType)
{
	// 获取当前角色的激活窗口状态
	FGameplayTagContainer currentTags;
	if (UAbilitySystemComponent* ASC = GetCachedASC())
	{
		ASC->GetOwnedGameplayTags(currentTags);

		// === 优先查武器专属连招表，降级到全局默认表 ===
		UComboInfoTable* comboInfoTable = GetCurrentWeaponComboTable();
		if (!comboInfoTable)
		{
			if (UDataTableManager* tableManager = UDataTableManager::Get(this))
			{
				comboInfoTable = Cast<UComboInfoTable>(tableManager->GetDataTable(EDataTableType::DT_ComboInfo));
			}
		}

		if (comboInfoTable)
		{
			// 检查是否有任何 State.Window.* Tag 存在
			const FGameplayTag ComboWindowRoot = FGameplayTag::RequestGameplayTag(TEXT("State.Window"));
			bool bInComboWindow = currentTags.HasTag(ComboWindowRoot);

			FComboInfo FoundComboInfo;
			bool bFound = false;

			if (bInComboWindow)
			{
				bFound = comboInfoTable->FindNextComboInfo(currentTags, InputType, FoundComboInfo);
			}
			else
			{
				// 没有正在执行的 TASK，执行初始连段
				FGameplayTagContainer firstAttackTags;
				firstAttackTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Window.None")));
				bFound = comboInfoTable->FindNextComboInfo(firstAttackTags, InputType, FoundComboInfo);
			}

			if (bFound)
			{
				// ===== 按执行类型分流 =====
				switch (FoundComboInfo.ExecuteType)
				{
				case EComboExecuteType::Instant:
				{
					// 体力检查
					if (CachedStaminaComp.IsValid() && !CachedStaminaComp->CanAffordCost(FoundComboInfo.StaminaCost))
					{
						UE_LOG(LogTemp, Verbose, TEXT("Stamina: 体力不足"));
						return;
					}

					if (bInComboWindow)
					{
						nextComboInfo = FoundComboInfo;
						UAbilityTask_ComboMontage* ComboTask = ActiveComboTask.Get();
						if (ComboTask && IsValid(ComboTask))
						{
							EComboInputHandledResult AcceptType = ComboTask->OnInputReceived(InputType);
							if (AcceptType == EComboInputHandledResult::Accepted)
							{
								ComboTask->OnInterrupted.RemoveAll(this);
								ComboTask->OnInterrupted.AddUniqueDynamic(this, &USL_ComboManagerComponent::OnMontageBlendOut);
							}
							else if (AcceptType == EComboInputHandledResult::AcceptedAndBlended)
							{
								OnMontageBlendOut();
							}
						}
					}
					else
					{
						// 初始连段
						CurrentComboInfo = FoundComboInfo;
						ASC->TryActivateAbilityByClass(FoundComboInfo.NextAbilityClass);

						if (USL_StaminaComponent* StaminaComp = GetCachedStaminaComp())
						{
							StaminaComp->OnComboStarted();
							StaminaComp->ConsumeStamina(FoundComboInfo.StaminaCost);
						}
					}
					break;
				}

				case EComboExecuteType::Charge:
				{
					if (bInComboWindow)
					{
						nextComboInfo = FoundComboInfo;
						UAbilityTask_ComboMontage* ComboTask = ActiveComboTask.Get();
						if (ComboTask && IsValid(ComboTask))
						{
							ComboTask->OnInterrupted.RemoveAll(this);
							ComboTask->OnInterrupted.AddUniqueDynamic(this, &USL_ComboManagerComponent::OnMontageBlendOut);
						}
					}
					else
					{
						StartCharge(InputType, FoundComboInfo);
					}
					break;
				}

				case EComboExecuteType::Channel:
				{
					CurrentComboInfo = FoundComboInfo;
					ASC->TryActivateAbilityByClass(FoundComboInfo.NextAbilityClass);
					break;
				}

				default:
					break;
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
		StaminaComp->ConsumeStamina(nextComboInfo.StaminaCost);
	}
}

void USL_ComboManagerComponent::OnMontageFinished()
{
	if (ActiveComboTask.IsValid())
	{
		ActiveComboTask->OnInterrupted.RemoveAll(this);
	}
	ActiveComboTask.Reset();

	// 通知 StaminaComponent：连击结束
	if (USL_StaminaComponent* StaminaComp = GetCachedStaminaComp())
	{
		StaminaComp->OnComboEnded();
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

/************************************************************************/
/*                              蓄力系统                                        */
/************************************************************************/

void USL_ComboManagerComponent::StartCharge(EComboInputActionType InChargeInputType, const FComboInfo& InComboInfo)
{
	if (m_bIsCharging) return;

	m_bIsCharging = true;
	m_chargeStartTime = GetWorld()->GetTimeSeconds();
	m_chargeLevel = 0.0f;
	m_chargeComboInfo = InComboInfo;
	m_bChargeHoldAbilityActivated = false;

	// 启动 Tick
	SetComponentTickEnabled(true);

	UE_LOG(LogTemp, Log, TEXT("USL_ComboManagerComponent::StartCharge - Started charging"));
}

void USL_ComboManagerComponent::UpdateCharge(float DeltaTime)
{
	if (!m_bIsCharging) return;

	float HoldTime = GetWorld()->GetTimeSeconds() - m_chargeStartTime;
	
	// 蓄力进度：0.0（MinChargeTime）~ 1.0（MaxChargeTime）
	m_chargeLevel = FMath::Clamp(
		(HoldTime - m_chargeComboInfo.MinChargeTime) /
		FMath::Max(m_chargeComboInfo.MaxChargeTime - m_chargeComboInfo.MinChargeTime, 0.01f),
		0.0f, 1.0f);

	// 到达 MinChargeTime 时激活占位 GA（播放蓄力动画）
	if (!m_bChargeHoldAbilityActivated && HoldTime >= m_chargeComboInfo.MinChargeTime)
	{
		if (m_chargeComboInfo.ChargeHoldAbilityClass)
		{
			if (UAbilitySystemComponent* ASC = GetCachedASC())
			{
				ASC->TryActivateAbilityByClass(m_chargeComboInfo.ChargeHoldAbilityClass);
				m_bChargeHoldAbilityActivated = true;
			}
		}
	}

	// 广播蓄力进度给 UI
	OnChargeLevelChanged.Broadcast(m_chargeLevel);

	// 满蓄力后自动释放
	if (m_chargeLevel >= 1.0f)
	{
		ReleaseCharge();
	}
}

void USL_ComboManagerComponent::ReleaseCharge()
{
	if (!m_bIsCharging) return;

	m_bIsCharging = false;
	SetComponentTickEnabled(false);

	UAbilitySystemComponent* ASC = GetCachedASC();
	if (!ASC) return;

	// 按蓄力等级确定释放标签
	FGameplayTag ReleaseTag = (m_chargeLevel >= 1.0f)
		? FGameplayTag::RequestGameplayTag(TEXT("State.Charge.Full"))
		: FGameplayTag::RequestGameplayTag(TEXT("State.Charge.Partial"));

	// === 查武器专属连招表找释放 ability ===
	UComboInfoTable* ComboTable = GetCurrentWeaponComboTable();
	if (!ComboTable)
	{
		if (UDataTableManager* tableManager = UDataTableManager::Get(this))
		{
			ComboTable = Cast<UComboInfoTable>(tableManager->GetDataTable(EDataTableType::DT_ComboInfo));
		}
	}

	FComboInfo ReleaseInfo;
	if (ComboTable)
	{
		FGameplayTagContainer releaseTags;
		releaseTags.AddTag(ReleaseTag);

		if (ComboTable->FindNextComboInfo(releaseTags, m_chargeComboInfo.InputActionType, ReleaseInfo))
		{
			// 应用蓄力伤害倍率
			float ChargeDamageBonus = 1.0f + (m_chargeComboInfo.MaxChargeDamageMultiplier - 1.0f) * m_chargeLevel;
			ReleaseInfo.DamageMultiplier *= ChargeDamageBonus;

			CurrentComboInfo = ReleaseInfo;
			ASC->TryActivateAbilityByClass(ReleaseInfo.NextAbilityClass);

			if (USL_StaminaComponent* StaminaComp = GetCachedStaminaComp())
			{
				StaminaComp->OnComboStarted();
				StaminaComp->ConsumeStamina(ReleaseInfo.StaminaCost);
			}
		}
	}

	// 重置
	m_chargeLevel = 0.0f;
	m_bChargeHoldAbilityActivated = false;

	UE_LOG(LogTemp, Log, TEXT("USL_ComboManagerComponent::ReleaseCharge - Released with charge level %.2f"), m_chargeLevel);
}

/************************************************************************/
/*                              内部调用                                        */
/************************************************************************/

UAbilitySystemComponent* USL_ComboManagerComponent::GetCachedASC() const
{
	if (CachedASC.IsValid())
	{
		return CachedASC.Get();
	}

	if (IAbilitySystemInterface* ASC_IF = Cast<IAbilitySystemInterface>(GetOwner()))
	{
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
	if (CachedStaminaComp.IsValid())
	{
		return CachedStaminaComp.Get();
	}

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