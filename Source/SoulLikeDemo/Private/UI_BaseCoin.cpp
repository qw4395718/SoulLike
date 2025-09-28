// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_StatusIcon.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"


UUI_BaseCoin::UUI_BaseCoin()
{
	// 默认初始化成员变量
	ShowImage = nullptr;
	SoftImageReference = nullptr;
	PicWidth = 0;
	PicHeight = 0;
	DynamicStr = "";
}

void UUI_BaseCoin::SetLoadImageData(FString ImgPath)
{
	RETURN_IF_TRUE(ImgPath == "");
		// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	SoftImageReference = TSoftObjectPtr<UImage>(FSoftObjectPath(*ImgPath));
	Streamable.RequestAsyncLoad(
		SoftImageReference.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UUI_BaseSlot::OnLoadedImage)
	);

}

void UUI_BaseCoin::SetPicSize(int Width, int Height)
{
	RETURN_IF_TRUE(Width <= 0 || Height <= 0);
	PicWidth = Width;
	PicHeight = Height;
}

void UUI_BaseCoin::SetDynamicStr(FString Str)
{
	DynamicStr = Str;
}

void UUI_BaseCoin::OnLoadedImage()
{
	// 资源使用
	if (SoftImageReference.Get() != nullptr)
	{
		ShowImage = SoftImageReference.Get();
	}
}
