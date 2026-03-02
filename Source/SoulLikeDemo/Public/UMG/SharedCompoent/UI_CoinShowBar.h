// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "UI_BaseCoin.h"
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UI_CoinShowBar.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UUI_CoinShowBar : public UUserWidget
{
	GENERATED_BODY()
public:
	UUI_CoinShowBar(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/

};


