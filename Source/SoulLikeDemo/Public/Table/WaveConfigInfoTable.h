// Public/Table/WaveConfigInfoTable.h

#pragma once

#include "CoreMinimal.h"
#include "BaseDataTable.h"
#include "SoulLikeGameGlobal.h"
#include "WaveConfigInfoTable.generated.h"

UCLASS(BlueprintType)
class SOULLIKEDEMO_API UWaveConfigInfoTable : public UBaseDataTable
{
	GENERATED_BODY()

public:
	virtual bool InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset) override;
	virtual bool PostExecuteData() override;
	virtual UBaseDataTable* GetDataTable() const override;

	/** 根据关卡ID获取所有波次配置 */
	UFUNCTION(BlueprintPure, Category = "WaveConfig")
		bool GetWavesForLevel(int32 LevelID, TArray<FWaveConfigInfo>& OutWaves) const;

	/** 根据波次ID获取波次配置 */
	UFUNCTION(BlueprintPure, Category = "WaveConfig")
		bool GetWaveConfig(int32 WaveID, FWaveConfigInfo& OutConfig) const;

	/** 获取指定关卡的波次数量 */
	UFUNCTION(BlueprintPure, Category = "WaveConfig")
		int32 GetWaveCountForLevel(int32 LevelID) const;

protected:
	/** 关卡索引：LevelID -> WaveID列表 */
	TMap<int32, TArray<int32>> LevelWaveIndex;

	/** 波次数据缓存：WaveID -> WaveConfig */
	UPROPERTY()
		TMap<int32, FWaveConfigInfo> WaveConfigMap;
};