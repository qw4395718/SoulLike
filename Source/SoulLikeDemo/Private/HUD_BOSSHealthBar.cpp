// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_BOSSHealthBar.h"
#include "SoulLikeGameGlobal.h"

void UHUD_BOSSHealthBar::InitHealthBar(float Min, float Max)
{
	ProgressPercentMin = Min;
	ProgressPercentMax = Max;
	CurrentProgressBarPercent = ProgressPercentMax;
}

void UHUD_BOSSHealthBar::ReduceHealthPercent(float ReducePercent)
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

void UHUD_BOSSHealthBar::ReviveHealthPercent(float RevivePercent)
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
