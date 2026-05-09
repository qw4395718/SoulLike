// Public/AI/BT/BTService_UpdateTarget.h
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

    /************************************************************************/
    /*                               继承实现                               */
    /************************************************************************/
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    /************************************************************************/
    /*                               外部访问                               */
    /************************************************************************/
    /** 搜索目标用的EQS模板 */
    UPROPERTY(EditAnywhere, Category = "EQS")
        class UEnvQuery* EQSQueryTemplate;

    /** 输出目标到黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector TargetActorKey;

    /** 输出目标位置到黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector TargetLocationKey;

    /** 输出可疑位置到黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector SuspectedLocationKey;

    /** 输出是否有可疑位置到黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector HasSuspectedLocationKey;

    /** 输出到目标的距离到黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector DistanceToTargetKey;

    /** 输出是否有目标到黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector HasTargetKey;

    /** 输出是否在攻击范围内到黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector InAttackRangeKey;

    /** 输出是否在追击范围内到黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector InChaseRangeKey;

    /** 目标丢失超时时间（秒） */
    UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
        float LoseTargetTimeout = 5.0f;

    /** EQS查询频率 */
    UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.1"))
        float SearchInterval = 0.5f;

private:
    /************************************************************************/
    /*                               内部调用                               */
    /************************************************************************/
    // EQS查询完成的回调
    void OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result);

    // 更新黑板中距离和目标状态
    void UpdateTargetState(UBehaviorTreeComponent& OwnerComp, AActor* TargetActor);

private:
    /************************************************************************/
    /*                               内部访问                               */
    /************************************************************************/
    float TimeSinceLastSearch = 0.0f;
    float TimeTargetLost = 0.0f;

    // 标记当前是否有EQS查询正在进行（防止重复查询）
    bool bIsQuerying = false;

    // 对于异步EQS查询，存储当前的行为树组件引用
    UBehaviorTreeComponent* CachedBTComp;
};