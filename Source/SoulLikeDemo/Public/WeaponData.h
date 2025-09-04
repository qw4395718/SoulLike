#pragma once
#include "SoulLikeGameGlobal.h"
#include "WeaponData.generated.h"

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()

	// 武器ID
	int32 WeaponID;
	// 唯一标识
	int32 WeaponUniqueID;
	// 模型名
	FString Mesh;
	// 动画蓝图名
	FString AnimClass;
	// 蒙太奇名
	FString MentageName;
	// 各段攻击的伤害
	TMap<EAttackType, float> BaseDamageMap;
	// 各段攻击消耗的体力
	TMap<EAttackType, float> APCostMap;
	// 各段攻击消耗的蓝量
	TMap<EAttackType, float> MPCostMap;
	// 武器碰撞盒大小
	FVector WeaponCollisionBoxSize;
	// 武器模组加载情况
	TMap<EWeaponComponentType, bool> NeedLoadComponentInfoMap;

	// 以下信息为表格信息
	///************************************************************************/
	///*                        近战模组(最多只能装在一个)                     */
	///************************************************************************/
	//// 近战攻击模组
	//bool bNeedLoadMeleeAttackComponent;
	//// 投射物攻击模组
	//bool bNeedLoadProjectileComponent;

	///************************************************************************/
	///*                        近战模组(最多只能装在一个)                    */
	///************************************************************************/
	//// 防御模组
	//bool bNeedLoadDefenceComponent;

	///************************************************************************/
	///*                        技能模组(最多只能装在一个)                    */
	///************************************************************************/
	//// 技能模组
	//bool bNeedLoadSkillComponent;

	///************************************************************************/
	///*                        特殊模组(可装载多个)                    */
	///************************************************************************/
	//// 处决模组
	//bool bNeedLoadExecuteComponent;

	//// 处决模组
	//bool bNeedLoadBackStabComponent;

	// ...其他通用参数
};

