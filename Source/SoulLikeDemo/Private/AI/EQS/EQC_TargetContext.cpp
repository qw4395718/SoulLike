// Private/AI/EQC_TargetContext.cpp

#include "EQC_TargetContext.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "AIController.h"
#include "SL_EnemyBase.h"
#include <BehaviorTree/BlackboardComponent.h>
#include <BehaviorTree/BehaviorTreeComponent.h>

void UEQC_TargetContext::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// 获取查询的所有者（通常是AIController）
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner);
	if (!QueryOwner) return;

	// 从查询所有者获取AIController
	AAIController* AIController = Cast<AAIController>(QueryOwner);
	if (!AIController) return;

	// 获取黑板
	UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
	if (!BlackboardComp) return;

	// 获取当前目标
	AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (CurrentTarget)
	{
		// 将目标Actor添加到上下文数据中
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, CurrentTarget);
	}
}