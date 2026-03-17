#pragma once
#include "SL_Macros.h"

/************************************************************************/
/*                              Const                                         */
/************************************************************************/
#define UNLUA_INCLUDED

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
const float REDUCE_DAMAGE_PERCENTAGE = 0.9f;

// 体力消耗最小下限
const float REDUCE_STAMINACOST_PERCENTAGE = 0.5f;

// 对话选项栏最大缓存交互控件个数
const INT INTERACT_BTN_MAX = 5;

// 对话选项栏控件高度
const INT INTERACT_BTN_HEIGHT = 40;

/************************************************************************/
/*                              Enum                                         */
/************************************************************************/

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
	WEAPONMODE_Min,
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



// 武器行为消耗资源类别
UENUM()
enum class EWeaponActionCostType :uint8
{
	EWeaponAction_Min,
	EWeaponAction_Health,
	EWeaponAction_Stamina,
	EWeaponAction_Magic,
	EWeaponAction_Max
};



// 动画通知种类
UENUM()
enum class EAnimNotifyType :uint32
{
	EAnimNotify_Min,

	// 连招系统相关
	EAnimNotify_LH_Active_NormalComboWindow,
	EAnimNotify_LH_InActive_NormalComboWindow,
	EAnimNotify_LH_Active_SkillComboWindow,
	EAnimNotify_LH_InActive_SkillComboWindow,
	EAnimNotify_RH_Active_NormalComboWindow,
	EAnimNotify_RH_InActive_NormalComboWindow,
	EAnimNotify_RH_Active_SkillComboWindow,
	EAnimNotify_RH_InActive_SkillComboWindow,

	EAnimNotify_LH_EnableCollision_Melee,
	EAnimNotify_LH_DisableCollision_Melee,
	EAnimNotify_RH_EnableCollision_Melee,
	EAnimNotify_RH_DisableCollision_Melee,

	EAnimNotify_LH_AcitiveParryWindow_Melee,
	EAnimNotify_LH_InAcitiveParryWindow_Melee,
	EAnimNotify_RH_AcitiveParryWindow_Melee,
	EAnimNotify_RH_InAcitiveParryWindow_Melee,

	EAnimNotify_LH_EnableCollision_ComboSkil_Parry,
	EAnimNotify_LH_DisableCollision_ComboSkil_Parry,
	EAnimNotify_RH_EnableCollision_ComboSkil_Parry,
	EAnimNotify_RH_DisableCollision_ComboSkil_Parry,

	EAnimNotify_Max
};

// 动画通知种类
UENUM()
enum class EWeaponAnimNotifyType :uint32
{
	EWeaponAnimNotify_Min,

	EWeaponAnimNotify_Active_NormalComboWindow,
	EWeaponAnimNotify_InActive_NormalComboWindow,
	EWeaponAnimNotify_Active_SkillComboWindow,
	EWeaponAnimNotify_InActive_SkillComboWindow,

	EWeaponAnimNotify_EnableCollision_Melee,
	EWeaponAnimNotify_DisableCollision_Melee,

	EWeaponAnimNotify_ActiveParryWindow_Melee,
	EWeaponAnimNotify_InActiveParryWindow_Melee,

	EWeaponAnimNotify_EnableCollision_ComboSkil_Parry,
	EWeaponAnimNotify_DisableCollision_ComboSkil_Parry,

	EWeaponAnimNotify_Max
};

UENUM(BlueprintType)
enum class EHUDEquipmentSlotType :uint8
{
	EHUDEquipmentSlotType_None,
	EHUDEquipmentSlotType_Up,
	EHUDEquipmentSlotType_Down,
	EHUDEquipmentSlotType_Left,
	EHUDEquipmentSlotType_Right,
	EHUDEquipmentSlotType_Max,
};

// 界面类型
UENUM(BlueprintType)
enum class EWidgetType :uint8
{
	EWIDGET_None,
	EWIDGET_PlayerStatus,
	EWIDGET_BossStatus,
	EWIDGET_MainMenu,
	EWIDGET_InterActPanel,
	EWIDGET_NPCDialog,
	EWIDGET_Inventory,
	EWIDGET_Max
};

// GAS的能力槽位枚举
UENUM(BlueprintType)
enum class EMyAbilitySlotsEnum : uint8
{
	PrimaryAbility,
	SecondaryAbility,
	UltimateAbility
};

UENUM(BlueprintType)
enum class EPawnStatusOperation :uint8
{
	EPawnStatusOperation_None,
	EPawnStatusOperation_Add,
	EPawnStatusOperation_Update,
	EPawnStatusOperation_Remove,
	EPawnStatusOperation_Max
};


/************************************************************************/
/*                              Struct                                         */
/************************************************************************/

// 槽位信息
struct FInventorySlot
{
	EInventoryCompartmentType SlotType;
	int SlotIndex;
};

// 连段消耗资源倍率
struct FAbilityCostInfo
{
	float HealthCost = 0;
	float StaminaCost = 0;
	float MagicCost = 0;
};

// 连段控制信息
struct FComboCoordinatorInfo
{
	// 武器基础伤害
	float BaseDamage;

	// 连段对应的伤害倍率
	TArray<float> ComboDamageMultiplier;

	// 连招最高上限
	int ComboMaxNum;

	// 当前连段
	int CurrentComboNum;

	// 当前连段窗口是否激活
	bool CanContinueCombo;

	// 对应连段的资源消耗(对应角色的基础属性,例如体力,血量,蓝量)
	TMap<int, FAbilityCostInfo> ComboStateCostMultiplier;
};

struct FStatusIconInfo
{
	int IconIndex;
	int32 TotalTime;
	int32 ElapsedTime;
};

