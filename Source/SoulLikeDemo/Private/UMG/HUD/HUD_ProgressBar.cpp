// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ProgressBar.h"
#include "SoulLikeGameGlobal.h"
#include "Components/ProgressBar.h"

UHUD_ProgressBar::UHUD_ProgressBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	// Ĭ��ֵ
	m_progressPercentMin = 0.0f;
	m_progressPercentMax = 1.0f;
	m_currentProgressBarPercent = 1.0f;
}

void UHUD_ProgressBar::UpdateProgressBar_Implementation(float min, float max,float current)
{
	m_progressPercentMin = min;
	m_progressPercentMax = max;
	if (current > max)
	{
		m_currentProgressBarPercent = m_progressPercentMax;
	}
	else if (current < min)
	{
		m_currentProgressBarPercent = m_progressPercentMin;
	}
	else
	{
		m_currentProgressBarPercent = float(current)/ (m_progressPercentMax - m_progressPercentMin);
	}
	m_progressBar->SetPercent(m_currentProgressBarPercent);
}

