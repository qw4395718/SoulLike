// Public/AI/BTService/BTService_UpdateTarget.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "BTService_UpdateTarget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SOULLIKEDEMO_API UBTService_UpdateTarget : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdateTarget();

    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    /** 搜索目标用的EQS模板 */
    UPROPERTY(EditAnywhere, Category = "EQS")
        class UEnvQuery* EQSQueryTemplate;

    /** 输出目标到黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector TargetActorKey;

    // 输出目标点位置到黑板键
	UPROPERTY(EditAnywhere, Category = "Blackboard")
		struct FBlackboardKeySelector TargetActorLocationKey;

	// 输出是否有目标到黑板键
	UPROPERTY(EditAnywhere, Category = "Blackboard")
		struct FBlackboardKeySelector HasTargetStateKey;

	// 输出是否在攻击范围内到黑板键
	UPROPERTY(EditAnywhere, Category = "Blackboard")
		struct FBlackboardKeySelector InAttackRangeStateKey;

    /** 目标丢失超时时间（秒） */
    UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
        float LoseTargetTimeout = 5.0f;

    /** EQS查询频率 */
    UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.1"))
        float SearchInterval = 0.5f;

    /** EQS查询完成的回调 */
    void OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result);

private:
    float TimeSinceLastSearch = 0.0f;
    float TimeTargetLost = 0.0f;

    // 标记当前是否有EQS查询正在进行（防止重复查询）
    bool bIsQuerying = false;

	// 对于异步EQS查询，存储当前的行为树组件引用
	UBehaviorTreeComponent* CachedBTComp;
};