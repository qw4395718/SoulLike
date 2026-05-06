// Public/AI/BTTask/BTTask_EnemyAttack.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "BTTask_EnemyAttack.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SOULLIKEDEMO_API UBTTask_EnemyAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_EnemyAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

protected:
    /** 攻击前使用的EQS（可选：用于确认攻击位置有效性） */
    UPROPERTY(EditAnywhere, Category = "EQS")
        class UEnvQuery* EQSValidateAttackQuery;

    /** 攻击的Montage Section名称 */
    UPROPERTY(EditAnywhere, Category = "Attack")
        FName AttackSectionName;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector TargetActorKey;

    /** EQS查询完成回调 */
    void OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result);

    /** 实际执行攻击 */
    void PerformAttack(AAIController* AIController, AActor* Target);

private:
    // 对于异步EQS查询，存储当前的行为树组件引用
    UBehaviorTreeComponent* CachedBTComp;
};