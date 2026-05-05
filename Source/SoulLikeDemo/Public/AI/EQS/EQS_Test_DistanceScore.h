// Public/AI/EQS_Test_DistanceScore.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include <EnvironmentQuery/Tests/EnvQueryTest_Distance.h>
#include "EQS_Test_DistanceScore.generated.h"

/**
 * EQS测试：根据距离评分
 * 越近的目标得分越高（用于选择最近的攻击目标）
 */
UCLASS()
class SOULLIKEDEMO_API UEQS_Test_DistanceScore : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEQS_Test_DistanceScore();

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	float CalculateScore(float Distance) const;
	/** 距离评分模式 */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
		TEnumAsByte<EEnvTestDistance::Type> DistanceMode;

	/** 理想距离（在此距离内得满分） */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
		float IdealDistance;
};