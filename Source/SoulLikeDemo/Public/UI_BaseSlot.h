// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_BaseSlot.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UUI_BaseSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	UUI_BaseSlot(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "Base Coin")
		void SetImageBrush(UTexture2D* icon);

	UFUNCTION()
		void SetStacksNum(int num);


protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// Icon图片
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		class UImage* m_showImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		class UTextBlock* m_stackNum;

};


