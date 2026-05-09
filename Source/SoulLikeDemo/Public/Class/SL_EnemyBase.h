// Public/Class/SL_EnemyBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <SoulLikeGameGlobal.h>
#include <WeaponAccessory_IF.h>
#include <AbilitySystemInterface.h>
#include "SL_EnemyBase.generated.h"

/** 敌人状态 */
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Alive   UMETA(DisplayName = "存活"),
    Dead    UMETA(DisplayName = "死亡")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDied);

UCLASS()
class SOULLIKEDEMO_API ASL_EnemyBase : public ACharacter,
public IWeaponAccessory_IF,
public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASL_EnemyBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:
    /************************************************************************/
    /*                    IWeaponAccessory_IF 接口实现                        */
    /************************************************************************/
    virtual ASL_WeaponBase* GetLeftHandWeapon() const override;
    virtual ASL_WeaponBase* GetRightHandWeapon() const override;
    virtual ASL_WeaponBase* GetWeaponByHand(int32 HandIndex) const override;

    /************************************************************************/
    /*                    IAbilitySystemInterface 接口实现                        */
    /************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "Enemy|Ability")
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

public:

	// ===== 初始化 =====
	/** 初始化敌人数据 */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
		void InitializeEnemy(int32 EnemyID);

	UFUNCTION(BlueprintPure, Category = "Enemy")
		EEnemyState GetEnemyState() const { return CurrentState; }

	/*
	 * 公开给AIController访问
	 */
	// AIController获取行为树配置
    UFUNCTION(BlueprintPure, Category = "Enemy|AI")
        UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	// AIController获取黑板空间配置
    UFUNCTION(BlueprintPure, Category = "Enemy|AI")
        UBlackboardData* GetBlackboardData() const { return BlackboardData; }

    UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
		bool IsAlive();

	// AIController引用
    UFUNCTION(BlueprintPure, Category = "Enemy|AI")
        class ASL_EnemyAIController* GetEnemyAIController() const;

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

public:
	// ===== 委托 =====
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

	/** 根据武器ID派生武器实例 */
    ASL_WeaponBase* SpawnWeaponByID(int32 WeaponID);

    /** 根据配置生成左右手武器 */
    void SpawnEnemyWeapons(const FEnemyConfigInfo& Config);
	
protected:
	// ===== 配置数据 =====
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Config")
		FEnemyConfigInfo EnemyConfig;

	// 后续从EnemyConfig获取配置
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Config")
		float PerceptionRange = 1000.0f;

	// 后续从EnemyConfig获取配置
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

	// ===== AI组件->提供给AIController =====
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

	/** 武器派生模板类 */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Config")
		TSubclassOf<ASL_WeaponBase> WeaponBaseClass;

	    // ===== 新增：敌人持有的武器 =====
    /** 左手武器实例（在 ApplyEnemyConfig 中创建） */
    UPROPERTY()
        ASL_WeaponBase* LeftHandWeapon;

    /** 右手武器实例 */
    UPROPERTY()
        ASL_WeaponBase* RightHandWeapon;

};