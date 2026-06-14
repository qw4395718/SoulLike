// Public/Class/SL_PlayerControllerBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include <SoulLikeGameGlobal.h>
#include "SL_PlayerControllerBase.generated.h"

class UUIManagerSubsystem;
class USL_ComboManagerComponent;
class USL_EquipmentComponent;
class USL_InventoryComponent;
class UHUD_ItemUseUI;
class ASL_SummonSign;
class ASL_PhantomCharacter;
class USL_SummonSessionComponent;
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

    /************************************************************************/
    /*                         联机RPC：游戏状态操作                         */
    /************************************************************************/

    // 路由入口：请求新游戏（客户端→Server RPC，服务器→直接调用）
    UFUNCTION(BlueprintCallable, Category = "Game")
    void RequestNewGame();

    // 路由入口：请求加载存档
    UFUNCTION(BlueprintCallable, Category = "Game")
    void RequestLoadGame();

    // 路由入口：请求加载指定关卡
    UFUNCTION(BlueprintCallable, Category = "Game")
    void RequestLoadLevel(int32 InLevelID);

    // Server RPC：客户端请求新游戏
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RequestNewGame();

    // Server RPC：客户端请求加载存档
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RequestLoadGame();

    // Server RPC：客户端请求加载指定关卡
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RequestLoadLevel(int32 InLevelID);

    // Server RPC：客户端请求使用道具（服务器再做权威检查并执行）
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UseItem(FName InItemID);

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

	// ===== 道具使用 =====
	// 使用当前选中的道具（绑定"R"键）
	void OnUseItemPressed();

	// 切换到上一个道具（绑定"Q"键）
	void OnSelectPrevItemPressed();

	// 切换到下一个道具（绑定"E"键）
	void OnSelectNextItemPressed();

	// 交互（绑定"G"键）
	void OnInteractPressed();

	// 客户端定期扫描附近的召唤标记（用于显示交互提示）
	void Client_CheckNearbySummonSigns();
	void UpdateSummonSignPrompt(class ASL_SummonSign* InSign);

	// 安全获取 ComboManager
	USL_ComboManagerComponent* GetComboManagerComponent() const;

	USL_EquipmentComponent* GetEquipmentComponent() const;

	// 安全获取 InventoryComponent
	USL_InventoryComponent* GetInventoryComponent() const;

	// 从 UIManager 获取道具使用界面控件
	UHUD_ItemUseUI* GetItemUseUIWidget() const;

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

	// 联机召唤会话组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USL_SummonSessionComponent* SummonSessionCmp;

	// 当前目标召唤标记（用于交互提示）
	UPROPERTY()
		ASL_SummonSign* CurrentSummonSignTarget;

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

	FTimerHandle SummonSignScanTimerHandle;
};