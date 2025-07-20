// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulLikeGameGlobal.h"
#include "SoulLikeCharacter.h"
#include "WeaponBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class SOULLIKEDEMO_API UWeaponBase : public UObject
{
	GENERATED_BODY()

public:
	// 初始化武器（绑定到角色）
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual void InitializeWeapon(ASoulLikeCharacter* OwnerCharacter);

	// 获取伤害数据（含类魂特性计算）
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
		FDamageData GetDamageData() const;

	// 武器动作接口
	UFUNCTION(BlueprintNativeEvent, Category = "Animation")
		void PlayAttackMontage(EAttackType AttackType);

	// 碰撞检测回调
	UFUNCTION()
		virtual void OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, FVector NormalImpulse,
			const FHitResult& Hit);

protected:
	//状态
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		bool IsStaticMesh;
	// 组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* SkeletalWeaponMesh;

	// 组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* StaticWeaponMesh;

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
