// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "Components/Image.h"
#include "StatusEffectInfo.h"
#include "HUD_ItemIcon.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_ItemIcon : public UUserWidget
{
	GENERATED_BODY()
public:
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	
	UFUNCTION()
		void InitializeIcon(FStatusEffectInfo EffectInfo);

	UFUNCTION()
		void UpdateIcon(float RemainingTime, int32 Stacks);

	UFUNCTION()
		bool EqualIconIndex(int Index);


	UFUNCTION(BlueprintImplementableEvent)
		void PlayFlashingEffect();

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
		UImage* ImageIcon;

	UPROPERTY()
		TSoftObjectPtr<UImage> SoftImageReference;

	UPROPERTY()
		bool IsFlashing;

	UPROPERTY(meta = (BindWidget))
		int StatusStacksNum;

	UPROPERTY()
		int IconIndex;

};


