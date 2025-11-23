// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_BaseCoin.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "SL_Macros.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


UUI_BaseCoin::UUI_BaseCoin(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

}

void UUI_BaseCoin::InitializeUIComp()
{
}

void UUI_BaseCoin::SetImageBrush(UTexture2D* showImage)
{
	RETURN_IF_TRUE(m_showImage == nullptr || showImage == nullptr);
	m_showImage->SetBrushFromTexture(showImage,true);
}

void UUI_BaseCoin::SetDynamicStr(FString Str)
{
	RETURN_IF_TRUE(m_dynamicStr == nullptr);
	m_dynamicStr->SetText(FText::FromString(Str));
}
