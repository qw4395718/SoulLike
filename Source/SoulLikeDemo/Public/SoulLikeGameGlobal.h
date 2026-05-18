#pragma once
#include "CoreMinimal.h"
#include "SL_Macros.h"
#include <Engine/DataTable.h>
#include <GameplayTagContainer.h>
#include "SoulLikeGameGlobal.generated.h"

class UGameplayAbility;
class USkeletalMesh;
class UAnimBlueprint;
class UBehaviorTree;
class UBlackboardData;

/************************************************************************/
/*                              Const                                         */
/************************************************************************/
#define UNLUA_INCLUDED

// 角色配置-背刺距离限制
const int BACKSTAB_DISTANCE_THRESHOLD = 150;

// 角色配置-背刺角度限制
const int BACKSTAB_ANGLE_THRESHOLD = 30;

// 角色配置-处决距离限制
const int EXECUTE_DISTANCE_THRESHOLD = 150;

// 特殊攻击(背刺,处决)检测半径
const int DETECTION_RADIUS = 150;

// 装备栏单栏的槽位数量
const int EQUIPMENT_SINGLE_CAPACITY = 4;

// 仓库分栏类型数量
const int INVENTORYTYPE_NUM = 12;

// 仓库单栏类型容纳数量
const int INVENTORY_SINGLE_CAPACITY = 100;

// 减伤最大上限
const float REDUCE_DAMAGE_PERCENTAGE = 0.9f;

// 体力消耗最小下限
const float REDUCE_STAMINACOST_PERCENTAGE = 0.5f;

// 对话选项栏最大缓存交互控件个数
const int INTERACT_BTN_MAX = 5;

// 对话选项栏控件高度
const int INTERACT_BTN_HEIGHT = 40;

/************************************************************************/
/*                              Enum                                         */
/************************************************************************/

//伤害类型
UENUM(BlueprintType)
enum class EDamageType :uint8
{
	SLASH      UMETA(DisplayName = "斩击"),      
	PIERCE     UMETA(DisplayName = "穿刺"),     
	BLUNT      UMETA(DisplayName = "打击"),      
	FIRE       UMETA(DisplayName = "火焰"),       
	Max			UMETA(Hidden)
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
	EWIDGET_PawnStatusInScreen,
	EWIDGET_BossStatus,
	EWIDGET_MainMenu,
	EWIDGET_InterActPanel,
	EWIDGET_NPCDialog,
	EWIDGET_Inventory,
	EWIDGET_BeginPlayScreen,
	EWIDGET_LobbyScreen,
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

UENUM(BlueprintType)
enum class EComboInputActionType :uint8
{
	EComboInputAction_None			UMETA(DisplayName = "None"),
	EComboInputAction_Light			UMETA(DisplayName = "Light"),
	EComboInputAction_Height		UMETA(DisplayName = "Height"),
	EComboInputAction_Special		UMETA(DisplayName = "Special"),
	EComboInputAction_Max			UMETA(Hidden),
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
USTRUCT(BlueprintType)
struct FComboInfo : public FTableRowBase
{
	GENERATED_BODY()

	// 当前连击所需要的Tag窗口名
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FGameplayTag	ActiveRequireWindowTag;

	// 连击所绑定的输入按键
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EComboInputActionType	InputActionType;

	// 连击所属GA
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UGameplayAbility> NextAbilityClass;

	// 本次连击消耗的体力值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
		float StaminaCost = 20.0f;

