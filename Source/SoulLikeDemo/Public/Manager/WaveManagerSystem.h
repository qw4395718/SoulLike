// Public/Manager/WaveManagerSystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SoulLikeGameGlobal.h"
#include "WaveManagerSystem.generated.h"

class ASL_EnemyBase;

/** 波次状态 */
UENUM(BlueprintType)
enum class EWaveState : uint8
{
	None,
	Preparing,    // 准备中
	Spawning,     // 生成中
	Active,       // 战斗中
	Completed,    // 已完成
	Failed        // 失败
};

/** 波次事件委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStateChanged, int32, WaveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompleted);

UCLASS()
class SOULLIKEDEMO_API UWaveManagerSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ===== 关卡流程控制 =====
	/** 开始关卡流程 */
	UFUNCTION(BlueprintCallable, Category = "WaveManagerSystem")
		void StartLevel(int32 LevelID);

	/** 开始下一波 */
	UFUNCTION(BlueprintCallable, Category = "WaveManagerSystem")
		void StartNextWave();

	/** 暂停波次 */
	UFUNCTION(BlueprintCallable, Category = "WaveManagerSystem")
		void PauseWave();

	/** 继续波次 */
	UFUNCTION(BlueprintCallable, Category = "WaveManagerSystem")
		void ResumeWave();

	/** 重置关卡 */
	UFUNCTION(BlueprintCallable, Category = "WaveManagerSystem")
		void ResetLevel();

	// ===== 查询接口 =====
	UFUNCTION(BlueprintPure, Category = "WaveManagerSystem")
		int32 GetCurrentWaveID() const { return CurrentWaveID; }

	UFUNCTION(BlueprintPure, Category = "WaveManagerSystem")
		int32 GetTotalWaveCount() const { return TotalWaveCount; }

	UFUNCTION(BlueprintPure, Category = "WaveManagerSystem")
		int32 GetRemainingEnemyCount() const { return ActiveEnemies.Num(); }

	UFUNCTION(BlueprintPure, Category = "WaveManagerSystem")
		EWaveState GetCurrentWaveState() const { return CurrentWaveState; }

	UFUNCTION(BlueprintPure, Category = "WaveManagerSystem")
		bool IsAllWavesCompleted() const { return bAllWavesCompleted; }

public:
	// ===== 委托 =====
	UPROPERTY(BlueprintAssignable, Category = "WaveManagerSystem|Events")
		FOnWaveStateChanged OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "WaveManagerSystem|Events")
		FOnWaveStateChanged OnWaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "WaveManagerSystem|Events")
		FOnAllWavesCompleted OnAllWavesCompleted;

protected:
	// ===== 内部方法 =====
	/** 加载当前关卡的波次配置 */
	bool LoadWaveConfig(int32 LevelID);

	/** 生成怪物 */
	void SpawnWaveMonsters(FWaveConfigInfo& WaveConfig);

	/** 在指定生成点生成怪物 */
	ASL_EnemyBase* SpawnEnemyAtPoint(int32 SpawnerID, const FTransform& SpawnTransform);

	/** 检查波次是否完成（所有怪物死亡） */
	void CheckWaveCompletion();

	/** 怪物死亡回调 */
	UFUNCTION()
		void OnEnemyDiedCallback();

	/** 波次完成处理 */
	void OnWaveCompletedInternal();

	/** 所有波次完成 */
	void OnAllWavesCompletedInternal();

	/** 获取地图上的生成点 */
	TArray<FSpawnPointInfo> GetSpawnPointsForLevel(int32 LevelID);

protected:
	// ===== 配置 =====
	UPROPERTY()
		TArray<FWaveConfigInfo> WaveConfigs;

	int32 CurrentWaveIndex;
	int32 CurrentWaveID;
	int32 TotalWaveCount;

	EWaveState CurrentWaveState;
	bool bAllWavesCompleted;

	// ===== 怪物管理 =====
	UPROPERTY()
		TArray<ASL_EnemyBase*> ActiveEnemies;

	UPROPERTY()
		TArray<ASL_EnemyBase*> AllSpawnedEnemies;

	// ===== 生成参数 =====
	FTimerHandle SpawnTimerHandle;
	int32 PendingSpawnCount;
	int32 SpawnedCount;
	TArray<int32> SpawnerIDs;
};