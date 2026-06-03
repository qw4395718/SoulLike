// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_PlayerStatusBar.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_ProgressBar.h"
#include "HUD_StatusBar.h"
#include <SL_CharacterBase.h>
#include <GlobalDelegatesManager.h>


UHUD_PlayerStatusBar::UHUD_PlayerStatusBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	
}

void UHUD_PlayerStatusBar::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UHUD_PlayerStatusBar::NativeConstruct()
{
	Super::NativeConstruct();
	// 绑定
	BindGlobalDelegatesEvent();
}

void UHUD_PlayerStatusBar::NativeDestruct()
{
	// 解绑
	UnbindGlobalDelegatesEvent();
	Super::NativeDestruct();
}

void UHUD_PlayerStatusBar::BindGlobalDelegatesEvent()
{
	if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
	{
		EPlayerStatusAttributeType targetAttributeType = EPlayerStatusAttributeType::EPlayerStatusAttribute_Health;

		// 使用AddLambda替代AddDynamic
		HealthChangedHandle = globalDelegatesManager->OnAttributeHealthChanged.AddLambda([this, targetAttributeType](AActor* TargetPlayer, float OldValue, float NewValue, float MinValue, float MaxValue)
			{
				// 检测是否为该目标
				if (TargetPlayer && GetOwningPlayer() && TargetPlayer == GetOwningPlayer()->GetPawn())
				{
					SetProgressBarLimit(targetAttributeType, MinValue, MaxValue);
					UpdateProgressInfo(targetAttributeType, OldValue, NewValue);
				}
			});

		targetAttributeType = EPlayerStatusAttributeType::EPlayerStatusAttribute_Stamin;

		StaminChangedHandle = globalDelegatesManager->OnAttributeStaminaChanged.AddLambda([this, targetAttributeType](AActor* TargetPlayer, float OldValue, float NewValue, float MinValue, float MaxValue)
			{
				// 检测是否为该目标
				if (TargetPlayer && GetOwningPlayer() && TargetPlayer == GetOwningPlayer()->GetPawn())
				{
					SetProgressBarLimit(targetAttributeType, MinValue, MaxValue);
					UpdateProgressInfo(targetAttributeType, OldValue, NewValue);
				}
			});

		// ===== 绑定后主动拉取当前值完成初始化 =====
		if (ASL_CharacterBase* Character = Cast<ASL_CharacterBase>(GetOwningPlayerPawn()))
		{
			// Health 初始化
			float CurrentHealth = Character->GetCurrentHealth();
			float MaxHealth = Character->GetMaxHealth();
			SetProgressBarLimit(EPlayerStatusAttributeType::EPlayerStatusAttribute_Health, 0.0f, MaxHealth);
			UpdateProgressInfo(EPlayerStatusAttributeType::EPlayerStatusAttribute_Health, CurrentHealth, CurrentHealth);

			// Stamina 初始化
			float CurrentStamina = Character->GetCurrentStamina();
			float MaxStamina = Character->GetMaxStamina();
			SetProgressBarLimit(EPlayerStatusAttributeType::EPlayerStatusAttribute_Stamin, 0.0f, MaxStamina);
			UpdateProgressInfo(EPlayerStatusAttributeType::EPlayerStatusAttribute_Stamin, CurrentStamina, CurrentStamina);
		}
	}
}

void UHUD_PlayerStatusBar::UnbindGlobalDelegatesEvent()
{
	if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
	{
		if (HealthChangedHandle.IsValid())
		{
			globalDelegatesManager->OnAttributeHealthChanged.Remove(HealthChangedHandle);
			HealthChangedHandle.Reset();
		}
		if (StaminChangedHandle.IsValid())
		{
			globalDelegatesManager->OnAttributeStaminaChanged.Remove(StaminChangedHandle);
			StaminChangedHandle.Reset();
		}
	}
}

void UHUD_PlayerStatusBar::SetProgressBarLimit_Implementation(EPlayerStatusAttributeType AttributeType, float Min, float Max)
{
	switch (AttributeType)
	{
	case EPlayerStatusAttributeType::EPlayerStatusAttribute_Health:
	{
		RETURN_IF_TRUE(m_healthProgressBar == nullptr);
		m_healthProgressBar->SetProgressBarLimit(Min,Max);
	}break;
	case EPlayerStatusAttributeType::EPlayerStatusAttribute_Magic:
	{
		RETURN_IF_TRUE(m_magicProgressBar == nullptr);
		m_magicProgressBar->SetProgressBarLimit(Min, Max);
	}break;
	case EPlayerStatusAttributeType::EPlayerStatusAttribute_Stamin:
	{
		RETURN_IF_TRUE(m_staminProgressBar == nullptr);
		m_staminProgressBar->SetProgressBarLimit(Min, Max);
	}break;
	default:break;
	}
}

void UHUD_PlayerStatusBar::UpdateProgressInfo_Implementation(EPlayerStatusAttributeType AttributeType, float OldValue, float CurrentValue)
{
	switch (AttributeType)
	{
		case EPlayerStatusAttributeType::EPlayerStatusAttribute_Health:
		{
			RETURN_IF_TRUE(m_healthProgressBar == nullptr);
			m_healthProgressBar->UpdateProgressBar(CurrentValue);
		}break;
		case EPlayerStatusAttributeType::EPlayerStatusAttribute_Magic:
		{
			RETURN_IF_TRUE(m_magicProgressBar == nullptr);
			m_magicProgressBar->UpdateProgressBar(CurrentValue);
		}break;
		case EPlayerStatusAttributeType::EPlayerStatusAttribute_Stamin:
		{
			RETURN_IF_TRUE(m_staminProgressBar == nullptr);
			m_staminProgressBar->UpdateProgressBar(CurrentValue);
		}break;
		default:break;
	}
}

void UHUD_PlayerStatusBar::ChangePlayerStatus_Implementation(EPawnStatusOperation OperationType, const TArray<FStatusEffectInfo>& ChangeStatusArr)
{
	RETURN_IF_TRUE(m_playerStatusBar == nullptr);

	switch (OperationType)
	{
		case EPawnStatusOperation::EPawnStatusOperation_Add:
		{
			for (FStatusEffectInfo info : ChangeStatusArr)
			{
				m_playerStatusBar->AddStatus(info);
			}
		}break;
		case EPawnStatusOperation::EPawnStatusOperation_Update:
		{
			for (FStatusEffectInfo info : ChangeStatusArr)
			{
				m_playerStatusBar->AddStatus(info);
			}
		}break;
		case EPawnStatusOperation::EPawnStatusOperation_Remove:
		{
			for (FStatusEffectInfo info : ChangeStatusArr)
			{
				m_playerStatusBar->AddStatus(info);
			}
		}break;
		default:break;
	}
}
