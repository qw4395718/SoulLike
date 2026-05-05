// Public/AI/EQS_Generator_EnemyTargets.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include <DataProviders/AIDataProvider.h>
#include <EnvironmentQuery/EnvQueryContext.h>
#include "EQS_Generator_EnemyTargets.generated.h"

/**
 * EQS生成器：生成所有潜在的敌人目标（玩家）
 * 在感知范围内生成所有可能的攻击目标
 */
UCLASS()
class SOULLIKEDEMO_API UEQS_Generator_EnemyTargets : public UEnvQueryGenerator
{
	GENERATED_BODY()

public:
	UEQS_Generator_EnemyTargets();

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	/** 搜索半径（从查询上下文位置开始） */
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
		FAIDataProviderFloatValue SearchRadius;

	/** 搜索中心（默认为查询者的位置） */
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
		TSubclassOf<UEnvQueryContext> SearchCenter;
};