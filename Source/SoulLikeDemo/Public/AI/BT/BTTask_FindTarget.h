// Public/AI/BTTask/BTTask_FindTarget.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "BTTask_FindTarget.generated.h"

UCLASS()
class SOULLIKEDEMO_API UBTTask_FindTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_FindTarget();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

protected:
    /** 使用的EQS查询模板 */
    UPROPERTY(EditAnywhere, Category = "EQS")
        class UEnvQuery* EQSQueryTemplate;

    /** 输出目标到黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector TargetActorKey;

    /** EQS查询完成的回调 */
    void OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result);
};