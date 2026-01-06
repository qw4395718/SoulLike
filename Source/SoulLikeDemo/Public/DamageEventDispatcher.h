// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulLikeGameGlobal.h"
#include "DamageEventDispatcher.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FWeaponStats {
	GENERATED_BODY()

		// 基础属性
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float BaseDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float PoiseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float StaminaCost = 15.0f;

	// 类魂特性
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float CriticalMultiplier = 2.0f;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	//	EWeaponScalingType ScalingType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float WeaponCollisonBoxLength = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float WeaponCollisonBoxWidth = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float WeaponCollisonBoxHeight = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<EAttackType, float> AttackTypeMultipliers;
};

USTRUCT(BlueprintType)
struct FDamageData {
	GENERATED_BODY()

	float TotalDamage;
	float PoiseDamage;
	float StaminaCost;
	bool bCanCritical;
};

USTRUCT(BlueprintType)
struct FDamageEventData 
{
	GENERATED_BODY()

	// 基础伤害信息
	UPROPERTY(BlueprintReadWrite)
	float BaseDamage;

	UPROPERTY(BlueprintReadWrite)
		FVector HitLocation;

	UPROPERTY(BlueprintReadWrite)
		FVector HitNormal;

	// 类魂特性数据
	UPROPERTY(BlueprintReadWrite)
		bool bIsCriticalHit;

	UPROPERTY(BlueprintReadWrite)
		EDamageType AttackType;

	UPROPERTY(BlueprintReadWrite)
		AActor* DamageCauser;

	// 构造器
	FDamageEventData(float Damage = 0.0f,
		const FVector& Location = FVector::ZeroVector,
		const FVector& Normal = FVector::UpVector,
		bool bCritical = false,
		EDamageType Type = EDamageType::SLASH,
		AActor* Causer = nullptr)
		: BaseDamage(Damage),
		HitLocation(Location),
		HitNormal(Normal),
		bIsCriticalHit(bCritical),
		AttackType(Type),
		DamageCauser(Causer) {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageSignature, const FDamageEventData&, DamageEvent);

UCLASS()
class UDamageEventDispatcher : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Combat")
		FOnDamageSignature OnDamageEvent;

	UFUNCTION(BlueprintCallable, Category = "Combat")
		void BroadcastDamageEvent(const FDamageEventData& DamageEvent) {
		OnDamageEvent.Broadcast(DamageEvent);
	}
};