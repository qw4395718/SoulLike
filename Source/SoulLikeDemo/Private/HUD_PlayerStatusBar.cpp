// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_PlayerStatusBar.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_ProgressBar.h"
#include "HUD_StatusBar.h"


UHUD_PlayerStatusBar::UHUD_PlayerStatusBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

}

void UHUD_PlayerStatusBar::InitializePlayerStatusBar(
	UHUD_ProgressBar* HealthPB,
	UHUD_ProgressBar* StaminPB,
	UHUD_ProgressBar* MagicPB,
	UHUD_StatusBar* PlayerSB
	)
{
	RETURN_IF_TRUE(HealthPB == nullptr || StaminPB == nullptr || MagicPB == nullptr || PlayerSB == nullptr  )

	// 记录界面组件
	HealthProgressBar = HealthPB;
	StaminProgressBar = StaminPB;
	MagicProgressBar = MagicPB;
	PlayerStatusBar = PlayerSB;
}

void UHUD_PlayerStatusBar::UpdateProgressInfo(float HealthMax, float CurrnetHealth, float StaminaMax, float CurrentStamina, float MagicMax, float CurrentMagic)
{
	RETURN_IF_TRUE(HealthProgressBar == nullptr || StaminProgressBar == nullptr || MagicProgressBar == nullptr);
	HealthProgressBar->UpdateProgressBar(0, HealthMax, CurrnetHealth);
	StaminProgressBar->UpdateProgressBar(0, StaminaMax, CurrentStamina);
	MagicProgressBar->UpdateProgressBar(0, MagicMax, CurrentMagic);
}

void UHUD_PlayerStatusBar::AddPlayerStatus(TArray<FStatusEffectInfo> AddStatus)
{
	RETURN_IF_TRUE(PlayerStatusBar == nullptr);
	for(FStatusEffectInfo Info : AddStatus)
	{
		PlayerStatusBar->AddStatus(Info);
	}
}

void UHUD_PlayerStatusBar::UpdatePlayerStatus(TArray<FStatusEffectInfo> UpdateStatus)
{
	RETURN_IF_TRUE(PlayerStatusBar == nullptr);
	for(FStatusEffectInfo Info : UpdateStatus)
	{
		PlayerStatusBar->UpdateStatus(Info);
	}
}

void UHUD_PlayerStatusBar::RemovePlayerStatus(TArray<FStatusEffectInfo> RemoveStatus)
{
	RETURN_IF_TRUE(PlayerStatusBar == nullptr);
	for(FStatusEffectInfo Info : RemoveStatus)
	{
		PlayerStatusBar->RemoveStatus(Info.IconIndex);
	}
}
