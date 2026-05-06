// Private/AI/BTTask/BTTask_FindTarget.cpp
#include "BTTask_FindTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"

UBTTask_FindTarget::UBTTask_FindTarget()
{
    NodeName = TEXT("Find Target (EQS)");
    bNotifyTick = false;
    bCreateNodeInstance = true; // 每个Task实例独立，避免回调冲突
}

EBTNodeResult::Type UBTTask_FindTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    if (!EQSQueryTemplate)
    {
        UE_LOG(LogTemp, Error, TEXT("UBTTask_FindTarget: EQSQueryTemplate is not set!"));
        return EBTNodeResult::Failed;
    }

    // UE4.26: 运行EQS查询
    FEnvQueryRequest QueryRequest(EQSQueryTemplate, AIController->GetPawn());
    
    // 设置EQS查询完成的回调
    QueryRequest.Execute(EEnvQueryRunMode::SingleResult,
        FQueryFinishedSignature::CreateUObject(this, &UBTTask_FindTarget::OnEQSQueryFinished));

    // BTTask保持运行中状态，等待EQS回调
    return EBTNodeResult::InProgress;
}

void UBTTask_FindTarget::OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    if (!Result.IsValid() || !Result->IsFinished())
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_FindTarget: EQS query failed or aborted"));
        return;
    }

    // 获取行为树组件
    UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(GetBTComponent());
    if (!BTComp) return;

    // 获取EQS查询结果中的最佳Actor
    AActor* BestTarget = Result->GetItemAsActor(0); // 索引0是最高分的item
    
    if (BestTarget && BTComp->GetBlackboardComponent())
    {
        BTComp->GetBlackboardComponent()->SetValueAsObject(TargetActorKey.SelectedKeyName, BestTarget);
        
        UE_LOG(LogTemp, Verbose, TEXT("UBTTask_FindTarget: Found target %s"), *BestTarget->GetName());

        // 通知行为树任务完成（成功）
        FinishLatentTask(*BTComp, EBTNodeResult::Succeeded);
        return;
    }

    // 没有找到目标
    if (BTComp->GetBlackboardComponent())
    {
        BTComp->GetBlackboardComponent()->ClearValue(TargetActorKey.SelectedKeyName);
    }

    // 通知行为树任务完成（失败）
    FinishLatentTask(*BTComp, EBTNodeResult::Failed);
}

EBTNodeResult::Type UBTTask_FindTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 如果需要取消正在进行的EQS查询，可以在这里处理
    // UE4.26: EnvQueryManager没有直接的方法取消单个查询
    // 但BTTask被中断时，OnEQSQueryFinished会检查BTComp是否仍然有效

    return EBTNodeResult::Aborted;
}

FString UBTTask_FindTarget::GetStaticDescription() const
{
    if (EQSQueryTemplate)
    {
        return FString::Printf(TEXT("Find Target via EQS: %s"), *EQSQueryTemplate->GetName());
    }
    return TEXT("Find Target via EQS: No Template Set!");
}