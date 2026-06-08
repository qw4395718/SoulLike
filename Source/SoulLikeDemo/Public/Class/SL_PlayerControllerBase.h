// Public/Class/SL_PlayerControllerBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include <SoulLikeGameGlobal.h>
#include "SL_PlayerControllerBase.generated.h"

class UUIManagerSubsystem;
class USL_ComboManagerComponent;
class USL_EquipmentComponent;
class USL_InventoryComponent;		// ===== 新增：前向声明 =====
class UDamageFloatingTextManagerComponent;

UCLASS()
class SOULLIKEDEMO_API ASL_PlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	// 构造函数
	ASL_PlayerControllerBase();

protected:
	/************************************************************************/
	/*                               继承实现                               */
	/************************************************************************/
	// 游戏开始或控制器生成时调用
	virtual void BeginPlay() override;

	// 控制器结束生命周期时调用
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 当控制器控制一个Pawn时调用
	virtual void OnPossess(APawn* InPawn) override;

	// 当控制器放弃控制一个Pawn时调用
	virtual void OnUnPossess() override;

	virtual void SetupInputComponent() override;

public:
	/************************************************************************/
	/* 输入回调                                                                     */
	/************************************************************************/
	void OnLockOnPressed();
	void OnOpenPauseMenuPressed();

public:
	/************************************************************************/
	/*                               外部调用                               */
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

	// 显示开始界面 
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowBeginPlayScreen();

    // 隐藏开始界面 
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideBeginPlayScreen();

    // 检查是否已经显示开始界面 
    UFUNCTION(BlueprintPure, Category = "UI")
    bool IsBeginPlayScreenVisible() const;

    // ===== 死亡事件 =====
    // 绑定角色死亡事件监听
    void BindPlayerDeathEvent();

    // 角色死亡事件响应
    UFUNCTION()
    void OnPlayerDiedHandler(AActor* InDeadActor, AActor* InInstigator);

protected:
	/************************************************************************/
	/*                               内部调用                               */
	/************************************************************************/
	void OnLightAttackPressed();
	void OnLightAttackReleased();

	void OnHeavyAttackPressed();
	void OnHeavyAttackReleased();

	void OnSpecialAttackPressed();

	void OnDodgePressed();

	// ===== 新增：道具使用 =====
	// 使用当前选中的道具（绑定"E"键）
	void OnUseItemPressed();

	// 安全获取 ComboManager 
	USL_ComboManagerComponent* GetComboManagerComponent() const;

	USL_EquipmentComponent* GetEquipmentComponent() const;

	// ===== 新增：获取背包组件 =====
	// 安全获取 InventoryComponent
	USL_InventoryComponent* GetInventoryComponent() const;

	// 通用输入处理，所有输入都走这个方法 
	void ProcessComboInput(EComboInputActionType InputType);

	// 检查是否有存档并初始化界面 
    void CheckSaveDataAndInitScreen();

protected:
	/************************************************************************/
	/*                               内部访问                                */
	/************************************************************************/
	// 伤害飘字管理器
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UDamageFloatingTextManagerComponent* FloatingTextManager;

private:
    /************************************************************************/
    /*                               内部访问                                */
    /************************************************************************/
	// 是否已经创建UI
	bool bIsUIInitialized;
	UPROPERTY()
		UUIManagerSubsystem* UIManager;

	// 缓存 ComboManager 引用，避免频繁 FindComponent
	mutable TWeakObjectPtr<USL_ComboManagerComponent> CachedComboManager;

	// 缓存 EquipmentComp 引用，避免频繁 FindComponent
	mutable TWeakObjectPtr<USL_EquipmentComponent> CachedEquipmentComp;

	// ===== 新增：缓存 =====
	// 缓存 InventoryComp 引用，避免频繁 FindComponent
	mutable TWeakObjectPtr<USL_InventoryComponent> CachedInventoryComp;

	// ===== 死亡事件 =====
	// 死亡委托句柄（防止重复绑定）
	FDelegateHandle OnPlayerDiedHandle;
};