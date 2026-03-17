// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SL_PlayerControllerBase.generated.h"

class UUIManagerSubsystem;

UCLASS()
class SOULLIKEDEMO_API ASL_PlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	// 构造函数
	ASL_PlayerControllerBase();

protected:
	// 游戏开始或控制器生成时调用
	virtual void BeginPlay() override;

	// 当控制器控制一个Pawn时调用
	virtual void OnPossess(APawn* InPawn) override;

	// 当控制器放弃控制一个Pawn时调用
	virtual void OnUnPossess() override;

	// 设置输入模式
	UFUNCTION(BlueprintCallable, Category = "Input")
		void SetInputModeUIOnly(UWidget* InWidgetToFocus = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Input")
		void SetInputModeGameOnly();

	UFUNCTION(BlueprintCallable, Category = "Input")
		void SetInputModeGameAndUI(UWidget* InWidgetToFocus = nullptr, bool bLockMouseToViewport = false);

public:
	// 玩家状态变化事件
	UFUNCTION(BlueprintImplementableEvent, Category = "Player")
		void OnPlayerStateChanged();

	// 获取当前控制的角色
	UFUNCTION(BlueprintPure, Category = "Player")
		class ASL_CharacterBase* GetMyPlayerCharacter() const;

	// 创建玩家状态UI
	UFUNCTION(BlueprintCallable, Category = "UI")
		void CreatePlayerStatusUI();

	// 销毁玩家状态UI
	UFUNCTION(BlueprintCallable, Category = "UI")
		void DestroyPlayerStatusUI();

protected:


private:
	// 是否已经创建UI
	bool bIsUIInitialized;
	UPROPERTY()
	UUIManagerSubsystem* UIManager; 
};