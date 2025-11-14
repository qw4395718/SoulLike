// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_BossStatusBar.generated.h"

class UHUD_ProgressBar;
class UHUD_StatusBar;

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_BossStatusBar : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_BossStatusBar(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	
	UFUNCTION(BlueprintCallable, Category = "Boss StatusBar")
		void InitializeBossStatusBar(
			UHUD_ProgressBar* HealthPB,
			UHUD_StatusBar* BossSB
			);

	UFUNCTION(BlueprintCallable, Category = "Boss StatusBar")
		void UpdateProgressInfo(
		float HealthMax,
		float CurrnetHealth);

	UFUNCTION(BlueprintCallable, Category = "Boss StatusBar")
		void AddBossStatus(TArray<FStatusEffectInfo> AddStatus);

	UFUNCTION(BlueprintCallable, Category = "Boss StatusBar")
		void UpdateBossStatus(TArray<FStatusEffectInfo> UpdateStatus);

	UFUNCTION(BlueprintCallable, Category = "Boss StatusBar")
		void RemoveBossStatus(TArray<FStatusEffectInfo> RemoveStatus);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 控件引用
	UPROPERTY()
		UHUD_ProgressBar* HealthProgressBar;


	UPROPERTY()
		UHUD_StatusBar* BossStatusBar;

};

