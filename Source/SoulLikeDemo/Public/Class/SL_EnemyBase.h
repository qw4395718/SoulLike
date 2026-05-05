// Public/Class/SL_EnemyBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <SoulLikeGameGlobal.h>
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

	// ===== 状态机接口 =====
	/** 状态进入回调 */
	virtual void OnStateEnter(EEnemyState NewState);

	/** 状态退出回调 */
	virtual void OnStateExit(EEnemyState OldState);

	/** 状态更新（每帧调用） */
	virtual void OnStateTick(float DeltaTime);

	/** 获取当前状态持续时间 */
	UFUNCTION(BlueprintPure, Category = "Enemy|State")
		float GetStateTime() const { return StateElapsedTime; }

	/** 获取当前目标 */
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
		AActor* GetCurrentTarget() const { return CurrentTarget; }

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

	/** 死亡处理 */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
		void Die();

	// ===== 感知接口 =====
	UFUNCTION(BlueprintPure, Category = "Enemy|Perception")
		float GetPerceptionRange() const { return PerceptionRange; }

	UFUNCTION(BlueprintPure, Category = "Enemy|Perception")
		float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintCallable, Category = "Enemy|Perception")
		void SetPerceptionRange(float NewRange) { PerceptionRange = NewRange; }

	UFUNCTION(BlueprintCallable, Category = "Enemy|Perception")
		void SetAttackRange(float NewRange) { AttackRange = NewRange; }

	/** 检测是否可以看见目标 */
	bool CanSeeTarget(AActor* TargetActor) const;

	/** 检测目标是否在攻击范围内 */
	bool IsTargetInAttackRange() const;

	/** 获取最近的敌人（玩家） */
	AActor* FindNearestTarget() const;


public:
	// ===== 委托 =====
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
		FOnEnemyStateChanged OnEnemyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
		FOnEnemyDied OnEnemyDied;

	/** 绑定GAS死亡事件 */
	void BindGASDeathEvent();

	/** GAS角色死亡回调 */
	UFUNCTION()
		void OnGASCharacterDied(AActor* DiedActor, AActor* KillerActor);

	bool bEnableDebugDraw = false;

protected:
	/************************************************************************/
	/*                               内部调用                                       */
	/************************************************************************/
	// 应用敌人配置到自身
	void ApplyEnemyConfig(const FEnemyConfigInfo& Config);

	// 加载敌人外观 
	void LoadEnemyAppearance(const FEnemyConfigInfo& Config);

	// 初始化敌人AI 
	void InitializeEnemyAI(const FEnemyConfigInfo& Config);

	// ===== 感知系统 =====
	/** 更新感知（每帧调用） */
	void UpdatePerception(float DeltaTime);

protected:
	// ===== 配置数据 =====
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Config")
		FEnemyConfigInfo EnemyConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Config")
		TSubclassOf<class UAIPerceptionComponent> PerceptionComponentClass;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Config")
		float PerceptionRange = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Config")
		float AttackRange = 200.0f;

	// ===== 状态 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
		EEnemyState CurrentState;

	/** 状态持续时间 */
	float StateElapsedTime;

	/** 上次状态切换时间 */
	float LastStateChangeTime;

	UPROPERTY()
		AActor* CurrentTarget;

	// ===== AI组件 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
		class UBrainComponent* BrainComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
		class UAIPerceptionComponent* AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
		class UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
		class UBlackboardData* BlackboardData;

	// GAS组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
		class USL_AbilitySystemComponent* AbilitySystemComp;

	UPROPERTY()
		class USL_StatusAttributeSet* StatusAttributeSet;

	// 死亡委托的句柄
	FDelegateHandle OnCharacterDiedHandle;

};