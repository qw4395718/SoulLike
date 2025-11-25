// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_Main_PC.generated.h"

class UHUD_PlayerStatusBar;
class UHUD_EquipmentBar;

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
		void UpdateProgressInfo(
			float currnetHealth,
			float currentStamina,
			float currentMagic);

	UFUNCTION(BlueprintCallable, Category = "Main PC")
		void AddPlayerStatus(TArray<FStatusEffectInfo> addStatusArr);

	UFUNCTION(BlueprintCallable, Category = "Main PC")
		void UpdatePlayerStatus(TArray<FStatusEffectInfo> updateStatusArr);

	UFUNCTION(BlueprintCallable, Category = "Main PC")
		void RemovePlayerStatus(TArray<FStatusEffectInfo> removeStatusArr);

	UFUNCTION(BlueprintCallable, Category = "Main PC")
		void UpdateTargetSlot(int type, FStatusEffectInfo status);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 控件引用-暴露给外部
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UHUD_PlayerStatusBar* m_playerStatusBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UHUD_EquipmentBar* m_playerEquipmentBar;

};

