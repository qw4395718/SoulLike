// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UI_DefaultSlot.generated.h"

class UUI_BaseSlot;

// 处理多个状态同时存在的情况
UENUM(BlueprintType)
enum class ESlotWidgetState : uint8
{
	Normal = 0,
	Hovered = 1,
	Selected = 2,
	Pressed = 3,  // 高优先级
	Disabled = 4   // 最高优先级
};


/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UUI_DefaultSlot : public UUI_BaseSlot
{
	GENERATED_BODY()
public:

	UUI_DefaultSlot();
	/************************************************************************/
	/*                               继承实现                                       */
	/************************************************************************/
	// 鼠标进入
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 鼠标离开
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// 鼠标点击(左/右)
	virtual void NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;



	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintImplementableEvent)
		void PlayMouseEnterEffect();

	UFUNCTION(BlueprintImplementableEvent)
		void PlayMouseLevelEffect();

	UFUNCTION(BlueprintImplementableEvent)
		void PlayMouseClickEffect();

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 新获取提示图标
	UPROPERTY(meta = (BindWidget))
		UImage* NewTipImage;

	// 悬浮光标图片
	UPROPERTY(meta = (BindWidget))
		UImage* OverLayImage;

	// 选中光标图片
	UPROPERTY(meta = (BindWidget))
		UImage* SelectedImage;



};