	// 本次连击的伤害倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
        float DamageMultiplier = 1.0f;
};

// ===== 武器类型枚举 =====
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Sword		UMETA(DisplayName = "剑"),
	Dagger		UMETA(DisplayName = "匕首"),
	Spear		UMETA(DisplayName = "矛"),
	GreatSword	UMETA(DisplayName = "大剑"),
	Shield		UMETA(DisplayName = "盾牌"),
	Bow			UMETA(DisplayName = "弓"),
	Staff		UMETA(DisplayName = "法杖"),
	Max			UMETA(Hidden)
};

// 武器持有状态
UENUM(BlueprintType)
enum class EWeaponHandType : uint8
{
	LeftHand		UMETA(DisplayName = "左手"),
	RightHand		UMETA(DisplayName = "右手"),
	TwoHand			UMETA(DisplayName = "双手"),
	Max				UMETA(Hidden)
};

// ===== 武器数据表结构 =====
USTRUCT(BlueprintType)
struct FWeaponDataInfo : public FTableRowBase
{
	GENERATED_BODY()

	// ===== 基础信息 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		int32 WeaponID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		FName WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		EWeaponType WeaponType;

	// ===== 资源 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
		TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
		TSoftObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
		TSoftObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
		TSoftObjectPtr<USoundBase> SwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
		TSoftObjectPtr<UParticleSystem> HitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
		TSoftObjectPtr<UAnimInstance> AnimClass;

	// ===== 碰撞盒配置 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
		FVector CollisionBoxSize;

	// 插槽偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
		FVector Offset;

	// 旋转
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotator")
		FRotator Rotator;

	// ===== 伤害配置 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		EDamageType DamageType = EDamageType::SLASH;

	// ===== 体力消耗倍率 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
		float StaminaCostMultiplier = 1.0f;

	// ===== 弹反窗口配置 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
		float ParryWindowDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
		float ParryStaminaCost = 20.0f;

	// ===== 是否双手武器 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		bool bIsTwoHanded = false;

	// ===== 是否左手武器 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		bool bIsLeftHanded = false;
};

// ===== 职业配置表结构 =====
USTRUCT(BlueprintType)
struct FClassConfigInfo : public FTableRowBase
{
	GENERATED_BODY()

		// ===== 基础信息 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		int32 ClassID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		FName ClassName;

