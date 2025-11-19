// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ProgressBar.h"
#include "SoulLikeGameGlobal.h"
#include "Components/ProgressBar.h"

UHUD_ProgressBar::UHUD_ProgressBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	// Ä¬ÈÏÖµ
	ProgressPercentMin = 0;
	ProgressPercentMax = 100;
	CurrentProgressBarPercent = 1;
}

void UHUD_ProgressBar::UpdateProgressBar(float Min, float Max,float Current)
{
	ProgressPercentMin = Min;
	ProgressPercentMax = Max;
	if (Current > Max)
	{
		CurrentProgressBarPercent = 1.0f;
	}
	else if (Current < Min)
	{
		CurrentProgressBarPercent = 0.0f;
	}
	else
	{
		CurrentProgressBarPercent = float(Current)/ Max;
	}
	
}

void UHUD_ProgressBar::InitializeProgressBar(UProgressBar* ProgressBarRefence)
{
	RETURN_IF_TRUE(ProgressBarRefence == nullptr);
	ProgressBar = ProgressBarRefence;
}

