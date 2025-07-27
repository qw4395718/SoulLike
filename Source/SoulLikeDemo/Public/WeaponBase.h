// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulLikeGameGlobal.h"
#include "SoulLikeCharacter.h"
#include "Components/BoxComponent.h"
#include "WeaponBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class SOULLIKEDEMO_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:

	AWeaponBase();

protected:
	/************************************************************************/
	/*                               继承实现                                       */
	/************************************************************************/
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
	// 初始化武器（绑定到角色）
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual void Initialize();

	// 获取伤害数据（含类魂特性计算）
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
		FDamageData GetDamageData() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void SetEnableCapsuleCheck(bool enable);

	// 伤害碰撞检测
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void CheckHit();


	// 武器动作接口
	UFUNCTION(BlueprintNativeEvent, Category = "Animation")
		void PlayAttackMontage(EAttackType AttackType);

	// 碰撞检测回调
	UFUNCTION()
		virtual void OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, FVector NormalImpulse,
			const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual float GetStaminaCost(EAttackType AttackType);

public:
	//状态
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Condition")
		bool IsStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Condition")
		bool bEnableCapsuleCheck;
	// 组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* SkeletalWeaponMesh;

	// 组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* StaticWeaponMesh;

	// 胶囊体-高频检测
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UCapsuleComponent* CapsuleComp;

	// 碰撞盒-低频检测,有需要的时候动态生产
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UBoxComponent* CollisonBox;

	// 武器属性
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
		FWeaponStats WeaponData;

	// 动画资源
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		TMap<EAttackType, UAnimMontage*> AttackMontages;

	// 持有者引用
	UPROPERTY(Transient)
		ASoulLikeCharacter* OwningCharacter;
	
};
