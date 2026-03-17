// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_PlayerStatusBar.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_ProgressBar.h"
#include "HUD_StatusBar.h"


UHUD_PlayerStatusBar::UHUD_PlayerStatusBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

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

void UHUD_PlayerStatusBar::UpdateProgressInfo_Implementation(EPlayerStatusAttributeType AttributeType, float CurrentValue)
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

