// Private/Class/SL_EnemyBase.cpp

#include "SL_EnemyBase.h"
#include <BrainComponent.h>
#include <Perception/AIPerceptionComponent.h>
#include <BehaviorTree/BehaviorTree.h>
#include <Manager/DataTableManager.h>
#include <EnemyConfigInfoTable.h>
#include <GameFramework/Character.h>
#include <GameplayAbilitySpec.h>
#include <Components/CapsuleComponent.h>
#include "Components/SkeletalMeshComponent.h"
#include <BehaviorTree/BlackboardData.h>
#include "SL_AbilitySystemComponent.h"
#include <SL_StatusAttributeSet.h>


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
	// 从DataTableManager获取敌人配置表
	UDataTableManager* TableManager = UDataTableManager::Get(this);
	if (!TableManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ASL_EnemyBase::InitializeEnemy - DataTableManager not found"));
		return;
	}

	UEnemyConfigInfoTable* EnemyTable = Cast<UEnemyConfigInfoTable>(TableManager->GetDataTable(EDataTableType::DT_EnemyConfigInfo));
	if (!EnemyTable)
	{
		UE_LOG(LogTemp, Error, TEXT("ASL_EnemyBase::InitializeEnemy - EnemyConfigInfoTable not found"));
		return;
	}

	FEnemyConfigInfo Config;
	if (!EnemyTable->GetEnemyConfig(EnemyID, Config))
	{
		UE_LOG(LogTemp, Error, TEXT("ASL_EnemyBase::InitializeEnemy - EnemyID=%d not found in config table"), EnemyID);
		return;
	}

	// 保存配置
	EnemyConfig = Config;

	// 应用配置
	ApplyEnemyConfig(Config);

	UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::InitializeEnemy - Initialized enemy: ID=%d, Name=%s, Type=%d"),
		EnemyID, *Config.EnemyName.ToString(), (int32)Config.EnemyType);
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

void ASL_EnemyBase::ApplyEnemyConfig(const FEnemyConfigInfo& Config)
{
	// 1. 设置属性（通过GAS）
	if (AbilitySystemComp)
	{
		// TODO: 通过GAS的AttributeSet设置初始属性
		// 例如：SetHealth(Config.BaseHealth)
		//       SetAttack(Config.BaseAttack)
		//       SetDefense(Config.BaseDefense)
		//       SetMoveSpeed(Config.BaseMoveSpeed)
	}

	// 2. 修改碰撞体大小
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCapsuleSize(Config.CapsuleRadius, Config.CapsuleHalfHeight);
	}

	// 3. 设置模型缩放
	if (GetMesh())
	{
		GetMesh()->SetRelativeScale3D(FVector(Config.MeshScale));
	}

	// 4. 加载外观
	LoadEnemyAppearance(Config);

	// 5. 初始化AI
	InitializeEnemyAI(Config);

	// 6. 授予GAS能力
	if (AbilitySystemComp && Config.GrantedAbilities.Num() > 0)
	{
		for (TSubclassOf<UGameplayAbility> AbilityClass : Config.GrantedAbilities)
		{
			if (AbilityClass)
			{
				AbilitySystemComp->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
			}
		}
	}
}

void ASL_EnemyBase::LoadEnemyAppearance(const FEnemyConfigInfo& Config)
{
	// 加载骨骼网格体
	if (!Config.SkeletalMesh.IsNull())
	{
		USkeletalMesh* EnemyMesh = Config.SkeletalMesh.LoadSynchronous();
		if (EnemyMesh && GetMesh())
		{
			GetMesh()->SetSkeletalMesh(EnemyMesh);
		}
	}

	// 加载动画蓝图
	if (!Config.AnimBlueprint.IsNull())
	{
		UAnimBlueprint* AnimBP = Config.AnimBlueprint.LoadSynchronous();
		if (AnimBP && GetMesh())
		{
			GetMesh()->SetAnimInstanceClass(AnimBP->GetAnimBlueprintGeneratedClass());
		}
	}
}

void ASL_EnemyBase::InitializeEnemyAI(const FEnemyConfigInfo& Config)
{
	// 设置AI感知范围
	PerceptionRange = Config.PerceptionRange;
	AttackRange = Config.AttackRange;

	// 加载行为树
	if (!Config.BehaviorTree.IsNull())
	{
		BehaviorTree = Config.BehaviorTree.LoadSynchronous();
	}

	// 加载黑板
	if (!Config.BlackboardData.IsNull())
	{
		BlackboardData = Config.BlackboardData.LoadSynchronous();
	}

	UE_LOG(LogTemp, Verbose, TEXT("ASL_EnemyBase::InitializeEnemyAI - PerceptionRange=%.2f, AttackRange=%.2f"),
		PerceptionRange, AttackRange);
}
