// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_MenuItem.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "SL_Macros.h"
#include "Components/Image.h"


UUI_MenuItem::UUI_MenuItem()
{

}

void UUI_MenuItem::SetNewAcquireImageVisible(bool visible)
{
	RETURN_IF_TRUE(m_newTipImage == nullptr);
	if (visible && m_newTipImage)
	{
		m_newTipImage->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		m_newTipImage->SetVisibility(ESlateVisibility::Visible);
	}
}
