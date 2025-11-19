// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_Main_PC.generated.h"

class UHUD_PlayerStatusBar;

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_Main_PC : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_Main_PC(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	
	UFUNCTION(BlueprintCallable, Category = "Main PC")
		void InitializeMainHUD_PC(UHUD_PlayerStatusBar* StatusBar);

	UFUNCTION(BlueprintCallable, Category = "Main PC")
		void UpdateProgressInfo(
			float HealthMax,
			float CurrnetHealth,
			float StaminaMax,
			float CurrentStamina,
			float MagicMax,
			float CurrentMagic);

	UFUNCTION(BlueprintCallable, Category = "Main PC")
		void AddPlayerStatus(TArray<FStatusEffectInfo> AddStatus);

	UFUNCTION(BlueprintCallable, Category = "Main PC")
		void UpdatePlayerStatus(TArray<FStatusEffectInfo> UpdateStatus);

	UFUNCTION(BlueprintCallable, Category = "Main PC")
		void RemovePlayerStatus(TArray<FStatusEffectInfo> RemoveStatus);
protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 控件引用-暴露给外部
	UPROPERTY()
		UHUD_PlayerStatusBar* PlayerStatusBar;

};

