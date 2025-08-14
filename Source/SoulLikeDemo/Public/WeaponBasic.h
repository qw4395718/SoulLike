// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulLikeGameGlobal.h"
#include "SoulLikeCharacter.h"
#include "Components/BoxComponent.h"
#include "WeaponDefinition.h"
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
	// 初始化武器信息
	UPROPERTY()
		InitWeaponInfo(FWeaponDefinition* pWeaponInfo);
	
	// 更新武器状态
	UPROPERTY()
		UpdateWeaponEquipState(EWeaponEquipState CurrentState);


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

	// 异步加载骨骼网格体模型
		TSoftObjectPtr<USkeletalMesh> Mesh;

	// 武器骨骼网格体组件
	UPROPERTY()
		USkeletalMeshComponent* SkeletalWeaponMesh;

	// 碰撞盒子尺寸
	UPROPERTY()
		FVector CollisionBoxSize;

	// 武器动画蓝图
	UPROPERTY()
		TSoftClassPtr<UAnimInstance> WeaponAnimInstance;

	// 武器模组(请注意武器模组与武器动画蓝图是直接关联的)
	UPROPERTY()
		TMap<EWeaponComponentType, USceneComponent*> WeaponComponentMap;

	// 武器模组加载情况
	UPROPERTY()
		TMap<EWeaponComponentType, bool> WeaponLoadComponentInfoMap;

	// 武器装备槽位情况
	UPROPERTY()
		EWeaponEquipState WeaponEquipInfo;
};
