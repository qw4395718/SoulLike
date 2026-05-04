// Public/Table/EnemyConfigInfoTable.h

#pragma once

#include "CoreMinimal.h"
#include "BaseDataTable.h"
#include "SoulLikeGameGlobal.h"
#include "EnemyConfigInfoTable.generated.h"

UCLASS(BlueprintType)
class SOULLIKEDEMO_API UEnemyConfigInfoTable : public UBaseDataTable
{
	GENERATED_BODY()

public:
	virtual bool InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset) override;
	virtual bool PostExecuteData() override;
	virtual UBaseDataTable* GetDataTable() const override;

	/** 根据敌人ID获取配置 */
	UFUNCTION(BlueprintPure, Category = "EnemyConfig")
		bool GetEnemyConfig(int32 EnemyID, FEnemyConfigInfo& OutConfig) const;

	/** 根据敌人类型获取配置列表 */
	UFUNCTION(BlueprintPure, Category = "EnemyConfig")
		TArray<FEnemyConfigInfo> GetEnemiesByType(EEnemyType EnemyType) const;

	/** 获取所有敌人ID列表 */
	UFUNCTION(BlueprintPure, Category = "EnemyConfig")
		TArray<int32> GetAllEnemyIDs() const;

protected:
	/** 敌人数据缓存：EnemyID -> Config */
	UPROPERTY()
		TMap<int32, FEnemyConfigInfo> EnemyConfigMap;

	/** 类型索引：EnemyType -> EnemyID列表 */
	TMap<EEnemyType, TArray<int32>> EnemyTypeIndex;
};