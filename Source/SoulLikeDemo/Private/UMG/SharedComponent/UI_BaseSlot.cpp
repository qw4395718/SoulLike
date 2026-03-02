// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_BaseSlot.h"
#include "SL_Macros.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"


UUI_BaseSlot::UUI_BaseSlot(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

}

void UUI_BaseSlot::SetImageBrush(UTexture2D* icon)
{
	RETURN_IF_TRUE(m_showImage == nullptr || icon == nullptr);
	m_showImage->SetBrushFromTexture(icon,true);
}

void UUI_BaseSlot::SetStacksNum(int num)
{
	RETURN_IF_TRUE(m_stackNum == nullptr);
	m_stackNum->SetText(FText::AsNumber(num));
}
