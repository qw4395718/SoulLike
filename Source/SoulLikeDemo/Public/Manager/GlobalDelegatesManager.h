// GlobalDelegatesManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GlobalDelegatesManager.generated.h"

// 前向声明
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnClickInterActBtnToDialog, const FString&, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnClickInterMainMenuButton, const int);

// 血量变动委托
DECLARE_MULTICAST_DELEGATE_FiveParams(FAttributeHealthChangedEvent,AActor*, float, float, float, float);

// 耐力变动委托
DECLARE_MULTICAST_DELEGATE_FiveParams(FAttributeStaminaChangedEvent, AActor*, float, float, float, float);

// 死亡事件委托
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCharacterDiedEvent, AActor* DeadActor, AActor* Instigator);

// 复活事件委托
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterRevivedEvent, AActor* RevivedActor);

// ===== 道具使用委托 =====
// 道具使用成功（玩家使用了某个道具）
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemUsedEvent, AActor* UserActor, FName ItemID);

// 道具效果触发（GE已经应用到目标）
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemEffectTriggeredEvent, AActor* UserActor, FName ItemID, AActor* TargetActor);

// 道具数量变更
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemCountChangedEvent, AActor* OwnerActor, FName ItemID, int32 NewCount);

UCLASS(BlueprintType, Blueprintable)
class SOULLIKEDEMO_API UGlobalDelegatesManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	// 获取单例实例
	UFUNCTION(BlueprintPure, Category = "Global Delegates", meta = (WorldContext = "WorldContextObject"))
		static UGlobalDelegatesManager* Get(const UObject* WorldContextObject);

	/************************************************************************/
	/*                              委托实例                                        */
	/************************************************************************/
	// 交互按钮行为委托
		FOnClickInterActBtnToDialog OnClickInterActBtnToDialog;
	// 菜单按钮行为委托
		FOnClickInterMainMenuButton OnClickInterMainMenuButton;
	// 血量变动响应委托
		FAttributeHealthChangedEvent OnAttributeHealthChanged;
	// 精力变动响应委托
		FAttributeStaminaChangedEvent OnAttributeStaminaChanged;
	// 死亡事件委托
		FOnCharacterDiedEvent OnCharacterDied;
	// 复活事件委托
		FOnCharacterRevivedEvent OnCharacterRevived;
	// ===== 道具委托 =====
	// 道具使用事件
	FOnItemUsedEvent OnItemUsed;
	// 道具效果触发事件
	FOnItemEffectTriggeredEvent OnItemEffectTriggered;
	// 道具数量变更事件
	FOnItemCountChangedEvent OnItemCountChanged;

	/**
	 * 委托响应函数
	 */
	// 交互按钮行为响应
	UFUNCTION(BlueprintCallable, Category = "Global Delegates")
		void BroadcastDialogShow(const FString& labelText, const FString& descText);

	// 菜单按钮行为响应
	UFUNCTION(BlueprintCallable, Category = "Global Delegates")
		void BroadcastOnClickMainMenuButton(const int widgetIndex);

		// ===== 道具委托广播 =====
	// 广播道具使用事件
	UFUNCTION(BlueprintCallable, Category = "Global Delegates|Item")
		void BroadcastItemUsed(AActor* InUserActor, FName InItemID);

	// 广播道具效果触发事件
	UFUNCTION(BlueprintCallable, Category = "Global Delegates|Item")
		void BroadcastItemEffectTriggered(AActor* InUserActor, FName InItemID, AActor* InTargetActor);

	// 广播道具数量变更事件
	UFUNCTION(BlueprintCallable, Category = "Global Delegates|Item")
		void BroadcastItemCountChanged(AActor* InOwnerActor, FName InItemID, int32 InNewCount);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 解除所有委托绑定
	void ClearAllBinding();
};