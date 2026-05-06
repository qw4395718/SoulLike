// Private/AI/BT/BTTask_WaitForCondition.cpp

#include "BTTask_WaitForCondition.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_WaitForCondition::UBTTask_WaitForCondition()
{
	NodeName = TEXT("Wait For Condition");
	bNotifyTick = true;
	bCreateNodeInstance = true;
	ElapsedTime = 0.0f;
}

EBTNodeResult::Type UBTTask_WaitForCondition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ElapsedTime = 0.0f;

	// 立即检查条件
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard)
	{
		bool bCurrentValue = Blackboard->GetValueAsBool(ConditionKey.SelectedKeyName);
		if (bCurrentValue == bExpectedValue)
		{
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_WaitForCondition::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ElapsedTime += DeltaSeconds;

	// 检查超时
	if (MaxWaitTime > 0.0f && ElapsedTime >= MaxWaitTime)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 检查条件是否满足
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard)
	{
		bool bCurrentValue = Blackboard->GetValueAsBool(ConditionKey.SelectedKeyName);
		if (bCurrentValue == bExpectedValue)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}