// Public/Class/WeaponBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "SoulLikeGameGlobal.h"
#include "SL_WeaponBase.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class UCombatComponent;
class USL_ComboManagerComponent;
class USL_StaminaComponent;
class ASL_CharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponHitDelegate, AActor*, TargetActor, const FHitResult&, HitResult, const FWeaponDataInfo&, WeaponData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponParryDelegate, AActor*, TargetActor);

UCLASS()
class SOULLIKEDEMO_API ASL_WeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ASL_WeaponBase();

	// ===== 初始化接口 =====
	/** 根据武器ID初始化武器 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void InitializeWeaponWithID(int32 WeaponID);

	/** 直接从数据行初始化（用于测试/动态武器） */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void InitializeFromDataRow(const FWeaponDataInfo& InWeaponData);

	// ===== 碰撞控制 =====
	/** 开启攻击碰撞检测 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Collision")
		void EnableAttackCollision();

	/** 关闭攻击碰撞检测 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Collision")
		void DisableAttackCollision();

	/** 开启弹反窗口 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Parry")
		void EnableParryWindow(float Duration);

	/** 关闭弹反窗口 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Parry")
		void DisableParryWindow();

	/** 是否在弹反窗口内 */
	UFUNCTION(BlueprintPure, Category = "Weapon|Parry")
		bool IsParryWindowActive() const { return bIsParryWindowActive; }

	// ===== 数据查询 =====
	UFUNCTION(BlueprintPure, Category = "Weapon|Data")
		int32 GetWeaponID() const { return WeaponData.WeaponID; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Data")
		const FWeaponDataInfo& GetWeaponData() const { return WeaponData; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Data")
		float GetBaseDamage() const { return WeaponData.BaseDamage; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Stamina")
		float GetStaminaCostMultiplier() const { return WeaponData.StaminaCostMultiplier; }

public:
	// ===== 委托 =====
	/** 武器命中目标时广播 */
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
		FOnWeaponHitDelegate OnWeaponHitDelegate;

	/** 武器弹反成功时广播 */
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
		FOnWeaponParryDelegate OnWeaponParryDelegate;

protected:
	virtual void BeginPlay() override;

	virtual void SetOwner(AActor* NewOwner) override;

	// ===== 碰撞回调 =====
	UFUNCTION()
		void OnCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnCollisionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** 定时器回调：对重叠的Actor应用伤害 */
	void ApplyDamageToOverlappingActors();

	/** 定时器回调：对重叠的Actor应用弹反 */
	void ApplyParryToOverlappingActors();

	// ===== 辅助方法 =====
	/** 初始化资源（网格体、音效、特效） */
	void LoadWeaponAssets();

	// 设置动画蓝图
	void SetupAnimClass();

	// 设置旋转
	void SetupRotator();

	// 设置偏移
	void SetupOffset();

	/** 设置碰撞盒大小 */
	void SetupCollisionBox();

protected:
	// ===== 组件 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* SkeletalWeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* StaticWeaponMesh;

	// ===== 数据 =====
	/** 当前武器配置数据 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Data")
		FWeaponDataInfo WeaponData;

	/** 持有者角色 */
	UPROPERTY()
		class ASL_CharacterBase* OwningCharacter;

	// ===== 状态 =====
	/** 弹反窗口是否激活 */
	bool bIsParryWindowActive;

	/** 是否使用静态网格体 */
	bool bIsStaticMesh;

	// ===== 碰撞系统 =====
	/** 攻击碰撞定时器 */
	FTimerHandle DamageTimerHandle;

	/** 弹反碰撞定时器 */
	FTimerHandle ParryTimerHandle;

	/** 攻击重叠Actor列表 */
	UPROPERTY()
		TArray<AActor*> AttackOverlappingActors;

	/** 弹反重叠Actor列表 */
	UPROPERTY()
		TArray<AActor*> ParryOverlappingActors;

	/** 已命中的Actor列表（防止重复伤害） */
	UPROPERTY()
		TSet<AActor*> AlreadyHitActors;

	/** 已弹反的Actor列表 */
	UPROPERTY()
		TSet<AActor*> AlreadyParryActors;

	// ===== 配置 =====
	/** 伤害检测间隔 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Config")
		float DamageInterval = 0.1f;

	/** 弹反检测间隔 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Config")
		float ParryInterval = 0.05f;
};