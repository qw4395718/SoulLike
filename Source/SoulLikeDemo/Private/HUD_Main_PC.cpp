// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_Main_PC.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_PlayerStatusBar.h"
#include "StatusEffectInfo.h"

UHUD_Main_PC::UHUD_Main_PC(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

}

void UHUD_Main_PC::InitializeMainHUD_PC(UHUD_PlayerStatusBar* StatusBar)
{
	RETURN_IF_TRUE(StatusBar == nullptr);
	PlayerStatusBar = StatusBar;
}

void UHUD_Main_PC::UpdateProgressInfo(float HealthMax, float CurrnetHealth, float StaminaMax, float CurrentStamina, float MagicMax, float CurrentMagic)
{
	RETURN_IF_TRUE(PlayerStatusBar == nullptr);
	PlayerStatusBar->UpdateProgressInfo(HealthMax, CurrnetHealth, StaminaMax, CurrentStamina, MagicMax, CurrentMagic);
}

void UHUD_Main_PC::AddPlayerStatus(TArray<FStatusEffectInfo> AddStatus)
{
	RETURN_IF_TRUE(PlayerStatusBar == nullptr);
	PlayerStatusBar->AddPlayerStatus(AddStatus);
}

void UHUD_Main_PC::UpdatePlayerStatus(TArray<FStatusEffectInfo> UpdateStatus)
{
	RETURN_IF_TRUE(PlayerStatusBar == nullptr);
	PlayerStatusBar->UpdatePlayerStatus(UpdateStatus);
}

void UHUD_Main_PC::RemovePlayerStatus(TArray<FStatusEffectInfo> RemoveStatus)
{
	RETURN_IF_TRUE(PlayerStatusBar == nullptr);
	PlayerStatusBar->RemovePlayerStatus(RemoveStatus);
}
