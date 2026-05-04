// Public/Class/SL_EnemyBase.h

#pragma once

#include "CoreMinimal.h"
#include "SL_CharacterBase.h"
#include <GameFramework/Character.h>
#include <BrainComponent.h>
#include <Perception/AIPerceptionComponent.h>
#include <BehaviorTree/BehaviorTree.h>
#include "SL_EnemyBase.generated.h"

/** 敌人状态 */
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Patrol,
	Alert,
	Combat,
	Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyStateChanged, EEnemyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDied);

UCLASS()
class SOULLIKEDEMO_API ASL_EnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	ASL_EnemyBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;



public:

	// ===== 初始化 =====
	/** 初始化敌人数据 */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
		void InitializeEnemy(int32 EnemyID);

	// ===== 状态管理 =====
	UFUNCTION(BlueprintCallable, Category = "Enemy")
		void SetEnemyState(EEnemyState NewState);

	UFUNCTION(BlueprintPure, Category = "Enemy")
		EEnemyState GetEnemyState() const { return CurrentState; }

	// ===== AI接口 =====
	/** 切换到战斗状态 */
	UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
		void EnterCombat();

	/** 切换到巡逻状态 */
	UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
		void EnterPatrol();

	/** 失去目标 */
	UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
		void LoseTarget();

	// ===== 战斗接口 =====
	/** 受到攻击 */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
		void SufferDamage(float DamageAmount, AActor* DamageInstigator);

	/** 死亡处理 */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
		void Die();

public:
	// ===== 委托 =====
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
		FOnEnemyStateChanged OnEnemyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
		FOnEnemyDied OnEnemyDied;

protected:
	// ===== 配置数据 =====
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Config")
		TSubclassOf<UBehaviorTree> BehaviorTreeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Config")
		TSubclassOf<UAIPerceptionComponent> PerceptionComponentClass;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Config")
		float DetectionRange = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Config")
		float CombatRange = 200.0f;

	// ===== 状态 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
		EEnemyState CurrentState;

	UPROPERTY()
		AActor* CurrentTarget;

	// ===== AI组件 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
		UAIPerceptionComponent* AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
		UBrainComponent* BrainComponent;
};