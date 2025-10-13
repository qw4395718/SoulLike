// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UI_BaseSlot.h"
#include "UI_DefaultSlot.generated.h"

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
	/************************************************************************/
	/* 继承实现																		*/
	/************************************************************************/


	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION()
		void SetNewAcquireImageVisible(bool visible);

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

};


