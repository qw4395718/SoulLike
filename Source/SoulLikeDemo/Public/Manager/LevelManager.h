// Public/Manager/LevelManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoulLikeGameGlobal.h"
#include "WaveManagerSystem.h"
#include "LevelConfigInfoTable.h"
#include "LevelManager.generated.h"

class UParticleSystem;
class USoundBase;
class UDataTableManager;

UCLASS()
class SOULLIKEDEMO_API ALevelManager : public AActor
{
	GENERATED_BODY()

public:
	ALevelManager();

protected:
	virtual void BeginPlay() override;

public:
	// ===== 波次事件回调 =====
	UFUNCTION()
		void OnWaveStarted(int32 WaveID);

	UFUNCTION()
		void OnWaveCompleted(int32 WaveID);

	UFUNCTION()
		void OnAllWavesCompleted();

	// ===== 玩家事件 =====
	UFUNCTION()
		void OnPlayerDied();

	// ===== 关卡控制 =====
	UFUNCTION(BlueprintCallable, Category = "Level")
		void StartLevel(int32 LevelID, int32 PlayerClassID = 1);

	UFUNCTION(BlueprintCallable, Category = "Level")
		void RetryLevel();

	UFUNCTION(BlueprintCallable, Category = "Level")
		void GoToNextLevel();

	UFUNCTION(BlueprintCallable, Category = "Level")
		int32 GetCurrentLevelID() const { return CurrentLevelID; };

	UFUNCTION(BlueprintCallable, Category = "Level")
		int32 GetNextLevelID() const { return CurrentLevelConfig.NextLevelID; };

	UFUNCTION(BlueprintCallable, Category = "Level")
		int32 GetCurrentPlayerClassID() const { return CurrentPlayerClassID; };

protected:
	// ===== UI显示 =====
	void ShowLevelStartUI(int32 LevelID);
	void ShowWaveStartUI(const FWaveConfigInfo& WaveConfig);
	void ShowWaveCompleteUI(int32 WaveID);
	void ShowLevelCompleteUI();
	void ShowPlayerDiedUI();
	void UpdateWaveHUD();

	// ===== 玩家状态管理 =====
	/** 重置玩家状态（传送回出生点 + 全恢复） */
	void ResetPlayerState();

	/** 将玩家传送到出生点 */
	void TeleportPlayerToStart(class ASL_CharacterBase* InPlayerCharacter);

	// ===== 特效控制 =====
	void PlayWaveCompleteEffect();
	void PlayLevelCompleteEffect();
	void PlayPlayerDiedEffect();

	// ===== 存档管理 =====
	void SaveGameProgress();
	void UnlockNextLevel(int32 InNextLevelID);

protected:
	// ===== 配置 =====
	UPROPERTY(EditDefaultsOnly, Category = "Level|Config")
		int32 CurrentLevelID;
	
	UPROPERTY(EditDefaultsOnly, Category = "Level|Config")
		FLevelConfigInfo CurrentLevelConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Level|Config")
		int32 CurrentPlayerClassID;

	/** 关卡配置表缓存引用 */
	UPROPERTY()
		ULevelConfigInfoTable* LevelConfigTable;

	/** 从 LevelTable 加载关卡配置 */
	void LoadLevelConfig(int32 LevelID);

	// ===== 特效 =====
	UPROPERTY(EditDefaultsOnly, Category = "Level|Effects")
		UParticleSystem* WaveCompleteEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Level|Effects")
		UParticleSystem* LevelCompleteEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Level|Effects")
		UParticleSystem* PlayerDiedEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Level|Effects")
		USoundBase* WaveCompleteSound;

	UPROPERTY(EditDefaultsOnly, Category = "Level|Effects")
		USoundBase* LevelCompleteSound;

	UPROPERTY(EditDefaultsOnly, Category = "Level|Effects")
		USoundBase* PlayerDiedSound;
};