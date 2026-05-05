// Private/AI/BTTask_ChaseTarget.cpp

#include "BTTask_ChaseTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SL_EnemyBase.h"

UBTTask_ChaseTarget::UBTTask_ChaseTarget()
{
	NodeName = "Chase Target";
	bNotifyTick = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_ChaseTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	// 获取目标
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));

	if (!TargetActor) return EBTNodeResult::Failed;

	// 设置移动速度
	ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(AIController->GetPawn());
	if (Enemy && Enemy->GetCharacterMovement())
	{
		Enemy->GetCharacterMovement()->MaxWalkSpeed *= SpeedMultiplier;
	}

	// 开始移动追击
	AIController->MoveToActor(TargetActor, AcceptanceRadius, true, true, false);

	return EBTNodeResult::InProgress;
}

void UBTTask_ChaseTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 检查是否到达目标
	if (!AIController->IsFollowingAPath())
	{
		// 到达目标位置，任务完成
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 检查目标是否死亡
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));

	if (!TargetActor || TargetActor->IsPendingKillPending())
	{
		// 目标已死亡，取消追击
		AIController->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
}