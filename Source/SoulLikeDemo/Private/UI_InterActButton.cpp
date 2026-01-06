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

void UUI_InterActButton::UpdateInterActBtnInfo(int32 index, UTexture2D* showIcon, FString showText)
{
	RETURN_IF_TRUE(m_interActIcon == nullptr || m_interActDesc == nullptr || index < 0);
	AssignedIndex = index;
	m_interActIcon->SetBrushFromTexture(showIcon);
	m_interActDesc->SetText(FText::FromString(showText));
	UE_LOG(LogTemp, Log, TEXT("UUI_InterActButton::UpdateInterActBtnInfo index:%d"),
		index);

}

void UUI_InterActButton::SetSelected(bool bIsSelected, bool bPlayAnimation /*= true*/)
{
	if (bIsSelected == bSelected) return; // 避免重复状态

	bSelected = bIsSelected;

	if (m_interActDesc)
	{
		FSlateColor NewColor = bSelected ?
			FSlateColor(SelectedColor) : FSlateColor(NormalColor);
		m_interActDesc->SetColorAndOpacity(NewColor);
	}

	// 播放动画
	if (bPlayAnimation)
	{
		PlaySelectionAnimation(bIsSelected);
	}
}

int32 UUI_InterActButton::GetAssignedIndex()
{
	return AssignedIndex;
}

void UUI_InterActButton::PlaySelectionAnimation_Implementation(bool bIsSelected)
{
	if (bIsSelected && SelectionAnimation)
	{
		PlayAnimation(SelectionAnimation);
	}
	else if (!bIsSelected && DeselectionAnimation)
	{
		PlayAnimation(DeselectionAnimation);
	}
}
