// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "StatusEffectInfo.h"
#include "UI_DefaultSlot.h"
#include "HUD_ItemIcon.generated.h"

/**
 * 
 */
class UTextBlock;

UCLASS()
class SOULLIKEDEMO_API UHUD_ItemIcon : public UUI_DefaultSlot
{
	GENERATED_BODY()
public:
	UHUD_ItemIcon();

	/************************************************************************/
	/* 继承实现																		*/
	/************************************************************************/

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	
	UFUNCTION()
		void SetData(FStatusEffectInfo effectInfo);

	UFUNCTION()
		void ClearData();

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
};


