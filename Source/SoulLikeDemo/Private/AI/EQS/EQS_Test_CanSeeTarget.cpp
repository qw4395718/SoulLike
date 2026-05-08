// Private/AI/EQS_Test_CanSeeTarget.cpp

#include "EQS_Test_CanSeeTarget.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "AIController.h"
#include "SL_EnemyBase.h"
#include "DrawDebugHelpers.h"

UEQS_Test_CanSeeTarget::UEQS_Test_CanSeeTarget()
{
	// 设置测试目的：筛选符合条件的目标
	TestPurpose = EEnvTestPurpose::FilterAndScore;

	// 设置为布尔类型测试（可见/不可见）
	SetWorkOnFloatValues(false);
}

void UEQS_Test_CanSeeTarget::RunTest(FEnvQueryInstance& QueryInstance) const
{
	// 获取查询所有者
	UObject* QueryOwner = QueryInstance.Owner.Get();
	AAIController* AIController = Cast<AAIController>(QueryOwner);
	if (!AIController) return;

	ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(AIController->GetPawn());
	if (!Enemy) return;

	// 获取当前查询的Actor
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		AActor* ItemActor = GetItemActor(QueryInstance, It.GetIndex());
		if (!ItemActor) continue;

		// 执行视线检测
		bool bCanSee = PerformLineOfSightCheck(Enemy, ItemActor);

		// 设置测试结果
		It.SetScore(TestPurpose, FilterType, bCanSee ? MaxThresholdValue : MinThresholdValue, MinThresholdValue, MaxThresholdValue);
	}
}

bool UEQS_Test_CanSeeTarget::PerformLineOfSightCheck(ASL_EnemyBase* Enemy, AActor* Target) const
{
	if (!Enemy || !Target) return false;

	// 设置检测参数
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Enemy);
	QueryParams.bTraceComplex = false;

	// 忽略指定的Actor类
	for (TSubclassOf<AActor> IgnoreClass : ActorsToIgnore)
	{
		if (Target->IsA(IgnoreClass))
		{
			QueryParams.AddIgnoredActor(Target);
		}
	}

	// 从敌人头部位置到目标位置进行射线检测
	FVector StartLocation = Enemy->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	FVector EndLocation = Target->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	bool bHit = Enemy->GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// 调试可视化
	if (Enemy->bEnableDebugDraw)
	{
		DrawDebugLine(
			Enemy->GetWorld(),
			StartLocation,
			EndLocation,
			bHit ? FColor::Red : FColor::Green,
			false, 0.1f, 0, 2.0f
		);
	}

	// 如果没有碰撞到任何东西，或者碰撞到的就是目标本身，则视为可见
	return (!bHit || HitResult.GetActor() == Target);
}