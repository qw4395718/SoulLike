// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "MeleeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API AMeleeWeapon : public AWeaponBase
{
	GENERATED_BODY()

public:
	// 初始化武器（绑定到角色）
		virtual void Initialize() override;

	// 武器体力消耗
		virtual float GetStaminaCost(EAttackType AttackType) override;

	// 碰撞检测回调
		virtual void OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, FVector NormalImpulse,
			const FHitResult& Hit) override;

protected:
	// 武器碰撞盒数组
		//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		//	TArray<UBoxComponent*> HitBoxes;
	
};
