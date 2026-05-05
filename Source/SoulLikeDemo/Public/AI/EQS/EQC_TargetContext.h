// Public/AI/EQC_TargetContext.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EQC_TargetContext.generated.h"

/**
 * EQS上下文：获取敌人当前目标
 * 提供给EQS查询使用的上下文，用于获取当前目标的Actor
 */
UCLASS()
class SOULLIKEDEMO_API UEQC_TargetContext : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};