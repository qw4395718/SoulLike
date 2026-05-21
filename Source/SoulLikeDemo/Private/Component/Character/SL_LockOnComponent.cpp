// Private/Component/Character/SL_LockOnComponent.cpp

#include "SL_LockOnComponent.h"
#include "SL_EnemyBase.h"
#include <ActorState_IF.h>
#include <Manager/UIManagerSubsystem.h>
#include <Kismet/GameplayStatics.h>
#include <GameFramework/Controller.h>
#include <GameFramework/Pawn.h>

USL_LockOnComponent::USL_LockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bIsLocked = false;
	LockedTarget = nullptr;
}

// ==================== 初始化 ====================

void USL_LockOnComponent::InitializeLockOnComponent()
{
	UE_LOG(LogTemp, Verbose, TEXT("LockOnComponent initialized"));
}

// ==================== 索敌主入口 ====================

void USL_LockOnComponent::TryLockOnTarget()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
		return;

	// 已锁定 -> 释放
	if (bIsLocked)
	{
		ReleaseLockOn();
		return;
	}

	// 未锁定 -> 搜索最近敌人
	ASL_EnemyBase* Target = FindNearestEnemyInRange();
	if (!Target)
	{
		UE_LOG(LogTemp, Display, TEXT("LockOn: No valid target found in range"));
		return;
	}

	// 锁定目标
	LockedTarget = Target;
	bIsLocked = true;

	// 绑定目标死亡事件
	BindTargetDeathEvent(Target);

	// 启动定时校验
	StartValidationTimer();

	// 显示UI指示器
	UpdateLockOnIndicator(true);

	UE_LOG(LogTemp, Display, TEXT("LockOn: Locked onto target %s"), *Target->GetName());
}

// ==================== 释放锁定 ====================

void USL_LockOnComponent::ReleaseLockOn()
{
	if (!bIsLocked)
		return;

	// 停止定时校验
	StopValidationTimer();

	// 解绑死亡事件
	UnbindTargetDeathEvent();

	// 隐藏UI指示器
	UpdateLockOnIndicator(false);

	LockedTarget = nullptr;
	bIsLocked = false;

	UE_LOG(LogTemp, Display, TEXT("LockOn: Released"));
}

// ==================== 获取朝向 ====================

FVector USL_LockOnComponent::GetLockDirection() const
{
	if (!bIsLocked || !LockedTarget.IsValid())
		return FVector::ZeroVector;

	AActor* Owner = GetOwner();
	if (!Owner)
		return FVector::ZeroVector;

	return (LockedTarget->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
}

// ==================== 搜索最近敌人 ====================

ASL_EnemyBase* USL_LockOnComponent::FindNearestEnemyInRange() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	UWorld* World = Owner->GetWorld();
	if (!World) return nullptr;

	// 收集所有 ASL_EnemyBase 实例
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, ASL_EnemyBase::StaticClass(), FoundActors);

	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector OwnerForward = Owner->GetActorForwardVector();
	const float CosAngleThreshold = FMath::Cos(FMath::DegreesToRadians(LockOnAngle));

	ASL_EnemyBase* ClosestEnemy = nullptr;
	float ClosestDistSq = FLT_MAX;

	for (AActor* Actor : FoundActors)
	{
		ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(Actor);
		if (!Enemy) continue;

		// 过滤：是否存活
		IActorState_IF* StateInterface = Cast<IActorState_IF>(Enemy);
		if (!StateInterface || !StateInterface->IsAlive())
			continue;

		// 过滤：距离
		const FVector DirToTarget = Enemy->GetActorLocation() - OwnerLocation;
		const float DistSq = DirToTarget.SizeSquared();
		const float RangeSq = FMath::Square(LockOnRange);
		if (DistSq > RangeSq || DistSq < 1.0f)
			continue;

		// 过滤：角度（玩家正前方扇形）
		const FVector DirToTargetNormal = DirToTarget.GetSafeNormal2D();
		const float Dot = FVector::DotProduct(OwnerForward, DirToTargetNormal);
		if (Dot < CosAngleThreshold)
			continue;

		// 选取距离最近的
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestEnemy = Enemy;
		}
	}

	return ClosestEnemy;
}

// ==================== 校验锁定条件 ====================

bool USL_LockOnComponent::ValidateLockCondition() const
{
	if (!LockedTarget.IsValid())
		return false;

	// 校验目标是否存活
	ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(LockedTarget.Get());
	if (!Enemy) return false;

	IActorState_IF* StateInterface = Cast<IActorState_IF>(Enemy);
	if (!StateInterface || !StateInterface->IsAlive())
		return false;

	// 校验距离（使用逃逸系数）
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	const float EscapeRange = LockOnRange * EscapeRangeMultiplier;
	const float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Enemy->GetActorLocation());
	if (DistSq > FMath::Square(EscapeRange))
		return false;

	return true;
}

// ==================== UI指示器 ====================

void USL_LockOnComponent::UpdateLockOnIndicator(bool bShow)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(Owner);
	if (!UIManager) return;

	if (bShow && LockedTarget.IsValid())
	{
		// 在锁定目标上创建世界空间UI
		FUICreateParams Params = FUICreateParams::ForWorldActor(
			LockOnIndicatorWidgetType,
			LockedTarget.Get(),
			LockOnRange * EscapeRangeMultiplier * 1.5f
		);
		Params.WorldOffset = FVector(0.0f, 0.0f, 200.0f);
		UIManager->OpenWidget(Params);
	}
	else
	{
		// 移除锁定指示器
		UIManager->CloseWidget(LockOnIndicatorWidgetType);
	}
}

// ==================== 死亡事件绑定 ====================

void USL_LockOnComponent::BindTargetDeathEvent(ASL_EnemyBase* InTarget)
{
	if (!InTarget) return;
	UnbindTargetDeathEvent();

	InTarget->OnEnemyDied.AddDynamic(this, &USL_LockOnComponent::OnLockedTargetDied);
}

void USL_LockOnComponent::UnbindTargetDeathEvent()
{
	if (!LockedTarget.IsValid())
		return;

	if (ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(LockedTarget.Get()))
	{
		Enemy->OnEnemyDied.RemoveDynamic(this, &USL_LockOnComponent::OnLockedTargetDied);
	}
}

void USL_LockOnComponent::OnLockedTargetDied()
{
	UE_LOG(LogTemp, Display, TEXT("LockOn: Target died, releasing lock"));
	ReleaseLockOn();
}

// ==================== 定时器控制 ====================

void USL_LockOnComponent::StartValidationTimer()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	Owner->GetWorldTimerManager().SetTimer(
		ValidationTimerHandle,
		this,
		&USL_LockOnComponent::OnValidationTimerTick,
		ValidationInterval,
		true
	);
}

void USL_LockOnComponent::StopValidationTimer()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	Owner->GetWorldTimerManager().ClearTimer(ValidationTimerHandle);
}

void USL_LockOnComponent::OnValidationTimerTick()
{
	if (!bIsLocked)
	{
		StopValidationTimer();
		return;
	}

	if (!ValidateLockCondition())
	{
		ReleaseLockOn();
	}
}
