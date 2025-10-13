// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UI_BaseCoin.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UUI_BaseCoin : public UUserWidget
{
	GENERATED_BODY()
public:

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION()
		void SetLoadImageData(FString ImgPath);

	UFUNCTION()
		void SetPicSize(int Width, int Height);

	UFUNCTION()
		void SetDynamicStr(FString Str);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

	void OnLoadedImage();



protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// Icon图片
	UPROPERTY(meta = (BindWidget))
		UImage* ShowImage;

	UPROPERTY()
		TSoftObjectPtr<UImage> SoftImageReference;

	UPROPERTY()
		int PicWidth;

	UPROPERTY()
		int PicHeight;

	UPROPERTY(meta = (BindWidget))
		FString DynamicStr;


};


