// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_PawnStatusBarInScreen.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_ProgressBar.h"
#include "HUD_StatusBar.h"
#include <GlobalDelegatesManager.h>
#include <Components/TextBlock.h>


UHUD_PawnStatusBarInScreen::UHUD_PawnStatusBarInScreen(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	OwningPawn = nullptr;
	
}

void UHUD_PawnStatusBarInScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UHUD_PawnStatusBarInScreen::NativeConstruct()
{
	Super::NativeConstruct();
	// 绑定
	BindGlobalDelegatesEvent();
}

void UHUD_PawnStatusBarInScreen::NativeDestruct()
{
	// 解绑
	UnbindGlobalDelegatesEvent();
	Super::NativeDestruct();
}

void UHUD_PawnStatusBarInScreen::BindGlobalDelegatesEvent()
{
	if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
	{

	}
}

void UHUD_PawnStatusBarInScreen::UnbindGlobalDelegatesEvent()
{
	if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
	{

	}
}

void UHUD_PawnStatusBarInScreen::SetProgressBarLimit_Implementation(EPlayerStatusAttributeType AttributeType, float Min, float Max)
{
	switch (AttributeType)
	{
	case EPlayerStatusAttributeType::EPlayerStatusAttribute_Health:
	{
		RETURN_IF_TRUE(m_healthProgressBar == nullptr);
		m_healthProgressBar->SetProgressBarLimit(Min,Max);
	}break;
	default:break;
	}
}

void UHUD_PawnStatusBarInScreen::UpdateProgressInfo_Implementation(EPlayerStatusAttributeType AttributeType, float OldValue, float CurrentValue)
{
	switch (AttributeType)
	{
		case EPlayerStatusAttributeType::EPlayerStatusAttribute_Health:
		{
			RETURN_IF_TRUE(m_healthProgressBar == nullptr);
			float LimitMin,LimitMax;
			m_healthProgressBar->GetProgressBarLimit(LimitMin, LimitMax);
			m_healthProgressBar->UpdateProgressBar(CurrentValue);
			FString PGstr = FString::Printf(TEXT("%.0f/%.0f"), CurrentValue, LimitMax);
			Text_HealthPG->SetText(FText::FromString(PGstr));
		}break;
		default:break;
	}
}

void UHUD_PawnStatusBarInScreen::ChangePlayerStatus_Implementation(EPawnStatusOperation OperationType, const TArray<FStatusEffectInfo>& ChangeStatusArr)
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

void UHUD_PawnStatusBarInScreen::SetOwningPawn(AActor* OwnPawn)
{
	OwningPawn = OwnPawn;
}

