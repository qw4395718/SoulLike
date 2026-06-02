// Public/Game/SL_GameModeBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SL_GameModeBase.generated.h"

class ALevelManager;
class USL_GameSaveSubsystem;

UCLASS()
class SOULLIKEDEMO_API ASL_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASL_GameModeBase();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ===== 关卡控制（供外部调用） =====
	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void StartTargetLevel(int32 InLevelID);

	// 设置是否使用存档 
    UFUNCTION(BlueprintCallable, Category = "GameMode")
   		void SetUseSaveData(bool bInUseSaveData) { bUseSaveData = bInUseSaveData; }

	// 获取是否使用存档 
    UFUNCTION(BlueprintPure, Category = "GameMode")
    	bool GetUseSaveData() const { return bUseSaveData; }

    // 检查是否存在存档 
    UFUNCTION(BlueprintPure, Category = "GameMode")
  	  	bool HasSaveData() const;

	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void RestartLevel();

	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void LoadLevel(int32 InLevelID);

	// ===== 存档控制（供外部调用） =====
	// 保存当前游戏进度
	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void SaveCurrentProgress();

	// ===== 获取引用 =====
	UFUNCTION(BlueprintPure, Category = "GameMode")
		ALevelManager* GetLevelManager() const { return LevelManager; }

protected:
	/** 创建 LevelManager */
	void CreateLevelManager();

	/** 初始化玩家 */
	void InitializePlayer(int32 InPlayerClassID);

protected:
	/************************************************************************/
	/*                               内部访问                               */
	/************************************************************************/
	UPROPERTY()
		ALevelManager* LevelManager;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode|Config")
		TSubclassOf<ALevelManager> LevelManagerClass;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode|Config")
		float LevelStartDelay = 2.0f;

	// 是否使用存档数据（由主菜单设置）
	bool bUseSaveData;
};