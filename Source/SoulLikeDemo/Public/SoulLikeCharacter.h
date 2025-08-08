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

UCLASS()
class SOULLIKEDEMO_API ASoulLikeCharacter : public ACharacter 
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASoulLikeCharacter();

protected:
	/************************************************************************/
	/*                                内部调用                                      */
	/************************************************************************/
	// 左键响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformAttack();

	// 左Ctrl响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformCombatSkill();

	// 白盒初始化
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void Initialize();

public:
	/************************************************************************/
	/*                                外部调用                                      */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PlayBackStabbedMontage();

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PlayExecutionedMontage();

protected:
	/************************************************************************/
	/*                               继承实现                                       */
	/************************************************************************/
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
		UCombatComponent* CombatComponent;

	// 动画资源-待处决
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* ExecutionedMontage;

	// 动画资源-被被刺
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* BackStabbedMontage;
};
