// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SL_CharacterBase.generated.h"


UCLASS()
class SOULLIKEDEMO_API ASL_CharacterBase : public ACharacter 
{
	GENERATED_BODY()



public:
	// Sets default values for this character's properties
	ASL_CharacterBase();

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

protected:
	/************************************************************************/
	/*                                内部调用                                      */
	/************************************************************************/
	// 左键响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformLeftMouse();

	// 右键响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformRightMouse();

	// 左Ctrl响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformCombatSkill();

public:
	/************************************************************************/
	/*                               外部调用                                       */
	/************************************************************************/
	// 白盒初始化
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void InitializeCharacter();

};
