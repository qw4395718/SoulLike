// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SoulLikeGameGlobal.h"
#include "Components/Image.h"
#include "StatusEffectInfo.h"
#include "UI_IconSlot.h"
#include "HUD_StatusIcon.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_StatusIcon : public UUI_IconSlot
{
	GENERATED_BODY()
public:
	UHUD_StatusIcon();
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	
	UFUNCTION()
		void InitializeIcon(FStatusEffectInfo EffectInfo);

	UFUNCTION()
		void UpdateIcon(float RemainingTime, int32 Stacks);

	UFUNCTION()
		bool EqualIconIndex(int Index);


	UFUNCTION(BlueprintNativeEvent)
		void PlayFlashingEffect();

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	UPROPERTY()
		bool IsFlashing;

	UPROPERTY()
		int IconIndex;

};


