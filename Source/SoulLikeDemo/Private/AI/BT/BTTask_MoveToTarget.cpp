// Private/AI/BT/BTTask_MoveToTarget.cpp

#include "BTTask_MoveToTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "SL_EnemyBase.h"

UBTTask_MoveToTarget::UBTTask_MoveToTarget()
{
	NodeName = TEXT("Move To Target");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_MoveToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return EBTNodeResult::Failed;

	// 获取目标
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	FVector TargetLocation = Blackboard->GetValueAsVector(TargetLocationKey.SelectedKeyName);

	if (TargetActor)
	{
		// 移动向目标Actor
		AIController->MoveToActor(TargetActor, AcceptanceRadius, true, true, false);
		return EBTNodeResult::InProgress;
	}
	else if (!TargetLocation.IsZero())
	{
		// 移动向目标位置
		AIController->MoveToLocation(TargetLocation, AcceptanceRadius, true);
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_MoveToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 检查移动状态
	UPathFollowingComponent* PathFollowing = AIController->GetPathFollowingComponent();
	if (PathFollowing)
	{
		EPathFollowingStatus::Type Status = PathFollowing->GetStatus();

		if (Status == EPathFollowingStatus::Idle)
		{
			// 已经到达目标
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}

	// 检查目标是否死亡
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (TargetActor && TargetActor->IsPendingKillPending())
	{
		AIController->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

EBTNodeResult::Type UBTTask_MoveToTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
	}
	return EBTNodeResult::Aborted;
}