// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SoulLikeGameGlobal.h"
#include "DamageEventDispatcher.h"
#include "Damageable.h"
#include "SoulLikeCharacter.generated.h"

class UCombatComponent;
class UDamageEventDispatcher;
class UMeleeWeapon;

UCLASS()
class SOULLIKEDEMO_API ASoulLikeCharacter : public ACharacter ,public IDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASoulLikeCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 实现IDamageable接口
	virtual void ReceiveDamage_Implementation(const FDamageEventData& DamageEvent) override;
	virtual bool CanReceiveDamage_Implementation() const override;

	// 类魂特性：韧性系统
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void ReducePoise(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Combat")
		void HandleDamage(const FDamageEventData& DamageEvent);
	//武器绑定到Socket
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void AttachWeaponToSocket(UWeaponBase* Weapon, FName SocketName);

	UFUNCTION(BlueprintCallable, Category = "Combat")
		FName GetWeaponHandSocket();

	UFUNCTION(BlueprintCallable, Category = "Combat")
		bool CanAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
		void ConsumeStamina(float costAp);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UDamageEventDispatcher* DamageDispatcher;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
		UMeleeWeapon* DefaultWeaponClass;

};
