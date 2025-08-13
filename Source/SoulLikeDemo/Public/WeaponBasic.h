// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulLikeGameGlobal.h"
#include "SoulLikeCharacter.h"
#include "Components/BoxComponent.h"
#include "SL_Attack_JumpSection_NS.h"
#include "WeaponBasic.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SOULLIKEDEMO_API AWeaponBasic : public AActor
{
	GENERATED_BODY()

public:

	AWeaponBasic();

	/************************************************************************/
	/*外部调用                                                                     */
	/************************************************************************/


protected:
	/************************************************************************/
	/*                               继承实现                                       */
	/************************************************************************/
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/************************************************************************/
	/*内部调用                                                                     */
	/************************************************************************/


protected:
	/************************************************************************/
	/*内部变量                                                                     */
	/************************************************************************/
	// 拥有者
	UPROPERTY()
	AActor* OwnerActor;

	// 武器ID
	UPROPERTY()
	uint32 WeaponID;

	// 武器骨骼网格体
	UPROPERTY()
	USkeletalMeshComponent* SkeletalWeaponMesh;

	// 碰撞盒子尺寸
	UPROPERTY()
		FVector CollisionBoxSize;

	// 武器动画蓝图

};
