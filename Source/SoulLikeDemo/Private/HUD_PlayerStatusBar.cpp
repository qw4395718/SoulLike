// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_PlayerStatusBar.h"
#include "SoulLikeGameGlobal.h"

UHUD_PlayerStatusBar::UHUD_PlayerStatusBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
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
