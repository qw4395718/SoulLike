// Public/Save/USL_GameSaveData.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "USL_GameSaveData.generated.h"

/**
 * 游戏存档数据结构
 * 存档保存在本地磁盘上
 */
UCLASS()
class SOULLIKEDEMO_API USL_GameSaveData : public USaveGame
{
	GENERATED_BODY()

public:
	USL_GameSaveData();

	/************************************************************************/
	/*                               外部访问                               */
	/************************************************************************/
	// 已解锁的最大关卡ID（从1开始）
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
		int32 SavedLevelID;

	// 玩家选择的职业ID
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
		int32 SavedPlayerClassID;

	// 存档时间
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
		FDateTime SaveTime;

	// 重置为默认值
	void ResetToDefault();
};