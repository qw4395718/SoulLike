// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulLikeDemo.h"

#if WITH_EDITOR
#include "Misc/CoreDelegates.h"
#include "Engine/AssetManager.h"
#endif

IMPLEMENT_PRIMARY_GAME_MODULE(FSoulLikeDemoModule, SoulLikeDemo, "SoulLikeDemo");

void FSoulLikeDemoModule::StartupModule()
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		// 引擎初始化完成后 AssetManager 才可用，此处用 OnPostEngineInit 延迟
		FCoreDelegates::OnPostEngineInit.AddLambda([]()
		{
			TArray<FSoftObjectPath> AssetsToLoad;

			AssetsToLoad.Add(FSoftObjectPath(
				TEXT("/Game/SoulLikeDemo/Blueprints/GameMode/ALS_GameMode_SoulLikeSP.ALS_GameMode_SoulLikeSP_C")
			));
			AssetsToLoad.Add(FSoftObjectPath(
				TEXT("/Game/SoulLikeDemo/Blueprints/CharacterLogic/ALS_SoulLike_CharacterBP.ALS_SoulLike_CharacterBP_C")
			));
			AssetsToLoad.Add(FSoftObjectPath(
				TEXT("/Game/SoulLikeDemo/Blueprints/CharacterLogic/SoulLike_Base_CharacterBP.SoulLike_Base_CharacterBP_C")
			));
			AssetsToLoad.Add(FSoftObjectPath(
				TEXT("/Game/SoulLikeDemo/Blueprints/CharacterLogic/SoulLike_AnimMan_CharacterBP.SoulLike_AnimMan_CharacterBP_C")
			));
			AssetsToLoad.Add(FSoftObjectPath(
				TEXT("/Game/AdvancedLocomotionV4/Blueprints/CharacterLogic/ALS_Player_Controller.ALS_Player_Controller_C")
			));

			UAssetManager::GetStreamableManager().RequestAsyncLoad(
				AssetsToLoad,
				FStreamableDelegate::CreateLambda([]()
				{
					UE_LOG(LogTemp, Log, TEXT("[Preload] 编辑器启动预加载完成"));
				})
			);

			UE_LOG(LogTemp, Log, TEXT("[Preload] 编辑器启动预加载已发起"));
		});
	}
#endif
}

void FSoulLikeDemoModule::ShutdownModule()
{
}
