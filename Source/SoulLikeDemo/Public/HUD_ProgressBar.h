// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD_ProgressBar.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_ProgressBar : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_ProgressBar(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION()
		void UpdateProgressBar(float Min, float Max, float Current);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/


protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 当前进度条百分比(0~1)
	UPROPERTY(BlueprintReadOnly,meta = (BindWidget))
		float CurrentProgressBarPercent;

	// 血量进度条上限,默认是1
	UPROPERTY(BlueprintReadOnly,meta = (BindWidget))
		float ProgressPercentMax;

	// 血量进度条上限,默认是1
	UPROPERTY(BlueprintReadOnly,meta = (BindWidget))
		float ProgressPercentMin;
};
