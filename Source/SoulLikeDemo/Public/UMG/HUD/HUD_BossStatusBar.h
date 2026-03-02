// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "StatusEffectInfo.h"
#include "HUD_BossStatusBar.generated.h"

class UHUD_ProgressBar;
class UHUD_StatusBar;
class UTextBlock;

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
		void UpdateProgressInfo(
		float currnetHealthPercent);

	UFUNCTION(BlueprintCallable, Category = "Boss StatusBar")
		void AddBossStatus(TArray<FStatusEffectInfo> addStatusArr);

	UFUNCTION(BlueprintCallable, Category = "Boss StatusBar")
		void UpdateBossStatus(TArray<FStatusEffectInfo> updateStatusArr);

	UFUNCTION(BlueprintCallable, Category = "Boss StatusBar")
		void RemoveBossStatus(TArray<FStatusEffectInfo> removeStatusArr);

	UFUNCTION(BlueprintCallable, Category = "Boss StatusBar")
		void SetBossName(FString bossName);

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
		UHUD_ProgressBar* m_healthProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UHUD_StatusBar* m_bossStatusBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_bossNameText;

};

