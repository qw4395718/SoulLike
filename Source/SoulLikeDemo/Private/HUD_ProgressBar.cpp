// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ProgressBar.h"
#include "SoulLikeGameGlobal.h"

UHUD_ProgressBar::UHUD_ProgressBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	// Ä¬ÈÏÖµ
	ProgressPercentMin = 0;
	ProgressPercentMax = 100;
	CurrentProgressBarPercent = 100;
}

void UHUD_ProgressBar::UpdateProgressBar(float Min, float Max,float Current)
{
	ProgressPercentMin = Min;
	ProgressPercentMax = Max;
	if (Current > Max)
	{
		CurrentProgressBarPercent = Max;
	}
	else if (Current < Min)
	{
		CurrentProgressBarPercent = Min;
	}
	else
	{
		CurrentProgressBarPercent = Current;
	}
	
}

