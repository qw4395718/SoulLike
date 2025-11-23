// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_BossStatusBar.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_ProgressBar.h"
#include "HUD_StatusBar.h"


UHUD_BossStatusBar::UHUD_BossStatusBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

}

void UHUD_BossStatusBar::InitializeBossStatusBar(UHUD_StatusBar* bossSB)
{
	RETURN_IF_TRUE(bossSB == nullptr)
	// 记录界面组件
	m_bossStatusBar = bossSB;
}

void UHUD_BossStatusBar::UpdateProgressInfo(float currnetHealthPercent)
{
	RETURN_IF_TRUE(m_healthProgressBar == nullptr);
	m_healthProgressBar->UpdateProgressBar(0, 1, currnetHealthPercent);
}

void UHUD_BossStatusBar::AddBossStatus(TArray<FStatusEffectInfo> addStatusArr)
{
	RETURN_IF_TRUE(m_bossStatusBar == nullptr);
	for(FStatusEffectInfo Info : addStatusArr)
	{
		m_bossStatusBar->AddStatus(Info);
	}
}

void UHUD_BossStatusBar::UpdateBossStatus(TArray<FStatusEffectInfo> updateStatusArr)
{
	RETURN_IF_TRUE(m_bossStatusBar == nullptr);
	for(FStatusEffectInfo Info : updateStatusArr)
	{
		m_bossStatusBar->UpdateStatus(Info);
	}
}

void UHUD_BossStatusBar::RemoveBossStatus(TArray<FStatusEffectInfo> removeStatusArr)
{
	RETURN_IF_TRUE(m_bossStatusBar == nullptr);
	for(FStatusEffectInfo Info : removeStatusArr)
	{
		m_bossStatusBar->RemoveStatus(Info.IconIndex);
	}
}
