// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ProgressBar.h"
#include "SoulLikeGameGlobal.h"
#include "Components/ProgressBar.h"

UHUD_ProgressBar::UHUD_ProgressBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	// 默认值
	m_progressPercentMin = 0.0f;
	m_progressPercentMax = 100.0f;
	m_currentProgressBarPercent = 1.0f;
}

void UHUD_ProgressBar::SetProgressBarLimit_Implementation(float min, float max)
{
	m_progressPercentMin = min;
	m_progressPercentMax = max;
}

void UHUD_ProgressBar::UpdateProgressBar_Implementation(float current)
{
	if (current > m_progressPercentMax)
	{
		m_currentProgressBarPercent = m_progressPercentMax;
	}
	else if (current < m_progressPercentMin)
	{
		m_currentProgressBarPercent = m_progressPercentMin;
	}
	else
	{
		m_currentProgressBarPercent = float(current)/ (m_progressPercentMax - m_progressPercentMin);
	}
	m_progressBar->SetPercent(m_currentProgressBarPercent);
}

void UHUD_ProgressBar::GetProgressBarLimit_Implementation(float& min, float& max)
{
	min = m_progressPercentMin;
	max = m_progressPercentMax;
}

