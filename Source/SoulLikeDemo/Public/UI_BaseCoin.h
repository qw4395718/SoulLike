// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UI_BaseCoin.generated.h"

class UTexture2D;
class UTextBlock;
class UImage;

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UUI_BaseCoin : public UUserWidget
{
	GENERATED_BODY()
public:
	UUI_BaseCoin(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "Base Coin")
		virtual void InitializeUIComp();

	UFUNCTION(BlueprintCallable, Category = "Base Coin")
		void SetImageBrush(UTexture2D* showImage);

	UFUNCTION()
		void SetDynamicStr(FString str);

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
		UImage* m_showImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_dynamicStr;


};


