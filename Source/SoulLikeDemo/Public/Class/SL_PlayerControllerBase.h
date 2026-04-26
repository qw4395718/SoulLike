// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include <SoulLikeGameGlobal.h>
#include "SL_PlayerControllerBase.generated.h"

class UUIManagerSubsystem;
class USL_ComboManagerComponent;

UCLASS()
class SOULLIKEDEMO_API ASL_PlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	// 构造函数
	ASL_PlayerControllerBase();

protected:
	/************************************************************************/
	/*                               继承实现                                       */
	/************************************************************************/

	// 游戏开始或控制器生成时调用
	virtual void BeginPlay() override;

	// 当控制器控制一个Pawn时调用
	virtual void OnPossess(APawn* InPawn) override;

	// 当控制器放弃控制一个Pawn时调用
	virtual void OnUnPossess() override;

	virtual void SetupInputComponent() override;

public:
	/************************************************************************/
	/*                               外部调用                                       */
	/************************************************************************/

	// 设置输入模式
	UFUNCTION(BlueprintCallable, Category = "Input")
		void SetInputModeUIOnly(UWidget* InWidgetToFocus = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Input")
		void SetInputModeGameOnly();

	UFUNCTION(BlueprintCallable, Category = "Input")
		void SetInputModeGameAndUI(UWidget* InWidgetToFocus = nullptr, bool bLockMouseToViewport = false);

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
	/************************************************************************/
	/*                               内部调用                                       */
	/************************************************************************/

	void OnLightAttackPressed();
	void OnLightAttackReleased();

	void OnHeavyAttackPressed();
	void OnHeavyAttackReleased();

	void OnSpecialAttackPressed();

	void OnDodgePressed();

	/** 安全获取 ComboManager */
	USL_ComboManagerComponent* GetComboManagerComponent() const;

	/** 通用输入处理，所有输入都走这个方法 */
	void ProcessComboInput(EComboInputActionType InputType);

private:
	// 是否已经创建UI
	bool bIsUIInitialized;
	UPROPERTY()
	UUIManagerSubsystem* UIManager; 

	// 缓存 ComboManager 引用，避免频繁 FindComponent
	mutable TWeakObjectPtr<USL_ComboManagerComponent> CachedComboManager;
};