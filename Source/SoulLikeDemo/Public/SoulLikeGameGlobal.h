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

UENUM(BlueprintType)
enum class EAttackType :uint8
{
	Normal_Combo_Phase_1		UMETA(DisplayName = "Normal_Combo_Phase_1"),		// 普攻第一段
	Normal_Combo_Phase_2		UMETA(DisplayName = "Normal_Combo_Phase_2"),		// 普攻第二段
	Normal_Combo_Phase_3		UMETA(DisplayName = "Normal_Combo_Phase_3"),		// 普攻第三段
	Skill_Combo_Phase_1			UMETA(DisplayName = "Skill_Combo_Phase_1")			// 战机第一段
};


// 角色配置-背刺距离限制
const INT BackstabDistanceThreshold = 150;

// 角色配置-背刺角度限制
const INT BackstabAngleThreshold = 30;

// 角色配置-处决距离限制
const INT ExecuteDistanceThreshold = 150;

// 特殊攻击(背刺,处决)检测半径
const INT DetectionRadius = 150;