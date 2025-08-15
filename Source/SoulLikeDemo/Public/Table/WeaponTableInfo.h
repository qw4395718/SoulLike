#pragma once
#include "SoulLikeGameGlobal.h"
#include <Engine/DataTable.h>
#include "WeaponTableInfo.generated.h"

USTRUCT(BlueprintType, meta = (DisplayName = "WeaponConfigData"))
struct FWeaponTableInfo : public FTableRowBase
{
	GENERATED_BODY()

	// 唯一标识
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 WeaponID;
	// 模型名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString Mesh;
	// 动画蓝图名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString AnimClass;
	// 蒙太奇名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString Mentage;
	// 各段攻击的伤害
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TMap<EAttackType, float> BaseDamageMap;
	// 各段攻击消耗的体力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TMap<EAttackType, float> APCostMap;
	// 各段攻击消耗的蓝量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TMap<EAttackType, float> MPCostMap;
	// 武器碰撞盒大小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FVector WeaponCollisionBoxSize;
	// 武器模组加载情况
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
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
