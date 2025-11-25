// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_Main_PC.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_PlayerStatusBar.h"
#include "HUD_EquipmentBar.h"
#include "StatusEffectInfo.h"

UHUD_Main_PC::UHUD_Main_PC(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

}

void UHUD_Main_PC::UpdateProgressInfo(float currnetHealth, float currentStamina, float currentMagic)
{
	RETURN_IF_TRUE(m_playerStatusBar == nullptr);
	m_playerStatusBar->UpdateProgressInfo(currnetHealth, currentStamina, currentMagic);
}

void UHUD_Main_PC::AddPlayerStatus(TArray<FStatusEffectInfo> addStatusArr)
{
	RETURN_IF_TRUE(m_playerStatusBar == nullptr);
	m_playerStatusBar->AddPlayerStatus(addStatusArr);
}

void UHUD_Main_PC::UpdatePlayerStatus(TArray<FStatusEffectInfo> updateStatusArr)
{
	RETURN_IF_TRUE(m_playerStatusBar == nullptr);
	m_playerStatusBar->UpdatePlayerStatus(updateStatusArr);
}

void UHUD_Main_PC::RemovePlayerStatus(TArray<FStatusEffectInfo> removeStatusArr)
{
	RETURN_IF_TRUE(m_playerStatusBar == nullptr);
	m_playerStatusBar->RemovePlayerStatus(removeStatusArr);
}

void UHUD_Main_PC::UpdateTargetSlot(int type, FStatusEffectInfo status)
{
	RETURN_IF_TRUE(m_playerEquipmentBar == nullptr);
	m_playerEquipmentBar->UpdateTargetSlot(type, status);
}
