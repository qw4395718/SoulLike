// Public/AI/EQS_Test_CanSeeTarget.h

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EQS_Test_CanSeeTarget.generated.h"

/**
 * EQS测试：检测目标是否可见（视线检测）
 * 使用射线检测判断是否有障碍物遮挡
 */
UCLASS()
class SOULLIKEDEMO_API UEQS_Test_CanSeeTarget : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEQS_Test_CanSeeTarget();

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	bool PerformLineOfSightCheck(class ASL_EnemyBase* Enemy, AActor* Target) const;
	/** 视线检测时忽略的Actor类 */
	UPROPERTY(EditDefaultsOnly, Category = "Test")
		TArray<TSubclassOf<AActor>> ActorsToIgnore;
};