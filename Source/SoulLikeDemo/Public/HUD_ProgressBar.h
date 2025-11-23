// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD_ProgressBar.generated.h"

class UProgressBar;

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
		void UpdateProgressBar(float min, float max, float current);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/


protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 控件引用
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UProgressBar* m_progressBar;

	// 当前进度条百分比(0~1)
	UPROPERTY()
		float m_currentProgressBarPercent;

	// 血量进度条上限,默认是1
	UPROPERTY()
		float m_progressPercentMax;

	// 血量进度条下限,默认是0
	UPROPERTY()
		float m_progressPercentMin;
};
