// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "Damageable.h"
#include "CombatComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

	// 精准盾反判定（仅C++可用）
	bool CheckPerfectParry(float PlayerInputTime, float EnemyAttackTime);

	// 暴露给蓝图的伤害应用接口
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ProcessAttackHit(AActor* HitActor, const FHitResult& HitResult);

	// 抗性数据（C++计算）
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		TMap<EDamageType, float> DamageResistances;

protected:
	// 当前装备武器
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		UWeaponBase* EquippedWeapon;

	// 武器库存
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		TArray<UWeaponBase*> WeaponInventory;

	// 组件引用
	UPROPERTY()
		ASoulLikeCharacter* CharacterOwner;
		
};
