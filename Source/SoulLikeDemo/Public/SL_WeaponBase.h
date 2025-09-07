// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulLikeGameGlobal.h"
#include "SoulLikeCharacter.h"
#include "Components/BoxComponent.h"
#include "WeaponBehavior_IF.h"
#include "WeaponData.h"
#include "SL_WeaponBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SOULLIKEDEMO_API ASL_WeaponBase : public AActor
{
	GENERATED_BODY()

public:

	ASL_WeaponBase();
public:
	/************************************************************************/
	/*                               接口实现                                       */
	/************************************************************************/
	// 攻击行为响应
	UFUNCTION()
		void Attack(AActor* OwnerActor);

	// 防御行为响应
	UFUNCTION()
		void Defence(AActor* OwnerActor);

	// 技能行为响应
	UFUNCTION()
		void ComboSkill(AActor* OwnerActor);

	// 处决行为响应
	UFUNCTION()
		void Execute(AActor* OwnerActor);

	// 背刺行为响应
	UFUNCTION()
		void BackStab(AActor* OwnerActor);

	/************************************************************************/
	/*外部调用                                                                     */
	/************************************************************************/
	// 初始化武器信息
	UFUNCTION()
		void InitWeaponInfo(const FWeaponData& WeaponInfo);

	// 激活该武器
	UFUNCTION()
		void ActiveWeapon();

	// 静默该武器
	UFUNCTION()
		void InActiveWeapon();
	
	// 更新武器状态
	UFUNCTION()
		void UpdateWeaponEquipState(EWeaponEquipState CurrentState);

	// 该武器是否配置了处决模组
	UFUNCTION()
		bool IsLoadExecuteMod();

	// 该武器是否配置了背刺模组
	UFUNCTION()
		bool IsLoadBackStabMod();
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
	void LoadWeaponMentageAsync(EWeaponMontageType MentageType,const FString MentagePath);

	// 异步加载武器动画蓝图
	void LoadWeaponAnimInstanceAsync(const FString WeapinAnimName);

	// 当武器网格体加载完成时
	void OnLoadedWeaponAnimInstance();

	// 加载武器模组
	bool LoadWeaponComponents(const TMap<EWeaponComponentType, bool>& WeaponComponentInfo);

	// 播放武器蒙太奇
	void PlayWeaponMentage(AActor* OwnerActor,EWeaponMontageType MentageType,FName MentageSectionName);
protected:
	/************************************************************************/
	/*内部变量                                                                     */
	/************************************************************************/

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
		TMap<EWeaponMontageType, UAnimMontage*> WeaponMentageMap;

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
