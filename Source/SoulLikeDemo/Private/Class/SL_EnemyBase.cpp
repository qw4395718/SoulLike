// Private/Class/SL_EnemyBase.cpp

#include "SL_EnemyBase.h"

ASL_EnemyBase::ASL_EnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CurrentState = EEnemyState::Idle;
	CurrentTarget = nullptr;

	// AI感知组件（预留）
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
}

void ASL_EnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// 初始状态
	SetEnemyState(EEnemyState::Idle);
}

void ASL_EnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASL_EnemyBase::InitializeEnemy(int32 EnemyID)
{
	// TODO: 从EnemyDataTable读取敌人配置
	// 例如：血量、攻击力、AI行为树等
	UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::InitializeEnemy - EnemyID=%d"), EnemyID);
}

// ==================== 状态管理 ====================

void ASL_EnemyBase::SetEnemyState(EEnemyState NewState)
{
	if (CurrentState == NewState) return;

	EEnemyState OldState = CurrentState;
	CurrentState = NewState;

	OnEnemyStateChanged.Broadcast(NewState);

	UE_LOG(LogTemp, Log, TEXT("Enemy State: %d -> %d"), (int32)OldState, (int32)NewState);
}

void ASL_EnemyBase::EnterCombat()
{
	SetEnemyState(EEnemyState::Combat);

	// TODO: 启动AI行为树
	/*if (BehaviorTreeClass && BrainComponent)
	{
		 BrainComponent->StartBehaviorTree(BehaviorTreeClass);
	}*/
}

void ASL_EnemyBase::EnterPatrol()
{
	SetEnemyState(EEnemyState::Patrol);
}

void ASL_EnemyBase::LoseTarget()
{
	CurrentTarget = nullptr;
	SetEnemyState(EEnemyState::Patrol);
}

void ASL_EnemyBase::SufferDamage(float DamageAmount, AActor* DamageInstigator)
{
	if (CurrentState == EEnemyState::Dead) return;

	// 受到伤害，进入战斗状态
	CurrentTarget = DamageInstigator;
	if (CurrentState != EEnemyState::Combat)
	{
		EnterCombat();
	}

	// TODO: 通过GAS应用伤害
	// 这里预留接口，后续由GAS系统处理
}

void ASL_EnemyBase::Die()
{
	SetEnemyState(EEnemyState::Dead);

	// 广播死亡事件
	OnEnemyDied.Broadcast();

	// 禁用AI
	if (BrainComponent)
	{
		BrainComponent->StopLogic(TEXT("Enemy Dead"));
	}

	// TODO: 播放死亡动画、掉落物品等
}