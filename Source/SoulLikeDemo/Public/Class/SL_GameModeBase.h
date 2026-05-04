// Public/Game/SL_GameModeBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SL_GameModeBase.generated.h"

class ALevelManager;

UCLASS()
class SOULLIKEDEMO_API ASL_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASL_GameModeBase();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ===== 关卡控制 =====
	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void StartCurrentLevel();

	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void RestartLevel();

	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void LoadLevel(int32 LevelID);

	// ===== 获取引用 =====
	UFUNCTION(BlueprintPure, Category = "GameMode")
		ALevelManager* GetLevelManager() const { return LevelManager; }

protected:
	/** 创建 LevelManager */
	void CreateLevelManager();

	/** 初始化玩家 */
	void InitializePlayer(int32 PlayerClassID);

protected:
	UPROPERTY()
		ALevelManager* LevelManager;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode|Config")
		TSubclassOf<ALevelManager> LevelManagerClass;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode|Config")
		float LevelStartDelay = 2.0f;
};