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

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PlayBackStabbedMontage();

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PlayExecutionedMontage(FName MontageSectionName);

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void ReadTableTest(const FString TablePath);

public:
	/************************************************************************/
	/*                                外部调用                                      */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformExecuted(FName MontageSectionName);

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformBackStabbed();
	
	UFUNCTION(BlueprintCallable, Category = "GetState")
		bool IsAlive() const { return bIsAlive; }

	UFUNCTION(BlueprintCallable, Category = "GetState")
		bool IsReadyForExecution() const { return bIsReadyForExecution; }

	UFUNCTION(BlueprintCallable, Category = "SetState")
		void SetWaitExecutionState(bool bIsWaitExecution);

	UFUNCTION(BlueprintCallable, Category = "SetState")
		void SetExecutingState(bool bIsExecuting);

	UFUNCTION(BlueprintCallable, Category = "SetState")
		void SetBackStabbingState(bool bIsBackStabbing);

	UFUNCTION(BlueprintCallable, Category = "SetState")
		void MoveToLocationAndRotation(FVector LocationPosition, FRotator Rotaion);
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

protected:

	// 动画资源-待处决
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* ExecutionedMontage;

	// 动画资源-被被刺
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* BackStabbedMontage;

	// 角色状态-是否处于待处决状态
	UPROPERTY(EditDefaultsOnly, Category = "CharacterState")
		bool bIsReadyForExecution = false;

	// 角色状态-是否处于正在处决状态(无敌,不被打断)
	UPROPERTY(EditDefaultsOnly, Category = "CharacterState")
		bool bIsExecuting = false;

	// 角色状态-是否处于正在处决状态(无敌,不被打断)
	UPROPERTY(EditDefaultsOnly, Category = "CharacterState")
		bool bIsBackStabbing = false;

	// 角色状态-是否存活
	UPROPERTY(EditDefaultsOnly, Category = "CharacterState")
		bool bIsAlive = false;
	
};
