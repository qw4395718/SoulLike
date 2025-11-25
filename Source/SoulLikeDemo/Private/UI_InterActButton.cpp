// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_InterActButton.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Components/Image.h"
#include "SL_Macros.h"
#include "Components/TextBlock.h"

UUI_InterActButton::UUI_InterActButton(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	// 默认选中状态不可视

}

void UUI_InterActButton::UpdateInterActBtnInfo(UTexture2D* showIcon, FString showText)
{
	RETURN_IF_TRUE(m_interActIcon == nullptr || m_interActDesc == nullptr);
	m_interActIcon->SetBrushFromTexture(showIcon);
	m_interActDesc->SetText(FText::FromString(showText));
}
