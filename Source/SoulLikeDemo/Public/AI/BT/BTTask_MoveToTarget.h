// Public/AI/BT/BTTask_MoveToTarget.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToTarget.generated.h"

/**
 * 移动到目标位置
 * 使用导航系统移动到目标Actor或位置
 */
UCLASS(BlueprintType, Blueprintable)
class SOULLIKEDEMO_API UBTTask_MoveToTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_MoveToTarget();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    /** 目标Actor黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector TargetActorKey;

    /** 目标位置黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector TargetLocationKey;

    /** 停止距离 */
    UPROPERTY(EditAnywhere, Category = "Config")
        float AcceptanceRadius = 200.0f;

    /** 是否使用路径追踪 */
    UPROPERTY(EditAnywhere, Category = "Config")
        bool bUsePathFollowing = true;
};