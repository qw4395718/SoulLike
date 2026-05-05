// Private/AI/BTTask_FindNextPatrolPoint.cpp

#include "BTTask_FindNextPatrolPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include <BrainComponent.h>

UBTTask_FindNextPatrolPoint::UBTTask_FindNextPatrolPoint()
{
	NodeName = "Find Next Patrol Point";
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_FindNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	//if (!Blackboard) return EBTNodeResult::Failed;

	//// 获取巡逻点数组
	//TArray<FVector> PatrolPoints;
	//if (!Blackboard->GetValue<UBlackboardKeyType_Object>(PatrolPointsKey.GetSelectedKeyID()))
	//{
	//	// 如果黑板键是Object类型，尝试获取数组
	//	// 注意：UE4的Blackboard不直接支持数组，这里使用一种变通方法
	//	PatrolPoints = Blackboard->GetValue<UBlackboardKeyType_Vector>(PatrolPointsKey.GetSelectedKeyID());
	//}

	//if (PatrolPoints.Num() == 0)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("BTTask_FindNextPatrolPoint - No patrol points"));
	//	return EBTNodeResult::Failed;
	//}

	//// 获取当前索引
	//int32 CurrentIndex = Blackboard->GetValueAsInt(PatrolIndexKey.SelectedKeyName);

	//// 计算下一个索引
	//int32 NextIndex = CurrentIndex + 1;
	//if (NextIndex >= PatrolPoints.Num())
	//{
	//	if (bLoopPatrol)
	//	{
	//		NextIndex = 0;
	//	}
	//	else
	//	{
	//		return EBTNodeResult::Failed;
	//	}
	//}

	//// 设置新的巡逻点
	//Blackboard->SetValueAsInt(PatrolIndexKey.SelectedKeyName, NextIndex);
	//Blackboard->SetValueAsVector(TargetLocationKey.SelectedKeyName, PatrolPoints[NextIndex]);

	return EBTNodeResult::Succeeded;
}