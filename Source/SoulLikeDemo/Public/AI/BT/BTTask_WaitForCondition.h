// Public/AI/BT/BTTask_WaitForCondition.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WaitForCondition.generated.h"

/**
 * 等待条件满足
 * 等待黑板中的某个条件变为true，或等待超时
 */
UCLASS(BlueprintType, Blueprintable)
class SOULLIKEDEMO_API UBTTask_WaitForCondition : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_WaitForCondition();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    /** 要检查的Bool黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector ConditionKey;

    /** 期望的条件值 */
    UPROPERTY(EditAnywhere, Category = "Config")
        bool bExpectedValue = true;

    /** 最大等待时间（-1表示无限等待） */
    UPROPERTY(EditAnywhere, Category = "Config")
        float MaxWaitTime = 5.0f;

private:
    float ElapsedTime;
};