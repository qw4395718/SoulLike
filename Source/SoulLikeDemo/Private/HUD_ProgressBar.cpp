// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ProgressBar.h"
#include "SoulLikeGameGlobal.h"

void UHUD_ProgressBar::InitProgressBar(float Min, float Max)
{
	ProgressPercentMin = Min;
	ProgressPercentMax = Max;
	CurrentProgressBarPercent = ProgressPercentMax;
}

void UHUD_ProgressBar::ReduceProgressBarPercent(float ReducePercent)
{
	RETURN_IF_TRUE(ReducePercent < ProgressPercentMin || ReducePercent > ProgressPercentMax);
	if (CurrentProgressBarPercent - ReducePercent < ProgressPercentMin)
	{
		CurrentProgressBarPercent = ProgressPercentMin;
	}
	else if (CurrentProgressBarPercent - ReducePercent > ProgressPercentMin)
	{
		CurrentProgressBarPercent = ProgressPercentMax;
	}
	else
	{
		CurrentProgressBarPercent -= ReducePercent;
	}
}

void UHUD_ProgressBar::ReviveProgressBarPercent(float RevivePercent)
{
	RETURN_IF_TRUE(RevivePercent < ProgressPercentMin || RevivePercent > ProgressPercentMax);
	if (CurrentProgressBarPercent + RevivePercent < ProgressPercentMin)
	{
		CurrentProgressBarPercent = ProgressPercentMin;
	}
	else if (CurrentProgressBarPercent + RevivePercent > ProgressPercentMin)
	{
		CurrentProgressBarPercent = ProgressPercentMax;
	}
	else
	{
		CurrentProgressBarPercent += RevivePercent;
	}
}

