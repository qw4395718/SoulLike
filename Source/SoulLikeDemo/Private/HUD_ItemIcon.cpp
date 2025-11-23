// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ItemIcon.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

UHUD_ItemIcon::UHUD_ItemIcon()
{
}

void UHUD_ItemIcon::SetData(FStatusEffectInfo effectInfo)
{
	RETURN_IF_TRUE(m_stackNum == nullptr || m_showImage == nullptr);
	// ÉèÖÃ¶ÑµþÊýÁ¿
	m_stackNum->SetText(FText::AsNumber(effectInfo.Stacks));
	m_showImage->SetBrushFromTexture(effectInfo.Icon);
}

void UHUD_ItemIcon::ClearData()
{
	RETURN_IF_TRUE(m_stackNum == nullptr|| m_showImage == nullptr);
	m_stackNum->SetText(FText::AsNumber(0));
	m_showImage->SetVisibility(ESlateVisibility::Hidden);
}


