// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_BossStatusBar.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_ProgressBar.h"
#include "HUD_StatusBar.h"
#include "Components/TextBlock.h"


UHUD_BossStatusBar::UHUD_BossStatusBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

}

void UHUD_BossStatusBar::SetBossProgressBarLimit_Implementation(EBossStatusAttributeType AttributeType, float Min, float Max)
{
	switch (AttributeType)
	{
		case EBossStatusAttributeType::EBossStatusAttribute_Health:
		{
			RETURN_IF_TRUE(m_healthProgressBar == nullptr);
			m_healthProgressBar->SetProgressBarLimit(Min, Max);
		}break;
		default:break;
	}
}


void UHUD_BossStatusBar::UpdateBossProgressInfo_Implementation(EBossStatusAttributeType AttributeType, float CurrentValue)
{
	switch (AttributeType)
	{
		case EBossStatusAttributeType::EBossStatusAttribute_Health:
		{
			RETURN_IF_TRUE(m_healthProgressBar == nullptr);
			m_healthProgressBar->UpdateProgressBar(CurrentValue);
		}break;
		default:break;
	}
}

void UHUD_BossStatusBar::ChangeBossStatus_Implementation(EPawnStatusOperation OperationType, const TArray<FStatusEffectInfo>& ChangeStatusArr)
{
	RETURN_IF_TRUE(m_bossStatusBar == nullptr);

	switch (OperationType)
	{
		case EPawnStatusOperation::EPawnStatusOperation_Add:
		{
			for (FStatusEffectInfo info : ChangeStatusArr)
			{
				m_bossStatusBar->AddStatus(info);
			}
		}break;
		case EPawnStatusOperation::EPawnStatusOperation_Update:
		{
			for (FStatusEffectInfo info : ChangeStatusArr)
			{
				m_bossStatusBar->AddStatus(info);
			}
		}break;
		case EPawnStatusOperation::EPawnStatusOperation_Remove:
		{
			for (FStatusEffectInfo info : ChangeStatusArr)
			{
				m_bossStatusBar->AddStatus(info);
			}
		}break;
		default:break;
	}
}

void UHUD_BossStatusBar::SetBossName(FString bossName)
{
	RETURN_IF_TRUE(m_bossNameText == nullptr);
	m_bossNameText->SetText(FText::FromString(bossName));
}
