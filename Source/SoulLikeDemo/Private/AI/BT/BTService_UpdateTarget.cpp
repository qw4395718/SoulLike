// Private/AI/BT/BTService_UpdateTarget.cpp
#include "BTService_UpdateTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "SL_EnemyBase.h"
#include <ActorState_IF.h>

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
    NodeName = TEXT("Update Target");
    Interval = 0.25f;
    bNotifyBecomeRelevant = false;
    bNotifyCeaseRelevant = false;
    bCreateNodeInstance = true;
}

/************************************************************************/
/*                               继承实现                               */
/************************************************************************/

void UBTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return;

    // AI自身是否存活
    ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(AIController->GetPawn());
    if (!Enemy || !Enemy->IsAlive()) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
    
    /************************************************************************/
    /*              情况1：已有目标，更新目标状态或检查丢失                    */
    /************************************************************************/
    if (CurrentTarget)
    {
		/************************************************************************/
	    /*              情况2：有目标但已死亡,清理信息,通过EQS搜索新目标                              */
	    /************************************************************************/
        // 检测目标是否死亡
        if (IActorState_IF* ActorState_IF = Cast<IActorState_IF>(CurrentTarget))
        {
            if (ActorState_IF->IsDie() == true)
            {
                ClearCurrentTargetInfo();
                FindNewTarget(OwnerComp);
                return;
            }
        }
        else
        {// 为继承状态接口的目标为无效目标
            return;
        }

        float Distance = FVector::Dist(Enemy->GetActorLocation(), CurrentTarget->GetActorLocation());

        // 更新目标位置和距离
        BB->SetValueAsVector(TargetLocationKey.SelectedKeyName, CurrentTarget->GetActorLocation());
        BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);

        // 更新攻击范围状态
        bool bInAttackRange = Distance <= Enemy->GetAttackRange();
        bool bInChaseRange = Distance <= Enemy->GetPerceptionRange();
        BB->SetValueAsBool(InAttackRangeKey.SelectedKeyName, bInAttackRange);
        BB->SetValueAsBool(InChaseRangeKey.SelectedKeyName, bInChaseRange);

        // 检查目标是否丢失（超出追击距离）
        if (!bInChaseRange)
        {
            TimeTargetLost += Interval;
            if (TimeTargetLost >= LoseTargetTimeout)
            {
                // 目标丢失，但记录最后已知位置作为可疑位置
                BB->SetValueAsVector(SuspectedLocationKey.SelectedKeyName, CurrentTarget->GetActorLocation());
                BB->SetValueAsBool(HasSuspectedLocationKey.SelectedKeyName, true);

                // 清除当前目标
                BB->ClearValue(TargetActorKey.SelectedKeyName);
                BB->SetValueAsBool(HasTargetKey.SelectedKeyName, false);
                BB->SetValueAsBool(InAttackRangeKey.SelectedKeyName, false);

                TimeTargetLost = 0.0f;

                UE_LOG(LogTemp, Verbose, TEXT("BTService_UpdateTarget: Lost target %s, recording last position"),
                    *CurrentTarget->GetName());
            }
        }
        else
        {
            TimeTargetLost = 0.0f; // 重置丢失计时
        }

        return; // 已有目标，不需要搜索
    }

	/************************************************************************/
    /*              情况3：没有目标，通过EQS搜索                              */
    /************************************************************************/
    FindNewTarget(OwnerComp);

}

/************************************************************************/
/*                               内部调用                               */
/************************************************************************/

void UBTService_UpdateTarget::OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    bIsQuerying = false;

    if (!Result.IsValid() || !Result->IsFinished()) return;
    if (!CachedBTComp) return;

    UBlackboardComponent* BB = CachedBTComp->GetBlackboardComponent();
    if (!BB) return;

    // 获取EQS结果中的最佳Actor
    AActor* BestTarget = Result->GetItemAsActor(0);

    if (BestTarget)
    {
        BB->SetValueAsObject(TargetActorKey.SelectedKeyName, BestTarget);
        BB->SetValueAsVector(TargetLocationKey.SelectedKeyName, BestTarget->GetActorLocation());
        BB->SetValueAsBool(HasTargetKey.SelectedKeyName, true);

        UE_LOG(LogTemp, Verbose, TEXT("BTService_UpdateTarget: Found target %s via EQS"), *BestTarget->GetName());
    }
}

/************************************************************************/
/*                               内部调用                               */
/************************************************************************/

void UBTService_UpdateTarget::UpdateTargetState(UBehaviorTreeComponent& OwnerComp, AActor* TargetActor)
{
    // 此函数保留用于扩展，目前逻辑直接在 TickNode 中处理
}

void UBTService_UpdateTarget::ClearCurrentTargetInfo()
{
	if (!CachedBTComp) return;

	UBlackboardComponent* BB = CachedBTComp->GetBlackboardComponent();
	if (!BB) return;
	// 清除当前目标的所有信息
	BB->ClearValue(TargetActorKey.SelectedKeyName);
    BB->ClearValue(TargetLocationKey.SelectedKeyName);
    BB->ClearValue(SuspectedLocationKey.SelectedKeyName);
    BB->SetValueAsBool(HasSuspectedLocationKey.SelectedKeyName, false);
	BB->SetValueAsBool(HasTargetKey.SelectedKeyName, false);
	BB->SetValueAsBool(InAttackRangeKey.SelectedKeyName, false);
    BB->SetValueAsBool(InChaseRangeKey.SelectedKeyName, false);
}

void UBTService_UpdateTarget::FindNewTarget(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	// AI自身是否存活
	ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(AIController->GetPawn());
	if (!Enemy || !Enemy->IsAlive()) return;

	if (!EQSQueryTemplate || bIsQuerying) return;

	TimeSinceLastSearch += Interval;
	if (TimeSinceLastSearch < SearchInterval) return;
	TimeSinceLastSearch = 0.0f;

	// 运行EQS查询
	FEnvQueryRequest QueryRequest(EQSQueryTemplate, Enemy);
	bIsQuerying = true;
	CachedBTComp = &OwnerComp;

	QueryRequest.Execute(EEnvQueryRunMode::SingleResult,
		FQueryFinishedSignature::CreateUObject(this, &UBTService_UpdateTarget::OnEQSQueryFinished));

}
