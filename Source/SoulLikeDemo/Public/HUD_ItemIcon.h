// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "Components/Image.h"
#include "StatusEffectInfo.h"
#include "UI_DefaultSlot.h"
#include "HUD_ItemIcon.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_ItemIcon : public UUI_DefaultSlot
{
	GENERATED_BODY()
public:
	UHUD_ItemIcon();
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	
	UFUNCTION()
		void SetData(FStatusEffectInfo EffectInfo);

	UFUNCTION()
		void ClearData();

	UFUNCTION()
		bool EqualIconIndex(int Index);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/

	UPROPERTY(meta = (BindWidget))
		FString ItemName;

	UPROPERTY(meta = (BindWidget))
		FString ItemDesc;

	UPROPERTY()
		int ItemIndex;

};


