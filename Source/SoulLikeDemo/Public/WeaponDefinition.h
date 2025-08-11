#pragma once
#include "SoulLikeGameGlobal.h"
#include "WeaponDefinition.generated.h"

USTRUCT(BlueprintType)
struct FWeaponDefinition
{
	GENERATED_BODY()

	// 唯一标识
	int32 WeaponID;
	// 异步加载模型
	TSoftObjectPtr<USkeletalMesh> Mesh;
	// 动画蓝图
	TSoftClassPtr<UAnimInstance> AnimClass;
	// 动态组件
	TArray<TSubclassOf<UActorComponent>> Components;
	// 各段攻击的伤害
	TMap<EAttackType, float> BaseDamageMap;
	// 各段攻击消耗的体力
	TMap<EAttackType, float> APCostMap;
	// 各段攻击消耗的蓝量
	TMap<EAttackType, float> MPCostMap;
	// 武器碰撞盒大小
	FVector WeaponCollisionBoxSize;

	/************************************************************************/
	/*                        近战模组(最多只能装在一个)                     */
	/************************************************************************/
	// 近战攻击模组
	bool bNeedLoadMeleeAttackComponent;
	// 投射物攻击模组
	bool bNeedLoadProjectileComponent;

	/************************************************************************/
	/*                        近战模组(最多只能装在一个)                    */
	/************************************************************************/
	// 防御模组
	bool bNeedLoadDefenceComponent;

	/************************************************************************/
	/*                        技能模组(最多只能装在一个)                    */
	/************************************************************************/
	// 技能模组
	bool bNeedLoadSkillComponent;

	// ...其他通用参数
};
