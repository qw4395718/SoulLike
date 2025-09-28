// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_DefaultSlot.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"



UUI_DefaultSlot::UUI_DefaultSlot()
{
	// 状态标志管理Map
	StateFlagMap=[
	{ ESlotWidgetState::Normal, true },
	{ ESlotWidgetState::Hovered, false }, 
	{ ESlotWidgetState::Selected, false },
	{ ESlotWidgetState::Pressed, false },
	{ ESlotWidgetState::Disabled, false }
	];


}

void UUI_DefaultSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	// 播放鼠标进入动画
	SetSlotState(ESlotWidgetState::Hovered);
}

void UUI_DefaultSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	// 播放鼠标进入动画
	SetSlotState(ESlotWidgetState::Normal);
}

void UUI_DefaultSlot::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& NativeOnMouseButtonUp)
{
	Super::NativeOnMouseButtonUp(InGeometry, NativeOnMouseButtonUp);
	// 播放鼠标退出动画
	SetSlotState(ESlotWidgetState::Pressed);
}
