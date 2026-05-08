// Private/AI/EQS_Generator_EnemyTargets.cpp

#include "EQS_Generator_EnemyTargets.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "AIController.h"
#include "SL_CharacterBase.h"
#include "SL_EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include <EnvironmentQuery/Contexts/EnvQueryContext_Querier.h>

UEQS_Generator_EnemyTargets::UEQS_Generator_EnemyTargets()
{
	ItemType = UEnvQueryItemType_Actor::StaticClass();
	SearchCenter = UEnvQueryContext_Querier::StaticClass();
}

void UEQS_Generator_EnemyTargets::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	// 获取搜索中心位置
	TArray<FVector> CenterLocation;
	CenterLocation.Add(FVector::ZeroVector);
	if (!QueryInstance.PrepareContext(SearchCenter, CenterLocation))
	{
		return;
	}

	// 获取搜索半径
	float Radius = SearchRadius.GetValue();
	if (Radius <= 0.0f)
	{
		return;
	}

	// 获取查询所有者
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner);
	if (!QueryOwner) return;

	// 获取敌人类
	ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(QueryOwner);
	if (!Enemy) return;

	// 获取AI控制器
	AAIController* AIController = Cast<AAIController>(Enemy->GetController());
	if (!AIController) return;

	// 查找所有潜在的敌人目标（玩家）
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(QueryOwner->GetWorld(), ASL_CharacterBase::StaticClass(), FoundActors);

	// 过滤并添加目标
	for (AActor* Actor : FoundActors)
	{
		// 排除自己
		if (Actor == Enemy) continue;

		// 检查目标是否存活
		ASL_CharacterBase* TargetChar = Cast<ASL_CharacterBase>(Actor);
		if (TargetChar /*&& TargetChar->IsAlive()*/)
		{
			// 检查距离是否在搜索半径内
			float Distance = FVector::Dist(CenterLocation[0], Actor->GetActorLocation());
			if (Distance <= Radius)
			{
				QueryInstance.AddItemData<UEnvQueryItemType_Actor>(Actor);
			}
		}
	}
}