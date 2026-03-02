// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UObject/NoExportTypes.h"
#include "Styling/SlateColor.h"
#include "UI_InterActButton.generated.h"

/**
 * 
 */

 class UButton;
 class UImage;
 class UTextBlock;
 class UWidgetAnimation;




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
		void UpdateInterActBtnInfo(int32 index,UTexture2D* showIcon,FString showText);

	UFUNCTION(BlueprintCallable, Category = "InterActOption")
		void SetSelected(bool bIsSelected, bool bPlayAnimation = true);

	UFUNCTION(BlueprintCallable, Category = "InterActOption")
		int32 GetAssignedIndex();

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

	// 为蓝图暴露的动画事件-播放动画
	UFUNCTION(BlueprintNativeEvent, Category = "Animation")
		void PlaySelectionAnimation(bool bIsSelected);

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UButton* m_interActBtn;
protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/



	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UImage* m_interActIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_interActDesc;

	// 后续增加针对不同的设备的管理
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UImage* m_playerInputIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_playerInputDesc;

	// 动画相关属性
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		UWidgetAnimation* SelectionAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		UWidgetAnimation* DeselectionAnimation;

	// 允许蓝图配置的颜色
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		FLinearColor NormalColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
		FLinearColor SelectedColor = FLinearColor::Yellow;

	// 选中状态
	bool bSelected = false;

	// 外部序号
	int32 AssignedIndex;

};


