#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Struct/OnlineSummonStructs.h"
#include "SL_SignManager.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSignRegistered, const FSummonSignInfo& /*SignInfo*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSignRemoved, const FGuid& /*SignID*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSignInteracted, const FGuid& /*SignID*/, const FString& /*InteractorName*/);

/**
 * 召唤标记管理器（GameInstanceSubsystem）
 * 维护当前游戏中所有活跃的召唤标记列表，提供注册/查询/移除接口
 * Phase1 中运行在单服实例中，Phase2+ 扩展为跨服匹配
 */
UCLASS()
class SOULLIKEDEMO_API USL_SignManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/************************************************************************/
	/*                               外部调用                               */
	/************************************************************************/
	// 注册一个新的召唤标记，返回分配的 SignID
	UFUNCTION(BlueprintCallable, Category = "SignManager")
	FGuid RegisterSign(const FSummonSignInfo& InSignInfo);

	// 移除一个召唤标记
	UFUNCTION(BlueprintCallable, Category = "SignManager")
	bool UnregisterSign(const FGuid& InSignID);

	// 根据关卡名查询该关卡中的所有活跃标记
	UFUNCTION(BlueprintPure, Category = "SignManager")
	TArray<FSummonSignInfo> QuerySignsByLevel(FName InLevelName) const;

	// 根据等级范围过滤查询标记
	UFUNCTION(BlueprintPure, Category = "SignManager")
	TArray<FSummonSignInfo> QuerySignsByLevelRange(FName InLevelName, int32 InPlayerLevel, int32 InWeaponLevel, const FSummonMatchConfig& InMatchConfig) const;

	// 获取指定标记的信息
	UFUNCTION(BlueprintPure, Category = "SignManager")
	bool GetSignInfo(const FGuid& InSignID, FSummonSignInfo& OutSignInfo) const;

	// 报告标记被交互（锁定标记，防止重复召唤）
	UFUNCTION(BlueprintCallable, Category = "SignManager")
	bool ReportSignInteraction(const FGuid& InSignID, const FString& InInteractorName);

	// 获取当前所有活跃标记数量
	UFUNCTION(BlueprintPure, Category = "SignManager")
	int32 GetActiveSignCount() const { return ActiveSigns.Num(); }

	// 每帧更新过期标记
	void TickSigns(float DeltaTime);

	// 清理所有过期标记
	void CleanExpiredSigns();

public:
	// 事件委托
	FOnSignRegistered OnSignRegistered;
	FOnSignRemoved OnSignRemoved;
	FOnSignInteracted OnSignInteracted;

protected:
	// 活跃标记列表（SignID → SignInfo）
	UPROPERTY()
	TMap<FGuid, FSummonSignInfo> ActiveSigns;

	// 默认标记存在时间（秒）
	UPROPERTY(EditDefaultsOnly, Category = "SignManager|Config")
	float DefaultSignDuration = 300.0f;
};
