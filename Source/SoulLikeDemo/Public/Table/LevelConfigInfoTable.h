// LevelConfigInfoTable.h

#pragma once

#include "CoreMinimal.h"
#include "BaseDataTable.h"
#include "SoulLikeGameGlobal.h"
#include "LevelConfigInfoTable.generated.h"

UCLASS(BlueprintType)
class SOULLIKEDEMO_API ULevelConfigInfoTable : public UBaseDataTable
{
	GENERATED_BODY()

public:
	virtual bool InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset) override;
	virtual bool PostExecuteData() override;
	virtual UBaseDataTable* GetDataTable() const override;

	/** 根据关卡ID获取关卡配置 */
	UFUNCTION(BlueprintPure, Category = "LevelConfig")
		bool GetLevelConfig(int32 LevelID, FLevelConfigInfo& OutConfig) const;

	/** 获取所有关卡ID列表（升序） */
	UFUNCTION(BlueprintPure, Category = "LevelConfig")
		TArray<int32> GetAllLevelIDs() const;

	/** 获取关卡显示名称 */
	UFUNCTION(BlueprintPure, Category = "LevelConfig")
		FName GetLevelName(int32 LevelID) const;

	/** 获取关卡对应的地图包名（空字符串表示同地图内切换） */
	UFUNCTION(BlueprintPure, Category = "LevelConfig")
		FName GetMapName(int32 LevelID) const;

protected:
	/** 关卡配置缓存 */
	UPROPERTY()
		TMap<int32, FLevelConfigInfo> LevelConfigMap;
};
