// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_BaseSlot.h"
#include "UI_IconSlot.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UUI_IconSlot : public UUI_BaseSlot
{
	GENERATED_BODY()
public:
	UUI_IconSlot();
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/

	UFUNCTION(BlueprintCallable, Category = "IconSlot")
		void SetData(FStatusEffectInfo effectInfo);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
};


