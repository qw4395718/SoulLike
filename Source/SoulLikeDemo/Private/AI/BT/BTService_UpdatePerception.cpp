// Private/AI/BTService_UpdatePerception.cpp

#include "BTService_UpdatePerception.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "SL_EnemyBase.h"

UBTService_UpdatePerception::UBTService_UpdatePerception()
{
	NodeName = "Update Perception";
	bNotifyBecomeRelevant = false;
	bNotifyCeaseRelevant = false;
	Interval = 0.2f; // 每0.2秒更新一次
	RandomDeviation = 0.1f;
}

void UBTService_UpdatePerception::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(AIController->GetPawn());
	if (!Enemy) return;

	// 获取当前目标
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));

	// 如果没有目标，尝试寻找
	if (!TargetActor)
	{
		AActor* NewTarget = Enemy->FindNearestTarget();
		if (NewTarget)
		{
			Blackboard->SetValueAsObject(TargetActorKey.SelectedKeyName, NewTarget);
			TargetActor = NewTarget;
		}
	}

	// 更新感知状态
	if (TargetActor)
	{
		// 检查是否可以看到目标
		bool bCanSee = Enemy->CanSeeTarget(TargetActor);
		Blackboard->SetValueAsBool(bCanSeeTargetKey.SelectedKeyName, bCanSee);

		// 检查是否在攻击范围内
		float Distance = FVector::Dist(Enemy->GetActorLocation(), TargetActor->GetActorLocation());
		bool bInRange = Distance <= Enemy->GetAttackRange();
		Blackboard->SetValueAsBool(bInCombatRangeKey.SelectedKeyName, bInRange);

		// 如果目标超出感知范围，清除目标
		if (Distance > Enemy->GetPerceptionRange())
		{
			Blackboard->ClearValue(TargetActorKey.SelectedKeyName);
		}
	}
	else
	{
		Blackboard->SetValueAsBool(bCanSeeTargetKey.SelectedKeyName, false);
		Blackboard->SetValueAsBool(bInCombatRangeKey.SelectedKeyName, false);
	}
}