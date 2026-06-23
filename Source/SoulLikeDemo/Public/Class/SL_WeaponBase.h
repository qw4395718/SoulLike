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
class UGameplayEffect;
class USL_WeaponAnimSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponHitDelegate, AActor*, TargetActor, const FHitResult&, HitResult, const FWeaponDataInfo&, WeaponData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponParryDelegate, AActor*, TargetActor);

UCLASS()
class SOULLIKEDEMO_API ASL_WeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ASL_WeaponBase();

	virtual void SetOwner(AActor* NewOwner) override;

	// ===== 初始化接口 =====
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void InitializeWeaponWithID(int32 InWeaponID, FName InSocketName);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void InitializeFromDataRow(const FWeaponDataInfo& InWeaponData);

	// ===== 碰撞控制 =====
	UFUNCTION(BlueprintCallable, Category = "Weapon|Collision")
		void EnableAttackCollision();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Collision")
		void DisableAttackCollision();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Parry")
		void EnableParryWindow(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Parry")
		void DisableParryWindow();

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

	// 获取武器动画数据集
	UFUNCTION(BlueprintPure, Category = "Weapon|Animation")
		class USL_WeaponAnimSet* GetWeaponAnimSet() const;

public:
	// ===== 委托 =====
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
		FOnWeaponHitDelegate OnWeaponHitDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
		FOnWeaponParryDelegate OnWeaponParryDelegate;

protected:
	virtual void BeginPlay() override;

	// ===== 碰撞回调 =====
	UFUNCTION()
		void OnCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnCollisionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ApplyDamageToOverlappingActors();

	float CalculateFinalDamage(AActor* InTargetActor) const;

	void ApplyParryToOverlappingActors();

	// ===== 打击感：命中反馈 =====
	/** 触发命中反馈（Server 端：Hit Stop + Multicast 广播） */
	void TriggerHitFeedback(AActor* InTarget, const FVector& InHitLocation, EHitSeverity InSeverity);

	/** Multicast RPC：所有客户端同步执行命中反馈 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnHitFeedback(const FHitFeedbackData& InData);

	// ===== 辅助方法 =====
	void LoadWeaponAssets();

	void SetupAnimClass();

	void SetupRotator();

	void SetupOffset();

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Data")
		FWeaponDataInfo WeaponData;

	UPROPERTY(EditDefaultsOnly, Category = "Damage|Config")
		TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY()
		class ACharacter* OwningCharacter;

	// ===== 状态 =====
	bool bIsParryWindowActive;

	bool bIsStaticMesh;

	// ===== 碰撞系统 =====
	FTimerHandle DamageTimerHandle;

	FTimerHandle ParryTimerHandle;

	UPROPERTY()
		TArray<AActor*> AttackOverlappingActors;

	UPROPERTY()
		TArray<AActor*> ParryOverlappingActors;

	UPROPERTY()
		TSet<AActor*> AlreadyHitActors;

	UPROPERTY()
		TSet<AActor*> AlreadyParryActors;

	// 首次命中时记录的骨骼名
	UPROPERTY()
	TMap<AActor*, FName> HitBoneMap;

	// ===== 配置 =====
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Config")
		float DamageInterval = 0.03f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Config")
		float ParryInterval = 0.05f;

	UPROPERTY()
		FName SocketName;


};
