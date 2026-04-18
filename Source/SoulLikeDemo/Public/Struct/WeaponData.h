#pragma once
#include "SoulLikeGameGlobal.h"
#include "WeaponData.generated.h"

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()

		// 武器ID
		UPROPERTY()
		int32 WeaponID = 0;

	// 唯一标识
	UPROPERTY()
		int32 WeaponUniqueID;

	// 模型名
	UPROPERTY()
		FString Mesh;

	// 绑定的插槽名
	UPROPERTY()
		FString SocketName;

	// 动画蓝图名
	UPROPERTY()
		FString AnimClass;

	// 攻击动画蒙太奇名
	UPROPERTY()
		FString AttackMentageName;

	// 技能动画蒙太奇名
	UPROPERTY()
		FString ComboSkillMentageName;

	// 处决动画蒙太奇名
	UPROPERTY()
		FString ExecuteMentageName;

	// 背刺动画蒙太奇名
	UPROPERTY()
		FString BackStabMentageName;

	// 武器碰撞盒大小
	UPROPERTY()
		FVector WeaponCollisionBoxSize;

	// 武器参数信息
	TMap<EWeaponModeTyoe, FComboCoordinatorInfo> ComboCoordinatorInfoMap;
	// 各段攻击消耗的体力
	TMap<EAttackType, float> APCostMap;
	// 各段攻击消耗的蓝量
	TMap<EAttackType, float> MPCostMap;
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

	// 网络序列化函数
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		// 序列化基础类型
		Ar << WeaponID;
		Ar << WeaponUniqueID;
		Ar << Mesh;
		Ar << SocketName;
		Ar << AnimClass;
		Ar << AttackMentageName;
		Ar << ComboSkillMentageName;
		Ar << ExecuteMentageName;
		Ar << BackStabMentageName;
		Ar << WeaponCollisionBoxSize;

		// 序列化 TMap（简化版：只序列化数量，不序列化实际内容）
		// 因为 ComboCoordinatorInfoMap 等可能很复杂，如果不需要在客户端使用，可以不复制
		int32 MapSize = 0;
		if (Ar.IsSaving())
		{
			// 保存时：只保存大小，不保存具体内容
			MapSize = 0; // 客户端不需要这些数据
			Ar << MapSize;
		}
		else
		{
			// 加载时
			Ar << MapSize;
			// 客户端清空这些 Map
			ComboCoordinatorInfoMap.Empty();
			APCostMap.Empty();
			MPCostMap.Empty();
			NeedLoadComponentInfoMap.Empty();
		}

		bOutSuccess = true;
		return true;
	}
};

// 添加网络序列化支持
template<>
struct TStructOpsTypeTraits<FWeaponData> : public TStructOpsTypeTraitsBase2<FWeaponData>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

