#pragma once
//伤害类型
UENUM(BlueprintType)
enum class EDamageType :uint8
{
	SLASH      UMETA(DisplayName = "Slash"),      // 斩击
	PIERCE     UMETA(DisplayName = "Pierce"),     // 穿刺
	BLUNT      UMETA(DisplayName = "Blunt"),      // 打击（补充类型）
	FIRE       UMETA(DisplayName = "Fire")        // 火焰
};

// 武器攻击动作类型
UENUM(BlueprintType)
enum class EAttackType :uint8
{
	Normal_Combo_Phase_1		UMETA(DisplayName = "Normal_Combo_Phase_1"),		// 普攻第一段
	Normal_Combo_Phase_2		UMETA(DisplayName = "Normal_Combo_Phase_2"),		// 普攻第二段
	Normal_Combo_Phase_3		UMETA(DisplayName = "Normal_Combo_Phase_3"),		// 普攻第三段
	Skill_Combo_Phase_1			UMETA(DisplayName = "Skill_Combo_Phase_1"),			// 战机第一段
	Skill_Combo_Phase_2,
	Skill_Combo_Phase_3,
	Special_Execution,
	Special_BackStab
};

UENUM(BlueprintType)
enum class EWeaponComponentType :uint8
{
	MeleeAttack,
	Parry,
	Execute,
	BackStab
};

UENUM(BlueprintType)
enum class EWeaponEquipState :uint8
{
	No_Equip,
	LH_Equip,
	RH_Equip
};

UENUM(BlueprintType)
enum class EArrowKeyType :uint8
{
	Up_ArrowKey,
	Down_ArrowKey,
	Left_ArrowKey,
	Right_ArrowKey
};


//USTRUCT(BlueprintType)
//struct FWeaponDefinition {
//
//	GENERATED_BODY()
//		// 唯一标识
//		int32 WeaponID;
//	// 异步加载模型
//	TSoftObjectPtr<USkeletalMesh> Mesh;
//	// 动画蓝图
//	TSoftClassPtr<UAnimInstance> AnimClass;
//	// 动态组件
//	TArray<TSubclassOf<UActorComponent>> Components;
//	// 各段攻击的伤害
//	TMap<EAttackType, float> BaseDamageMap;
//	// 各段攻击消耗的体力
//	TMap<EAttackType, float> APCostMap;
//	// 各段攻击消耗的蓝量
//	TMap<EAttackType, float> MPCostMap;
//	// 武器碰撞盒大小
//	FVector WeaponCollisionBoxSize;
//	/************************************************************************/
//	/*                               近战模组(最多只能装在一个)                                       */
//	/************************************************************************/
//	// 近战攻击模组
//	bool bNeedLoadMeleeAttackComponent;
//	// 投射物攻击模组
//	bool bNeedLoadProjectileComponent;
//
//	/************************************************************************/
//	/*                               近战模组(最多只能装在一个)                                       */
//	/************************************************************************/
//	// 防御模组
//	bool bNeedLoadDefenceComponent;
//
//	/************************************************************************/
//	/*                               技能模组(最多只能装在一个)                                       */
//	/************************************************************************/
//	// 技能模组
//	bool bNeedLoadSkillComponent;
//
//
//	// ...其他通用参数
//};


// 角色配置-背刺距离限制
const INT BackstabDistanceThreshold = 150;

// 角色配置-背刺角度限制
const INT BackstabAngleThreshold = 30;

// 角色配置-处决距离限制
const INT ExecuteDistanceThreshold = 150;

// 特殊攻击(背刺,处决)检测半径
const INT DetectionRadius = 150;