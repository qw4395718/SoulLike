// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_PlayerStatusBar.generated.h"

class UHUD_ProgressBar;
class UHUD_StatusBar;
class UTextBlock;
/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_PlayerStatusBar : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_PlayerStatusBar(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/

	UFUNCTION(BlueprintCallable, Category = "Player StatusBar")
		void UpdateProgressInfo(
		float currnetHealth,
		float currentStamina,
		float currentMagic);

	UFUNCTION(BlueprintCallable, Category = "Player StatusBar")
		void AddPlayerStatus(TArray<FStatusEffectInfo> addStatusArr);

	UFUNCTION(BlueprintCallable, Category = "Player StatusBar")
		void UpdatePlayerStatus(TArray<FStatusEffectInfo> updateStatusArr);

	UFUNCTION(BlueprintCallable, Category = "Player StatusBar")
		void RemovePlayerStatus(TArray<FStatusEffectInfo> removeStatusArr);

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
		UHUD_ProgressBar* m_staminProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UHUD_ProgressBar* m_magicProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UHUD_StatusBar* m_playerStatusBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_healthText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_staminText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_magicText;

};

