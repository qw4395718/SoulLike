// Private/AI/BTService/BTService_UpdateTarget.cpp
#include "BTService_UpdateTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "SL_EnemyBase.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
    NodeName = TEXT("Update Target (EQS)");
    Interval = 0.25f; // BTService的默认间隔
    bNotifyBecomeRelevant = false;
    bNotifyCeaseRelevant = false;
    bCreateNodeInstance = true;
}

void UBTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    CachedBTComp = &OwnerComp;

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return;

    ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(AIController->GetPawn());
    if (!Enemy || Enemy->GetEnemyState() == EEnemyState::Dead) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));

    // ===== 情况1：已有目标，检查是否丢失 =====
    if (CurrentTarget)
    {
        float Distance = FVector::Dist(Enemy->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (Distance <= Enemy->GetPerceptionRange())
        {
            // 目标在感知范围内，重置丢失计时
            TimeTargetLost = 0.0f;
            return; // 不需要搜索
        }

        // 目标超出范围，开始计时
        TimeTargetLost += Interval; // 使用Interval而不是DeltaSeconds，因为BTService不保证每帧调用
        if (TimeTargetLost >= LoseTargetTimeout)
        {
            UE_LOG(LogTemp, Verbose, TEXT("UBTService_UpdateTarget: Lost target %s"), *CurrentTarget->GetName());
            BB->ClearValue(TargetActorKey.SelectedKeyName);
            TimeTargetLost = 0.0f;
            // 清除目标后，继续执行下面的搜索逻辑
        }
        else
        {
            return; // 还在等待超时，不搜索
        }
    }

    // ===== 情况2：没有目标，通过EQS搜索 =====
    if (!EQSQueryTemplate || bIsQuerying) return;

    TimeSinceLastSearch += Interval;
    if (TimeSinceLastSearch < SearchInterval) return;
    TimeSinceLastSearch = 0.0f;

    // UE4.26: 运行EQS查询
    FEnvQueryRequest QueryRequest(EQSQueryTemplate, Enemy);
    bIsQuerying = true;

    QueryRequest.Execute(EEnvQueryRunMode::SingleResult,
        FQueryFinishedSignature::CreateUObject(this, &UBTService_UpdateTarget::OnEQSQueryFinished));

    UE_LOG(LogTemp, Verbose, TEXT("UBTService_UpdateTarget: Running EQS query %s"), *EQSQueryTemplate->GetName());
}

void UBTService_UpdateTarget::OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    bIsQuerying = false;

    if (!Result.IsValid() || !Result->IsFinished() || CachedBTComp == nullptr)
    {
        return;
    }

    // 获取行为树组件
    UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(CachedBTComp);
    if (!BTComp) return;

    // 获取EQS结果中的最佳Actor
    AActor* BestTarget = Result->GetItemAsActor(0);
    
    if (BestTarget && BTComp->GetBlackboardComponent())
    {
        BTComp->GetBlackboardComponent()->SetValueAsObject(TargetActorKey.SelectedKeyName, BestTarget);

        UE_LOG(LogTemp, Verbose, TEXT("UBTService_UpdateTarget: Found target %s via EQS"), *BestTarget->GetName());
    }
}