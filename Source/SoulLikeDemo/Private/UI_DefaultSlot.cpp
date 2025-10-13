// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_DefaultSlot.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "SL_Macros.h"


void UUI_DefaultSlot::SetNewAcquireImageVisible(bool visible)
{
	RETURN_IF_TRUE(NewTipImage == nullptr);
	if (visible && NewTipImage)
	{
		NewTipImage->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		NewTipImage->SetVisibility(ESlateVisibility::Visible);
	}
}
