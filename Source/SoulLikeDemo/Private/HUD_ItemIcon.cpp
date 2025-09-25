// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ItemIcon.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void UHUD_ItemIcon::InitializeIcon(FStatusEffectInfo EffectInfo)
{
	// 异步加载美术资源
	if (EffectInfo.IconPath == "") { return; }
	// 初始化参数
	IconIndex = EffectInfo.IconIndex;
	StatusStacksNum = EffectInfo.Stacks;

	// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	SoftImageReference = TSoftObjectPtr<UImage>(FSoftObjectPath(*(EffectInfo.IconPath)));
	Streamable.RequestAsyncLoad(
		SoftImageReference.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UHUD_ItemIcon::OnLoadedImage)
	);

}

void UHUD_ItemIcon::UpdateIcon(float RemainingTime, int32 Stacks)
{
	if (RemainingTime <= HUD_STATUSBAR_STATUSICONFLASHING && IsFlashing == false)
	{
		// 进入闪烁状态
		PlayFlashingEffect();
	}
	StatusStacksNum = Stacks;
}

bool UHUD_ItemIcon::EqualIconIndex(int Index)
{
	return (IconIndex == Index && IconIndex != 0);
}

void UHUD_ItemIcon::OnLoadedImage()
{
	if (SoftImageReference.Get() != nullptr)
	{
		ImageIcon = SoftImageReference.Get();
	}
}
