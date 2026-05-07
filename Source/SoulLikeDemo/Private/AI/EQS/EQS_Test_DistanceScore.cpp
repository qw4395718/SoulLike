// Private/AI/EQS_Test_DistanceScore.cpp

#include "EQS_Test_DistanceScore.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "AIController.h"
#include "SL_EnemyBase.h"

UEQS_Test_DistanceScore::UEQS_Test_DistanceScore()
{
	// 设置测试目的：评分
	TestPurpose = EEnvTestPurpose::Score;

	// 设置浮点值输出
	SetWorkOnFloatValues(true);

	// 默认使用3D距离
	DistanceMode = EEnvTestDistance::Distance3D;

	// 默认为0，表示使用最短距离
	IdealDistance = 0.0f;
}

void UEQS_Test_DistanceScore::RunTest(FEnvQueryInstance& QueryInstance) const
{
	// 获取查询所有者位置
	UObject* QueryOwner = QueryInstance.Owner.Get();
	AAIController* AIController = Cast<AAIController>(QueryOwner);
	if (!AIController) return;

	AActor* QueryOwnerActor = AIController->GetPawn();
	if (!QueryOwnerActor) return;

	// 对每个项目计算距离评分
	int32 ItemIndex = 0;
	for(auto& Item : QueryInstance.GetAllAsActors())
	{
		AActor* ItemActor = Cast<AActor>(Item);
		if (!ItemActor) continue;

		// 计算距离
		float Distance = FVector::Dist(QueryOwnerActor->GetActorLocation(), ItemActor->GetActorLocation());

		// 计算评分
		float Score = CalculateScore(Distance);

		// 设置评分
		QueryInstance.SetItemScore(ItemIndex, Score);
		ItemIndex++;
	}
}

float UEQS_Test_DistanceScore::CalculateScore(float Distance) const
{
	// 如果在理想距离内，得满分
	if (Distance <= IdealDistance)
	{
		return 1.0f;
	}

	// 否则按距离衰减
	// 使用1/distance作为评分，但限制最小值
	float Score = FMath::Max(0.0f, 1.0f / (Distance - IdealDistance + 1.0f));

	// 归一化到0-1范围
	Score = FMath::Clamp(Score, 0.0f, 1.0f);

	return Score;
}