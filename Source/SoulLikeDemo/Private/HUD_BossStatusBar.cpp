// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_BossStatusBar.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_ProgressBar.h"
#include "HUD_StatusBar.h"


UHUD_BossStatusBar::UHUD_BossStatusBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

}

void UHUD_BossStatusBar::InitializeBossStatusBar(
	UHUD_ProgressBar* HealthPB,
	UHUD_StatusBar* BossSB
	)
{
	RETURN_IF_TRUE(HealthPB == nullptr || BossSB == nullptr  )

	// 记录界面组件
	HealthProgressBar = HealthPB;
	BossStatusBar = BossSB;
}

void UHUD_BossStatusBar::UpdateProgressInfo(float HealthMax, float CurrnetHealth)
{
	RETURN_IF_TRUE(HealthProgressBar == nullptr);
	HealthProgressBar->UpdateProgressBar(0, HealthMax, CurrnetHealth);
}

void UHUD_BossStatusBar::AddBossStatus(TArray<FStatusEffectInfo> AddStatus)
{
	RETURN_IF_TRUE(BossStatusBar == nullptr);
	for(FStatusEffectInfo Info : AddStatus)
	{
		BossStatusBar->AddStatus(Info);
	}
}

void UHUD_BossStatusBar::UpdateBossStatus(TArray<FStatusEffectInfo> UpdateStatus)
{
	RETURN_IF_TRUE(BossStatusBar == nullptr);
	for(FStatusEffectInfo Info : UpdateStatus)
	{
		BossStatusBar->UpdateStatus(Info);
	}
}

void UHUD_BossStatusBar::RemoveBossStatus(TArray<FStatusEffectInfo> RemoveStatus)
{
	RETURN_IF_TRUE(BossStatusBar == nullptr);
	for(FStatusEffectInfo Info : RemoveStatus)
	{
		BossStatusBar->RemoveStatus(Info.IconIndex);
	}
}
