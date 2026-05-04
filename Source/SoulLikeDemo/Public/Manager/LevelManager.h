// Public/Manager/LevelManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoulLikeGameGlobal.h"
#include "WaveManagerSystem.h"
#include "LevelManager.generated.h"

class UParticleSystem;
class USoundBase;

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

protected:
	// ===== UI显示 =====
	void ShowLevelStartUI(int32 LevelID);
	void ShowWaveStartUI(const FWaveConfigInfo& WaveConfig);
	void ShowWaveCompleteUI(int32 WaveID);
	void ShowLevelCompleteUI();
	void ShowPlayerDiedUI();
	void UpdateWaveHUD();

	// ===== 特效控制 =====
	void PlayWaveCompleteEffect();
	void PlayLevelCompleteEffect();
	void PlayPlayerDiedEffect();

	// ===== 存档管理 =====
	void SaveGameProgress();
	void UnlockNextLevel(int32 NextLevelID);

protected:
	// ===== 配置 =====
	UPROPERTY(EditDefaultsOnly, Category = "Level|Config")
		int32 CurrentLevelID;

	UPROPERTY(EditDefaultsOnly, Category = "Level|Config")
		int32 CurrentPlayerClassID;

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