// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulLikeGameGlobal.h"
#include "WeaponDefinition.h"
#include "SoulLikeCharacter.h"
#include "Components/BoxComponent.h"
#include "WeaponDefinition.h"
#include "SL_WeaponComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SOULLIKEDEMO_API ASL_WeaponComponent : public AActor
{
	GENERATED_BODY()

public:

	ASL_WeaponComponent();

	/************************************************************************/
	/*外部调用                                                                     */
	/************************************************************************/
	// 初始化武器信息
	UFUNCTION()
		void InitWeaponInfo(const FWeaponDefinition& WeaponInfo);
	
	// 更新武器状态
	UFUNCTION()
		void UpdateWeaponEquipState(EWeaponEquipState CurrentState);

	// 鼠标左键响应
	UFUNCTION()
		void PerformAttack();

	// 鼠标右键响应
	UFUNCTION()
		void PerformDefence();

	// Ctrl键响应
	UFUNCTION()
		void PerformComboSkill();


protected:
	/************************************************************************/
	/*                               继承实现                                       */
	/************************************************************************/

	/************************************************************************/
	/*内部调用                                                                     */
	/************************************************************************/
	// 异步加载武器网格体
	void LoadWeaponMeshAsync(const FString WeaponMeshName);

	// 当武器网格体加载完成时
	void OnLoadedWeaponMesh();

	// 异步加载武器蒙太奇动画(供给角色类使用)
	void LoadWeaponMentageAsync(const FString MentagePath);

	// 异步加载武器动画蓝图
	void LoadWeaponAnimInstanceAsync(const FString WeapinAnimName);

	// 当武器网格体加载完成时
	void OnLoadedWeaponAnimInstance();

	// 加载武器模组
	bool LoadWeaponComponents(const TMap<EWeaponComponentType, bool>& pWeaponComponentInfo);

	// 处决检测
	bool CanExecute(AActor* MasterActor,float AllowedExecuteDistance, float AllowdBackStabRange);

	// 背刺检测
	bool CanBackStab(AActor* MasterActor,float AllowedBackStabDistance,float AllowdBackStabRange);


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
		TSoftObjectPtr<USkeletalMesh> SoftMeshReference;

	// 武器骨骼网格体组件
	UPROPERTY()
		USkeletalMeshComponent* SkeletalWeaponMesh;

	// 碰撞盒子尺寸
	UPROPERTY()
		FVector CollisionBoxSize;

	// 武器蒙太奇动画异步加载ptr
	UPROPERTY()
		TSoftClassPtr<UAnimMontage> SoftMentageRefrence;

	// 武器动画蓝图
	UPROPERTY()
		TSoftClassPtr<UClass> SoftWeaponAnimInstanceReference;

	// 武器模组(请注意武器模组与武器动画蓝图是直接关联的)
	UPROPERTY()
		TMap<EWeaponComponentType, USceneComponent*> WeaponComponentMap;

	// 武器模组实际加载情况
	UPROPERTY()
		TMap<EWeaponComponentType, bool> WeaponLoadComponentInfoMap;

	// 武器装备槽位情况
	UPROPERTY()
		EWeaponEquipState WeaponEquipInfo;
};
