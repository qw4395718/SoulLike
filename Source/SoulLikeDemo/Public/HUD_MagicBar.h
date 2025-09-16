// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD_MagicBar.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_MagicBar : public UUserWidget
{
	GENERATED_BODY()
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
		UFUNCTION()
		void InitMagicBar(float Min, float Max);

	UFUNCTION()
		void ReduceMagicPercent(float ReducePercent);

	UFUNCTION()
		void ReviveMagicPercent(float RevivePercent);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/


protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 当前进度条百分比(0~1)
	UPROPERTY()
		float CurrentProgressBarPercent;

	// 血量进度条上限,默认是1
	UPROPERTY()
		float ProgressPercentMax;

	// 血量进度条上限,默认是1
	UPROPERTY()
		float ProgressPercentMin;
};
