#pragma once


// 角色配置-背刺距离限制
const INT BACKSTAB_DISTANCE_THRESHOLD = 150;

// 角色配置-背刺角度限制
const INT BACKSTAB_ANGLE_THRESHOLD = 30;

// 角色配置-处决距离限制
const INT EXECUTE_DISTANCE_THRESHOLD = 150;

// 特殊攻击(背刺,处决)检测半径
const INT DETECTION_RADIUS = 150;

// 装备栏单栏的槽位数量
const INT EQUIPMENT_SINGLE_CAPACITY = 4;

// 仓库分栏类型数量
const INT INVENTORYTYPE_NUM = 12;

// 仓库单栏类型容纳数量
const INT INVENTORY_SINGLE_CAPACITY = 100;

// 减伤最大上限
const float REDUCE_DAMAGE_PERCENTAGE = 0.9f

// 体力消耗最小下限
const float REDUCE_STAMINACOST_PERCENTAGE = 0.5f


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
	ARROWKEY_None,
	ARROWKEY_Up,
	ARROWKEY_Down,
	ARROWKEY_Left,
	ARROWKEY_Right,
	ARROWKEY_Max
};

// 武器模组类别
UENUM(BlueprintType)
enum class EWeaponModeTyoe :uint8
{
	WEAPONMODE_None,
	WEAPONMODE_Attack,
	WEAPONMODE_Defence,
	WEAPONMODE_ComboSkill,
	WEAPONMODE_BackStab,
	WEAPONMODE_Execute,
	WEAPONMODE_Max
};

// 武器技能模组类别
UENUM(BlueprintType)
enum class EWeaponCombaoSkillType :uint8
{
	COMBAOSKILL_None,
	COMBAOSKILL_Attack,
	COMBAOSKILL_Parry,
	COMBAOSKILL_AddBuff,
	COMBAOSKILL_Max
};

// 装备栏类别
UENUM(BlueprintType)
enum class EEquipmentSlotType :uint8
{
	EQUIPMENT_None,
	EQUIPMENT_Item_Up,
	EQUIPMENT_Item_Down,
	EQUIPMENT_Weapon_Left,
	EQUIPMENT_Weapon_Right,
	EQUIPMENT_Max
};

// 仓库分栏类别
UENUM(BlueprintType)
enum class EInventoryCompartmentType :uint8
{
	INVENTORYTYPE_None,
	INVENTORYTYPE_Weapon,
	INVENTORYTYPE_Helmet,
	INVENTORYTYPE_Armor,
	INVENTORYTYPE_Glove,
	INVENTORYTYPE_Pant,
	INVENTORYTYPE_Boot,
	INVENTORYTYPE_Ammunition,
	INVENTORYTYPE_Badge,
	INVENTORYTYPE_EquippableItem,
	INVENTORYTYPE_NoEquippableItem,
	INVENTORYTYPE_ComboSkillItem,
	INVENTORYTYPE_MagicBook,
	INVENTORYTYPE_Max,
};

// UI页面类别
UENUM()
enum class EUIWidgetType :uint32
{
	EWIDGET_None,
	EWIDGET_Bonfire,
	EWIDGET_Max
};

// 武器蒙太奇类别
UENUM()
enum class EWeaponMontageType :uint8
{
	EWeaponMontag_None,
	EWeaponMontag_Attack,
	EWeaponMontag_Defence,
	EWeaponMontag_ComboSkill,
	EWeaponMontag_Execute,
	EWeaponMontag_BackStab,
	EWeaponMontag_Max
};