	// ===== 属性配置 =====
/** 基础生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float BaseHealth = 100.0f;

	/** 基础攻击力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float BaseAttack = 10.0f;

	/** 基础防御力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float BaseDefense = 5.0f;

	/** 基础移动速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float BaseMoveSpeed = 300.0f;

	/** 基础体力（用于自身的耐力系统） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float BaseStamina = 100.0f;

	// ===== 武器配置 =====
	/** 左手武器ID（0表示无武器） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		int32 LeftHandWeaponID = 0;

	/** 右手武器ID（0表示无武器） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		int32 RightHandWeaponID = 0;

	// ===== 左手武器持有插槽 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		FName LeftHandSocketName = TEXT("None");

	// ===== 右手武器持有插槽 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		FName RightHandSocketName = TEXT("None");

	// ===== 双手武器持有插槽 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		FName TwoHandSocketName = TEXT("None");

	// ===== 道具配置 =====
	/** 道具集IDs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		TArray<int32> SlotItemIDs;
};

// ===== 波次配置表结构 =====
USTRUCT(BlueprintType)
struct FWaveConfigInfo : public FTableRowBase
{
	GENERATED_BODY()

		// ===== 波次信息 =====
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
		int32 WaveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
		FName WaveName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
		int32 LevelID;  // 所属关卡ID

		// ===== 怪物配置 =====
		/** 怪物类型列表（SpawnerID -> 数量） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
		TMap<int32, int32> MonsterSpawnCounts;  // SpawnerID -> Count

		/** 怪物生成延迟（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
		float SpawnDelay = 1.0f;

	/** 怪物生成间隔（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
		float SpawnInterval = 0.5f;

	// ===== 波次条件 =====
	/** 本波次所有怪物被消灭后，延迟下一波的时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
		float NextWaveDelay = 3.0f;

	/** 是否是最后一波 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
		bool bIsFinalWave = false;
};

// ===== 怪物生成点配置 =====
USTRUCT(BlueprintType)
struct FSpawnPointInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SpawnPointID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRotator Rotation;
};

// ===== 敌人类型枚举 =====
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Normal		UMETA(DisplayName = "普通"),
	Elite		UMETA(DisplayName = "精英"),
	Boss		UMETA(DisplayName = "Boss"),
	Minion		UMETA(DisplayName = "小兵"),
	Max			UMETA(Hidden)
};

// ===== 敌人攻击方式枚举 =====
UENUM(BlueprintType)
enum class EEnemyAttackType : uint8
{
	Melee		UMETA(DisplayName = "近战"),
	Ranged		UMETA(DisplayName = "远程"),
	Magic		UMETA(DisplayName = "魔法"),
	Mixed		UMETA(DisplayName = "混合"),
	Max			UMETA(Hidden)
};

// ===== 敌人配置表结构 =====
USTRUCT(BlueprintType)
struct FEnemyConfigInfo : public FTableRowBase
{
	GENERATED_BODY()

	// ===== 基础信息 =====
	/** 怪物ID（主键） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		int32 EnemyID;

	/** 怪物名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		FName EnemyName;

	/** 怪物类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		EEnemyType EnemyType;

	/** 攻击方式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		EEnemyAttackType AttackType;

	// ===== 属性配置 =====
	/** 基础生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float BaseHealth = 100.0f;

	/** 基础攻击力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float BaseAttack = 10.0f;

	/** 基础防御力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float BaseDefense = 5.0f;

	/** 基础移动速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float BaseMoveSpeed = 300.0f;

	/** 基础体力（用于敌人自身的耐力系统） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float BaseStamina = 100.0f;

	// ===== 外观配置 =====
	/** 骨骼网格体 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
		TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	/** 动画蓝图 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
		TSoftObjectPtr<UAnimBlueprint> AnimBlueprint;

	/** 左手武器ID（0表示无武器） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
		int32 LeftHandWeaponID = 0;
	
	// 左手武器的缩放倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
		float LeftHandWeaponScale = 1.0f;

	/** 右手武器ID（0表示无武器） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
		int32 RightHandWeaponID = 0;

	// 右手武器的缩放倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
		float RightHandWeaponScale = 1.0f;

	// ===== 左手武器持有插槽 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		FName LeftHandSocketName = TEXT("None");

	// ===== 右手武器持有插槽 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		FName RightHandSocketName = TEXT("None");

	// ===== 双手武器持有插槽 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
		FName TwoHandSocketName = TEXT("None");

	// ===== AI配置 =====
	/** AI行为树 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		TSoftObjectPtr<UBehaviorTree> BehaviorTree;

	/** AI黑板 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		TSoftObjectPtr<UBlackboardData> BlackboardData;

	/** 感知范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		float PerceptionRange = 1000.0f;

	/** 攻击范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		float AttackRange = 200.0f;

	/** 警戒范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		float AlertRange = 500.0f;

	// ===== 掉落配置 =====
	/** 掉落经验值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
		int32 DropExperience = 10;

	/** 掉落金币 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
		int32 DropGold = 5;

	/** 掉落物品列表（物品ID -> 掉落概率 0.0~1.0） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
		TMap<int32, float> DropItems;

	// ===== 连击配置 =====
	/** 此敌人使用的连击窗口Tag（对应ComboInfo表中的攻击模式） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
		FGameplayTag ComboWindowTag;

	/** GAS能力列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
		TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

	// ===== Mesh配置 =====
	/** 模型缩放 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
		FVector MeshScale {1.0f, 1.0f,1.0f,};

	// 模型相对旋转量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
		FRotator MeshRelativeRotate {0.0f, -90.0f, 0.0f};

	// 模型相对位移量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
		FVector MeshRelativeLocationOffset;

	/** 碰撞胶囊体半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
		float CapsuleRadius = 44.0f;

	/** 碰撞胶囊体高度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
		float CapsuleHalfHeight = 44.0f;

	
};