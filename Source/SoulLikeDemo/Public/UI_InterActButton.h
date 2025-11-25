// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UI_InterActButton.generated.h"

/**
 * 
 */

 class UButton;
 class UImage;
 class UTextBlock;

UCLASS()
class SOULLIKEDEMO_API UUI_InterActButton : public UUserWidget
{
	GENERATED_BODY()
public:
	UUI_InterActButton(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "InterActOption")
		void UpdateInterActBtnInfo(UTexture2D* showIcon,FString showText);


protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	// 为蓝图暴露的动画事件
	UFUNCTION(BlueprintImplementableEvent, Category = "InterActOption")
		void OnSelected();

	UFUNCTION(BlueprintImplementableEvent, Category = "InterActOption")
		void UnSelected();

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UButton* m_interActBtn;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UImage* m_interActIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_interActDesc;

	// 后续增加针对不同的设备的管理
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UImage* m_playerInputIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_playerInputDesc;

};


