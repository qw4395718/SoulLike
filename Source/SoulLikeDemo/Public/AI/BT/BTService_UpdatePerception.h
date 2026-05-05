// Public/AI/BTService_UpdatePerception.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdatePerception.generated.h"

/**
 * 更新感知信息（服务节点，每帧执行）
 * 设置黑板键：TargetActor, bCanSeeTarget, bInCombatRange
 */
UCLASS()
class SOULLIKEDEMO_API UBTService_UpdatePerception : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdatePerception();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    /** 目标Actor黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector TargetActorKey;

    /** 是否可以看到目标 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector bCanSeeTargetKey;

    /** 是否在攻击范围内 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector bInCombatRangeKey;
};