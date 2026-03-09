// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_PlayerStatusBar.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_ProgressBar.h"
#include "HUD_StatusBar.h"


UHUD_PlayerStatusBar::UHUD_PlayerStatusBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

}

void UHUD_PlayerStatusBar::UpdateProgressInfo(float currnetHealth, float currentStamina, float currentMagic)
{
	RETURN_IF_TRUE(m_healthProgressBar == nullptr || m_staminProgressBar == nullptr || m_magicProgressBar == nullptr);
	m_healthProgressBar->UpdateProgressBar(0, 1, currnetHealth);
	m_staminProgressBar->UpdateProgressBar(0, 1, currentStamina);
	m_magicProgressBar->UpdateProgressBar(0, 1, currentMagic);
}

void UHUD_PlayerStatusBar::AddPlayerStatus_Implementation(const TArray<FStatusEffectInfo>& addStatusArr)
{
	RETURN_IF_TRUE(m_playerStatusBar == nullptr);
	for(FStatusEffectInfo info : addStatusArr)
	{
		m_playerStatusBar->AddStatus(info);
	}
}

void UHUD_PlayerStatusBar::UpdatePlayerStatus_Implementation(const TArray<FStatusEffectInfo>& updateStatusArr)
{
	RETURN_IF_TRUE(m_playerStatusBar == nullptr);
	for(FStatusEffectInfo info : updateStatusArr)
	{
		m_playerStatusBar->UpdateStatus(info);
	}
}

void UHUD_PlayerStatusBar::RemovePlayerStatus_Implementation(const TArray<FStatusEffectInfo>& removeStatusArr)
{
	RETURN_IF_TRUE(m_playerStatusBar == nullptr);
	for(FStatusEffectInfo info : removeStatusArr)
	{
		m_playerStatusBar->RemoveStatus(info.IconIndex);
	}
}
