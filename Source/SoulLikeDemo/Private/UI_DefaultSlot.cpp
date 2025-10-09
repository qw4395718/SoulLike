// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_DefaultSlot.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"



UUI_DefaultSlot::UUI_DefaultSlot()
{

}

void UUI_DefaultSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	PlayMouseEnterEffect();
}

void UUI_DefaultSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	PlayMouseLevelEffect();
}

void UUI_DefaultSlot::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& NativeOnMouseButtonUp)
{
	Super::NativeOnMouseButtonUp(InGeometry, NativeOnMouseButtonUp);

	PlayMouseClickEffect();
}


