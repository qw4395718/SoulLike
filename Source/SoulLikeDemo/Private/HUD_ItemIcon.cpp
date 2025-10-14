// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ItemIcon.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

UHUD_ItemIcon::UHUD_ItemIcon()
{
	ItemIndex = 0;
	ItemName = "";
	ItemDesc = "";
}

void UHUD_ItemIcon::SetData(FStatusEffectInfo EffectInfo)
{
	// 异步加载美术资源
	if (EffectInfo.IconPath == "") { return; }
	// 初始化参数
	ItemIndex = EffectInfo.IconIndex;
	// 设置堆叠数量
	SetStacksNum(EffectInfo.Stacks);

	// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	SoftImageReference = TSoftObjectPtr<UImage>(FSoftObjectPath(*(EffectInfo.IconPath)));
	Streamable.RequestAsyncLoad(
		SoftImageReference.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UHUD_ItemIcon::OnLoadedImage)
	);

}

void UHUD_ItemIcon::ClearData()
{
	ItemIndex = 0;
	ItemName = "";
	ItemDesc = "";
	SetStacksNum(0);
	if (ShowImage != nullptr)
	{// 设置不可视
		ShowImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

bool UHUD_ItemIcon::EqualIconIndex(int Index)
{
	return (ItemIndex == Index && ItemIndex != 0);
}

