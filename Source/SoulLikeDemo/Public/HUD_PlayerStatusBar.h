// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_PlayerStatusBar.generated.h"

class UHUD_ProgressBar;
class UHUD_StatusBar;

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
		void InitializePlayerStatusBar(
			UHUD_ProgressBar* HealthPB,
			UHUD_ProgressBar* StaminPB,
			UHUD_ProgressBar* MagicPB,
			UHUD_StatusBar* PlayerSB
			);




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
		UHUD_ProgressBar* StaminProgressBar;

	UPROPERTY()
		UHUD_ProgressBar* MagicProgressBar;

	UPROPERTY()
		UHUD_StatusBar* PlayerStatusBar;

};

