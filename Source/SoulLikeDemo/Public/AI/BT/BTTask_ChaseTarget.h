// Public/AI/BTTask_ChaseTarget.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ChaseTarget.generated.h"

/**
 * 追击目标
 * 需要黑板键：TargetActor, TargetLocation, AcceptanceRadius
 */
UCLASS()
class SOULLIKEDEMO_API UBTTask_ChaseTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ChaseTarget();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    /** 追击目标 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector TargetActorKey;

    /** 停止距离（到达此距离认为追击完成） */
    UPROPERTY(EditAnywhere, Category = "Config")
        float AcceptanceRadius = 200.0f;

    /** 移动速度比例 */
    UPROPERTY(EditAnywhere, Category = "Config")
        float SpeedMultiplier = 1.0f;
};