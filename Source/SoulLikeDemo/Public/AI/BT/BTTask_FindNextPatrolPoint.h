// Public/AI/BTTask_FindNextPatrolPoint.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindNextPatrolPoint.generated.h"

/**
 * 寻找下一个巡逻点
 * 需要黑板键：PatrolPathIndex, PatrolPoints, TargetLocation
 */
UCLASS()
class SOULLIKEDEMO_API UBTTask_FindNextPatrolPoint : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_FindNextPatrolPoint();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    /** 巡逻点数组 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector PatrolPointsKey;

    /** 当前巡逻点索引 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector PatrolIndexKey;

    /** 目标位置（输出） */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector TargetLocationKey;

    /** 是否循环巡逻 */
    UPROPERTY(EditAnywhere, Category = "Config")
        bool bLoopPatrol = true;
};