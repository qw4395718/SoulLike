# SoulLikeDemo API 文档

> 自动生成于 2026-05-12 | 基于 `SoulLikeDemo/Public/` 下所有头文件

---

## 公共定义与宏

### SL_Macros.h
**文件**: `SoulLikeDemo/Public/SL_Macros.h`

| 宏 | 描述 |
|---|---|
| `RETURN_IF_TRUE(Expression)` | 表达式为真则返回 |
| `RETURN_IF_FALSE(Expression)` | 表达式为假则返回 |
| `RETURN_VALUE_IF_TRUE(Expression, ReturnValue)` | 表达式为真则返回指定值 |
| `RETURN_VALUE_IF_FALSE(Expression, ReturnValue)` | 表达式为假则返回指定值 |

### SoulLikeGameGlobal.h — 全局常量与枚举
**文件**: `SoulLikeDemo/Public/SoulLikeGameGlobal.h`

**全局常量**:
- `BACKSTAB_DISTANCE_THRESHOLD = 150` — 背刺距离限制
- `BACKSTAB_ANGLE_THRESHOLD = 30` — 背刺角度限制
- `EXECUTE_DISTANCE_THRESHOLD = 150` — 处决距离限制
- `DETECTION_RADIUS = 150` — 特殊攻击检测半径
- `EQUIPMENT_SINGLE_CAPACITY = 4` — 装备栏单栏槽位数
- `INVENTORYTYPE_NUM = 12` — 仓库分栏类型数
- `INVENTORY_SINGLE_CAPACITY = 100` — 仓库单栏容纳量
- `REDUCE_DAMAGE_PERCENTAGE = 0.9f` — 减伤上限
- `REDUCE_STAMINACOST_PERCENTAGE = 0.5f` — 体力消耗下限
- `INTERACT_BTN_MAX = 5` — 对话选项栏最大缓存控件数
- `INTERACT_BTN_HEIGHT = 40` — 对话选项栏控件高度

**核心枚举**:

| 枚举 | 值 |
|---|---|
| `EDamageType` | SLASH (斩击), PIERCE (穿刺), BLUNT (打击), FIRE (火焰) |
| `EAttackType` | Normal_Combo_Phase_1/2/3, Skill_Combo_Phase_1/2/3, Special_Execution, Special_BackStab |
| `EWeaponComponentType` | MeleeAttack, Parry, Execute, BackStab |
| `EWeaponEquipState` | No_Equip, LH_Equip, RH_Equip |
| `EArrowKeyType` | ARROWKEY_None/Up/Down/Left/Right/Max |
| `EWeaponModeTyoe` | WEAPONMODE_Attack/Defence/ComboSkill/BackStab/Execute |
| `EWeaponCombaoSkillType` | COMBAOSKILL_Attack/Parry/AddBuff |
| `EEquipmentSlotType` | EQUIPMENT_Item_Up/Down, EQUIPMENT_Weapon_Left/Right |
| `EInventoryCompartmentType` | INVENTORYTYPE_Weapon/Helmet/Armor/Glove/Pant/Boot/Ammunition/Badge... (12种) |
| `EUIWidgetType` | EWIDGET_Bonfire |
| `EWeaponActionCostType` | EWeaponAction_Health/Stamina/Magic |
| `EAnimNotifyType` | 包含 LH/RH 的连招窗口、碰撞、弹反窗口等动画通知类型 |
| `EWeaponAnimNotifyType` | 对应武器侧的动画通知枚举 |
| `EHUDEquipmentSlotType` | EHUDEquipmentSlotType_Up/Down/Left/Right |
| `EWidgetType` | EWIDGET_PlayerStatus/PawnStatusInScreen/BossStatus/MainMenu/InterActPanel/NPCDialog/Inventory |
| `EMyAbilitySlotsEnum` | PrimaryAbility, SecondaryAbility, UltimateAbility |
| `EPawnStatusOperation` | EPawnStatusOperation_Add/Update/Remove |
| `EComboInputActionType` | EComboInputAction_Light/Height/Special |
| `EWeaponType` | Sword (剑), Dagger (匕首), Spear (矛), GreatSword (大剑), Shield (盾), Bow (弓), Staff (法杖) |
| `EWeaponHandType` | LeftHand (左手), RightHand (右手), TwoHand (双手) |
| `EEnemyType` | Normal (普通), Elite (精英), Boss (Boss), Minion (小兵) |
| `EEnemyAttackType` | Melee (近战), Ranged (远程), Magic (魔法), Mixed (混合) |

**核心结构体**:

| 结构体 | 描述 |
|---|---|
| `FInventorySlot` | 槽位信息: SlotType + SlotIndex |
| `FAbilityCostInfo` | 连段消耗倍率: HealthCost, StaminaCost, MagicCost |
| `FComboCoordinatorInfo` | 连段控制: BaseDamage, ComboDamageMultiplier[], ComboMaxNum, CurrentComboNum, CanContinueCombo, ComboStateCostMultiplier |
| `FStatusIconInfo` | 状态图标信息: IconIndex, TotalTime, ElapsedTime |
| `FComboInfo : FTableRowBase` | 连击信息表行: ActiveRequireWindowTag, InputActionType, NextAbilityClass, StaminaCost, DamageMultiplier |
| `FWeaponDataInfo : FTableRowBase` | 武器数据表行: WeaponID, WeaponName, WeaponType, 网格体/音效/特效资源, CollisionBoxSize, BaseDamage, DamageType, StaminaCostMultiplier, ParryWindowDuration, bIsTwoHanded, bIsLeftHanded |
| `FClassConfigInfo : FTableRowBase` | 职业配置: ClassID, ClassName, 属性(BaseHealth/Attack/Defense/MoveSpeed/Stamina), 武器ID, 插槽名, 道具ID列表 |
| `FWaveConfigInfo : FTableRowBase` | 波次配置: WaveID, WaveName, LevelID, MonsterSpawnCounts, SpawnDelay, SpawnInterval, NextWaveDelay, bIsFinalWave |
| `FSpawnPointInfo` | 生成点: SpawnPointID, Location, Rotation |
| `FEnemyConfigInfo : FTableRowBase` | 敌人配置: EnemyID, EnemyName, EnemyType, AttackType, 属性, 外观资源, 武器ID, AI配置(BehaviorTree/BlackboardData/感知范围/攻击范围), 掉落配置, 连击Tag, GAS能力列表 |

### DamageEventDispatcher.h — 伤害事件
**文件**: `SoulLikeDemo/Public/DamageEventDispatcher.h`

#### FWeaponStats : USTRUCT
**描述**: 武器基础属性结构。

**公开属性:**
- `float BaseDamage = 20.0f` — 基础伤害
- `float PoiseDamage = 10.0f` — 架势伤害
- `float StaminaCost = 15.0f` — 体力消耗
- `float CriticalMultiplier = 2.0f` — 暴击倍率
- `float WeaponCollisonBoxLength/Width/Height = 1.0f` — 碰撞盒尺寸
- `TMap<EAttackType, float> AttackTypeMultipliers` — 攻击类型倍率

#### FDamageData : USTRUCT
**描述**: 伤害计算临时数据。

- `float TotalDamage` / `PoiseDamage` / `StaminaCost`
- `bool bCanCritical`

#### FDamageEventData : USTRUCT
**描述**: 伤害事件数据。

**公开属性:**
- `float BaseDamage` / `FVector HitLocation` / `FVector HitNormal`
- `bool bIsCriticalHit` / `EDamageType AttackType` / `AActor* DamageCauser`

#### UDamageEventDispatcher : UObject
**描述**: 伤害事件分发器。

**公开方法:**
- `void BroadcastDamageEvent(const FDamageEventData& DamageEvent)` — 广播伤害事件

**公开属性:**
- `FOnDamageSignature OnDamageEvent` — 伤害事件委托

---

## AI 模块

### BlackboardKeys

#### USL_EnemyBlackboardKeys : UObject
**文件**: `SoulLikeDemo/Public/AI/SL_EnemyBlackboardKeys.h`
**描述**: 敌人AI黑板键定义，在蓝图创建黑板时使用。

**公开属性:**
- `static const FName bHasTarget` — 是否有目标 (Bool)
- `static const FName TargetActor` — 当前攻击目标 (Object)
- `static const FName TargetLocation` — 目标位置 (Vector)
- `static const FName DistanceToTarget` — 目标距离 (Float)
- `static const FName bInAttackRange` — 是否在攻击范围内 (Bool)
- `static const FName bHasSuspectedLocation` — 是否有可疑位置 (Bool)
- `static const FName SuspectedLocation` — 可疑位置 (Vector)
- `static const FName bInChaseRange` — 是否在追击范围内 (Bool)
- `static const FName bIsDead` — 是否死亡 (Bool)

### BehaviorTree

#### UBTService_UpdateTarget : UBTService
**文件**: `SoulLikeDemo/Public/AI/BT/BTService_UpdateTarget.h`
**描述**: 行为树服务，定期通过EQS搜索并更新目标信息。

**公开方法:**
- `virtual void TickNode(...)` — 每帧Tick触发目标更新

**公开属性:**
- `UEnvQuery* EQSQueryTemplate` — EQS查询模板
- `FBlackboardKeySelector TargetActorKey` — 目标Actor键
- `FBlackboardKeySelector TargetLocationKey` — 目标位置键
- `FBlackboardKeySelector SuspectedLocationKey` — 可疑位置键
- `FBlackboardKeySelector HasSuspectedLocationKey` — 是否有可疑位置键
- `FBlackboardKeySelector DistanceToTargetKey` — 目标距离键
- `FBlackboardKeySelector InAttackRangeKey` — 攻击范围键
- `FBlackboardKeySelector InChaseRangeKey` — 追击范围键

#### UBTTask_EnemyAttack : UBTTaskNode
**文件**: `SoulLikeDemo/Public/AI/BT/BTTask_EnemyAttack.h`
**描述**: 敌人攻击行为树任务，通过GAS激活攻击能力。

#### UBTTask_FindTarget : UBTTaskNode
**文件**: `SoulLikeDemo/Public/AI/BT/BTTask_FindTarget.h`
**描述**: 寻找目标行为树任务。

#### UBTTask_MoveToTarget : UBTTaskNode
**文件**: `SoulLikeDemo/Public/AI/BT/BTTask_MoveToTarget.h`
**描述**: 移动到目标位置的行为树任务。

#### UBTTask_WaitForCondition : UBTTaskNode
**文件**: `SoulLikeDemo/Public/AI/BT/BTTask_WaitForCondition.h`
**描述**: 等待条件满足的行为树任务。

### EQS

#### UEQC_TargetContext : UEnvQueryContext
**文件**: `SoulLikeDemo/Public/AI/EQS/EQC_TargetContext.h`
**描述**: EQS目标上下文。

#### UEQS_Generator_EnemyTargets : UEnvQueryGenerator
**文件**: `SoulLikeDemo/Public/AI/EQS/EQS_Generator_EnemyTargets.h`
**描述**: EQS敌人目标生成器。

#### UEQS_Test_CanSeeTarget : UEnvQueryTest
**文件**: `SoulLikeDemo/Public/AI/EQS/EQS_Test_CanSeeTarget.h`
**描述**: EQS视线检测测试。

#### UEQS_Test_DistanceScore : UEnvQueryTest
**文件**: `SoulLikeDemo/Public/AI/EQS/EQS_Test_DistanceScore.h`
**描述**: EQS距离评分测试。

---

## Class 模块

### ASL_CharacterBase : ACharacter, ICharacterComponent_IF, IAnimNotify_IF, IAbilitySystemInterface, IWeaponAccessory_IF, IActorState_IF
**文件**: `SoulLikeDemo/Public/Class/SL_CharacterBase.h`
**描述**: 角色基类，所有玩家和NPC角色的基础类。集成了GAS能力系统。

**公开方法:**
- `void InitializeCharacter()` — 白盒初始化角色
- `void InitPartmentComponent()` — 初始化部门组件
- `USL_InventoryComponent* GetInventoryComponentRef()` — 获取背包组件引用
- `void BindGASDeathEvent()` — 绑定GAS死亡事件
- `void OnGASCharacterDied(AActor*, AActor*)` — GAS角色死亡回调
- `void OnGASCharacterRevive(AActor*)` — GAS角色复活回调
- `UAbilitySystemComponent* GetAbilitySystemComponent()` — 获取ASC
- `void InitCharacterWithClassID(int32)` — 根据职业ID初始化
- `void SetClassID(int32)` — 设置职业ID

**公开属性:**
- `USL_AbilitySystemComponent* AbilitySystemComp` — GAS组件
- `USL_StatusAttributeSet* StatusAttributeSet` — 状态属性集
- `USL_CombatantComponent* CombatCmp` — 战斗组件
- `USL_EquipmentComponent* EquipmentCmp` — 装备组件
- `USL_HealthComponent* HealthCmp` — 生命组件
- `USL_InventoryComponent* InventoryCmp` — 背包组件
- `USL_StaminaComponent* StaminaCmp` — 体力组件
- `USL_StateComponent* StateCmp` — 状态组件
- `USL_MovementComponent* MovementCmp` — 移动组件
- `UWidgetComponent* ScreenWidgetCmp` — 屏幕UI组件
- `USL_ComboManagerComponent* ComboManagerCmp` — 连招管理组件
- `EPlayerState CurrentState` — 当前状态 (Alive/Dead)
- `FClassConfigInfo ClassConfig` — 职业配置

**枚举:**
- `EPlayerState` — Alive (存活), Dead (死亡)

### ASL_EnemyBase : ACharacter, IWeaponAccessory_IF, IAbilitySystemInterface, IActorState_IF
**文件**: `SoulLikeDemo/Public/Class/SL_EnemyBase.h`
**描述**: 敌人类，包含敌人配置加载、AI初始化、武器管理等。

**公开方法:**
- `ASL_WeaponBase* GetLeftHandWeapon()` — 获取左手武器
- `ASL_WeaponBase* GetRightHandWeapon()` — 获取右手武器
- `ASL_WeaponBase* GetWeaponByHand(int32)` — 按手获取武器
- `UAbilitySystemComponent* GetAbilitySystemComponent()` — 获取ASC
- `void InitializeEnemy(int32 EnemyID)` — 初始化敌人
- `EEnemyState GetEnemyState()` — 获取敌人状态
- `bool IsAlive()` / `bool IsDie()` — 生死判断
- `UBehaviorTree* GetBehaviorTree()` — 获取行为树
- `UBlackboardData* GetBlackboardData()` — 获取黑板
- `ASL_EnemyAIController* GetEnemyAIController()` — 获取AIC
- `void Die()` — 死亡处理
- `void RagDollStart()` / `RagDollEnd()` — 布娃娃
- `float GetPerceptionRange()` / `GetAttackRange()` — 感知/攻击范围
- `void SetPerceptionRange(float)` / `SetAttackRange(float)` — 设置范围
- `void BindGASDeathEvent()` — 绑定GAS死亡事件
- `void OnGASCharacterDied(AActor*, AActor*)` — GAS死亡回调

**公开属性:**
- `FOnEnemyDied OnEnemyDied` — 死亡委托
- `FEnemyConfigInfo EnemyConfig` — 敌人配置
- `float PerceptionRange = 1000.0f` / `AttackRange = 200.0f` — 感知/攻击范围
- `EEnemyState CurrentState` — 当前状态
- `USL_AbilitySystemComponent* AbilitySystemComp` — GAS组件
- `ASL_WeaponBase* LeftHandWeapon` / `RightHandWeapon` — 武器实例

**枚举:**
- `EEnemyState` — Alive (存活), Dead (死亡)

### ASL_EnemyAIController : AAIController
**文件**: `SoulLikeDemo/Public/Class/SL_EnemyAIController.h`
**描述**: 敌人AI控制器，管理行为树和黑板。

**公开方法:**
- `void InitializeAI(UBehaviorTree*, UBlackboardData*)` — 初始化AI
- `void SetTargetActor(AActor*)` — 设置目标
- `AActor* GetTargetActor()` — 获取目标
- `void SetBlackboardValue(const FName&, bool)` — 设置黑板Bool
- `void SetBlackboardValueAsObject(const FName&, UObject*)` — 设置黑板Object

### ASL_PlayerControllerBase : APlayerController
**文件**: `SoulLikeDemo/Public/Class/SL_PlayerControllerBase.h`
**描述**: 玩家控制器，处理输入绑定和UI管理。

**公开方法:**
- `void SetInputModeUIOnly(UWidget*)` / `SetInputModeGameOnly()` / `SetInputModeGameAndUI(...)` — 输入模式切换
- `void OnPlayerStateChanged()` — 玩家状态变化 (蓝图事件)
- `ASL_CharacterBase* GetMyPlayerCharacter()` — 获取控制的角色
- `void CreatePlayerStatusUI()` / `DestroyPlayerStatusUI()` — UI管理

### ASL_GameModeBase : AGameModeBase
**文件**: `SoulLikeDemo/Public/Class/SL_GameModeBase.h`
**描述**: 游戏模式基类，管理关卡流程和存档。

**公开方法:**
- `void StartCurrentLevel()` — 开始当前关卡
- `void RestartLevel()` — 重启关卡
- `void LoadLevel(int32)` — 加载指定关卡
- `void SaveCurrentProgress()` — 保存进度
- `void SetUseSaveData(bool)` — 设置是否使用存档
- `ALevelManager* GetLevelManager()` — 获取关卡管理器

### ASL_WeaponBase : AActor
**文件**: `SoulLikeDemo/Public/Class/SL_WeaponBase.h`
**描述**: 武器基类，管理碰撞检测、伤害计算和弹反窗口。

**公开方法:**
- `void InitializeWeaponWithID(int32, FName)` — 根据ID初始化
- `void InitializeFromDataRow(const FWeaponDataInfo&)` — 从数据行初始化
- `void EnableAttackCollision()` / `DisableAttackCollision()` — 攻击碰撞开关
- `void EnableParryWindow(float)` / `DisableParryWindow()` — 弹反窗口开关
- `bool IsParryWindowActive()` — 弹反窗口是否激活
- `int32 GetWeaponID()` — 获取武器ID
- `const FWeaponDataInfo& GetWeaponData()` — 获取武器数据
- `float GetBaseDamage()` — 获取基础伤害
- `float GetStaminaCostMultiplier()` — 获取体力消耗倍率

**公开属性:**
- `FOnWeaponHitDelegate OnWeaponHitDelegate` — 武器命中委托
- `FOnWeaponParryDelegate OnWeaponParryDelegate` — 弹反成功委托
- `FWeaponDataInfo WeaponData` — 武器配置数据

### UItemData : UObject
**文件**: `SoulLikeDemo/Public/Class/ItemDataObject.h`
**描述**: 物品基础数据结构，可在蓝图中创建和编辑。

**公开方法:**
- `void UseItem(AActor*)` — 使用物品 (BlueprintNativeEvent)
- `FLinearColor GetRarityColor()` — 根据稀有度获取颜色
- `bool CanStackWith(const UItemData*)` — 是否可堆叠
- `int32 MergeStack(UItemData*)` — 合并堆叠

**公开属性:**
- `FName ItemID` / `FText ItemName` / `FText ItemDescription`
- `UTexture2D* ItemIcon` / `int32 ItemCount` / `int32 MaxStackCount`
- `EItemRarity Rarity` / `EItemType ItemType`
- `float ItemWeight` / `int32 ItemValue`
- `bool bCanDrop` / `bool bCanUse`

**枚举:**
- `EItemRarity` — Common, Uncommon, Rare, Epic, Legendary
- `EItemType` — Consumable, Equipment, Material, Quest

### FItemDataRow : FTableRowBase
**文件**: `SoulLikeDemo/Public/Class/ItemDataStruct.h`
**描述**: 物品数据表行结构，包含GAS能力相关配置。

**公开属性:**
- 基础: `ItemID`, `ItemName`, `ItemDescription`, `ItemIcon`
- 堆叠: `ItemCount`, `MaxStackCount`
- 分类: `Rarity`, `ItemType`
- 经济: `ItemWeight`, `ItemValue`
- 行为: `bCanDrop`, `bCanUse`
- GAS: `UseAbilityTag`, `SelfEffectClass`, `TargetEffectClass`

### USL_GameSaveData : USaveGame
**文件**: `SoulLikeDemo/Public/Class/SL_GameSaveData.h`
**描述**: 游戏存档数据结构。

**公开属性:**
- `int32 SavedLevelID` — 已解锁最大关卡ID
- `int32 SavedPlayerClassID` — 玩家职业ID
- `FDateTime SaveTime` — 存档时间

---

## Component 模块

### Character 子模块

#### UCombatComponent : UActorComponent
**文件**: `SoulLikeDemo/Public/Component/Character/CombatComponent.h`
**描述**: 战斗组件，管理武器操作、攻击和伤害处理。

**公开方法:**
- `void InitWeaponInventory(TArray<ASL_WeaponBase*>)` — 初始化武器背包
- `void WeaponInventoryChange(int32, ASL_WeaponBase*)` — 武器背包变更
- `void DrawWeapon()` / `SheathWeapon()` — 拔/收武器
- `void SwitchToWeapon(int32)` — 切换武器
- `void PerformAttack()` / `PerformCombatSkill()` — 执行攻击/战技
- `bool CheckPerfectParry(float, float)` — 精准盾反判定
- `void ProcessAttackHit(AActor*, const FHitResult&)` — 攻击命中处理
- `void HandleDamage(const FDamageEventData&)` — 伤害处理
- `void HandleParry()` — 弹反处理
- `bool CanAction()` — 是否可执行动作
- `void ChangeAP(float)` — 消耗体力
- `void ReviveAP()` — 开启体力恢复

**公开属性:**
- `ASL_WeaponBase* LH_EquippedWeapon` / `RH_EquippedWeapon` — 当前装备武器
- `TArray<ASL_WeaponBase*> WeaponInventory` — 武器库存

#### USL_CombatantComponent : UActorComponent, ICombat_IF
**文件**: `SoulLikeDemo/Public/Component/Character/SL_CombatantComponent.h`
**描述**: 战斗者组件，实现 ICombat_IF 接口，处理战斗交互。

**公开方法 (ICombat_IF实现):**
- `void TakeDamage(float, FDamageEvent const&, ...)` — 承受伤害
- `int GetCurrentCombatState()` — 获取战斗状态
- `int GetTeamID()` — 获取阵营ID
- `void OnAttackEventCall()` — 攻击事件回调
- `void SetCanExecuteState(bool)` — 设置可处决状态
- `bool CanExecute()` / `bool CanBackStabs()` — 处决/背刺判定
- `void PerformAttack()` / `PerformDefence()` — 攻击/防御
- `void PerformExecuted(FName)` / `PerformBackStabbed(FName)` — 被处决/背刺
- `void MoveToLocationAndRotation(FVector, FRotator)` — 移动到指定位置

**公开方法:**
- `void InitCombatComponentInfo(AActor*, FString, int, bool)` — 初始化

#### USL_ComboManagerComponent : UActorComponent
**文件**: `SoulLikeDemo/Public/Component/Character/SL_ComboManagerComponent.h`
**描述**: 连招管理组件，处理输入缓存和连招衔接。

**公开方法:**
- `void HandleInputPressed(EComboInputActionType)` — 响应输入
- `void OnMontageBlendOut()` / `OnMontageFinished()` — 蒙太奇回调
- `void RegisterActiveComboTask(UAbilityTask_ComboMontage*)` — 注册ComboTask
- `void UnregisterActiveComboTask()` — 注销ComboTask
- `void SetNeedClearTag(FGameplayTag)` — 设置窗口标签
- `void ClearTargetWindowTag()` — 清理窗口标签
- `float GetCurrentComboDamageMultiplier()` — 当前连击伤害倍率
- `float GetCurrentComboStaminaCost()` — 当前连击体力消耗
- `const FComboInfo& GetCurrentComboInfo()` — 当前连击信息

#### USL_EquipmentComponent : UActorComponent, IWeaponAnimNotify_IF
**文件**: `SoulLikeDemo/Public/Component/Character/SL_EquipmentComponent.h`
**描述**: 装备组件，管理左右手武器装备和道具槽位。

**公开方法:**
- `void WeaponAnimNotifyResponse(int)` — 武器动画通知响应 (接口实现)
- `void InitializeWithClassID(int32)` — 根据职业初始化
- `void InitializeWithConfig(const FClassConfigInfo&)` — 根据配置初始化
- `ASL_WeaponBase* GetCurrentLeftHandWeapon()` — 获取左手武器
- `ASL_WeaponBase* GetCurrentRightHandWeapon()` — 获取右手武器
- `ASL_WeaponBase* GetWeaponByHand(int32)` — 按手获取武器
- `void UseSelectedSlotItem()` — 使用选中道具
- `int32 GetSelectSlotItemID()` — 获取选中道具ID
- `const FClassConfigInfo& GetClassConfig()` / `int32 GetClassID()` — 查询

**公开属性:**
- `FOnWeaponEquippedDelegate OnLeftHandWeaponEquipped` / `OnRightHandWeaponEquipped` — 武器装备委托

#### USL_HealthComponent : UActorComponent, IHealth_IF
**文件**: `SoulLikeDemo/Public/Component/Character/SL_HealthComponent.h`
**描述**: 生命值组件，管理HP和生死状态。

**公开方法:**
- `bool IsAlive()` — 是否存活
- `float GetCurrentHealthValue()` / `GetMaxHealthValue()` — HP查询
- `void ReduceCurrentHealth(float)` — 扣血
- `void ReviveCurrentHealth(float)` — 回血
- `void InitHealthInfo(float)` — 初始化

#### USL_InventoryComponent : UActorComponent
**文件**: `SoulLikeDemo/Public/Component/Character/SL_InventoryComponent.h`
**描述**: 库存组件（GAS + DataTable 重构版），管理道具使用。

**公开方法:**
- `void InitializeInventory()` — 初始化
- `bool AddItemByID(FName, int32 = 1)` — 添加道具
- `bool RemoveItemByID(FName, int32 = 1)` — 移除道具
- `int32 GetItemCount(FName)` — 获取道具数量
- `TArray<FName> GetAllItemIDs()` — 获取所有道具ID
- `bool UseItemByID(FName)` — 使用道具
- `bool UseSelectedItem()` — 使用选中道具
- `void SetSelectedItemID(FName)` / `FName GetSelectedItemID()` — 选中道具
- `bool CanUseItem(FName)` — 是否可使用

#### USL_MovementComponent : UActorComponent
**文件**: `SoulLikeDemo/Public/Component/Character/SL_MovementComponent.h`
**描述**: 移动组件，管理翻滚等特殊移动能力。

**公开方法:**
- `void InitMovemenetInfo(bool, FString)` — 初始化
- `void ExeRoll()` — 执行翻滚

#### USL_StaminaComponent : UActorComponent, IStamina_IF
**文件**: `SoulLikeDemo/Public/Component/Character/SL_StaminaComponent.h`
**描述**: 体力组件，基于GAS管理体力消耗、恢复和透支。

**公开方法:**
- `void InitializeStaminaComponent()` — 初始化
- `bool CanAffordCost(float)` — 是否有足够体力
- `bool IsOverdrawAttack(float)` — 是否透支攻击
- `float GetCurrentStamina()` / `GetMaxStamina()` — 体力查询
- `EStaminaRegenState GetRegenState()` — 恢复状态
- `float GetStaminaPercentage()` — 体力百分比
- `void ConsumeStamina(float)` — 消耗体力
- `void OnComboStarted()` / `OnComboEnded()` — 连击生命周期
- `void SetRegenEffect(TSubclassOf<UGameplayEffect>)` / `SetDelayEffect(...)` / `SetCostEffect(...)` — GE配置

**枚举:**
- `EStaminaRegenState` — Regenerating, Paused, Delayed

#### USL_StateComponent : UActorComponent, IStateCalculate_IF
**文件**: `SoulLikeDemo/Public/Component/Character/SL_StateComponent.h`
**描述**: 状态结算组件，处理伤害/体力等属性结算。

**公开方法 (IStateCalculate_IF实现):**
- `float DamageDealtCalculate(float)` — 造成伤害结算
- `float DamageReceivedCalculate(float)` — 承受伤害结算
- `float StaminaCostCalculate(float)` — 体力消耗结算

### Weapon 子模块

#### UWeaponComboCoordinatorComponent : UActorComponent, IWeaponCoordinatorControl_IF
**文件**: `SoulLikeDemo/Public/Component/Weapon/WeaponComboCoordinatorComponent.h`
**描述**: 武器连招协调组件，管理连段伤害和状态消耗。

**公开方法:**
- `float GetAttackComboDamage(int)` — 获取连段伤害
- `float GetAttackStateCost(int, int)` — 获取状态消耗
- `void InitComboCoordinatorComponet(const TMap<EWeaponModeTyoe, FComboCoordinatorInfo>)` — 初始化
- `void ActiveComboWindowInputState(EWeaponModeTyoe)` / `InActiveComboWindowInputState(...)` — 窗口开关
- `int GetNextComboNum(EWeaponModeTyoe)` — 获取下一段连招序号

#### UWeaponMeleeAttackComponent : USceneComponent
**文件**: `SoulLikeDemo/Public/Component/Weapon/WeaponMeleeAttackComponent.h`
**描述**: 武器近战攻击组件，管理碰撞检测和伤害应用。

**公开方法:**
- `void InitalizeWeaponComponent(AActor*, FVector)` — 初始化
- `void EnableCollisionBoxCheck()` / `DisableCollisionBoxCheck()` — 碰撞开关
- `void EnableParryWindowCheck(float)` / `DisableParryWindowCheck()` — 弹反窗口
- `bool IsActiveParryWindow()` — 是否在弹反窗口
- `float CalculateFinalDamage(AActor*)` — 伤害计算 (BlueprintNativeEvent)

#### UWeaponParryComponent : USceneComponent
**文件**: `SoulLikeDemo/Public/Component/Weapon/WeaponParryComponent.h`
**描述**: 武器弹反组件，管理弹反碰撞检测。

**公开方法:**
- `void InitalizeWeaponComponent(AActor*, FVector)` — 初始化
- `void EnableCollisionBoxCheck()` / `DisableCollisionBoxCheck()` — 碰撞开关

---

## Interface 模块

### IActorState_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/ActorState_IF.h`
**描述**: Actor生死状态接口。

- `bool IsAlive()` — 是否存活
- `bool IsDie()` — 是否死亡

### IAnimNotify_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/AnimNotify_IF.h`
**描述**: 动画通知响应接口。

- `void AnimNotifyResponse(int NotifyType)` — 动画通知响应

### IBehavioralResponse_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/BehavioralResponse_IF.h`
**描述**: 行为响应接口，处理攻击/防御/弹反/背刺/处决等行为。

- 攻击/防御/弹反/处决/背刺等响应方法 (10个)

### ICharacterComponent_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/CharacterComponent_IF.h`
**描述**: 角色组件获取接口，提供统一组件访问。

- `UActorComponent* GetCombatantComponent()` — 战斗组件
- `UActorComponent* GetEquipmentComponent()` — 装备组件
- `UActorComponent* GetHealthComponent()` — 生命组件
- `UActorComponent* GetInventoryComponent()` — 背包组件
- `UActorComponent* GetSpecialMovementComponent()` — 移动组件
- `UActorComponent* GetStaminaComponent()` — 体力组件
- `UActorComponent* GetStateComponent()` — 状态组件
- `UActorComponent* GetComboManagerComponent()` — 连招管理组件

### ICharacterInfo_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/CharacterInfo_IF.h`
**描述**: 角色信息接口 (无公开方法)。

### ICombat_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/Combat_IF.h`
**描述**: 战斗接口，定义战斗行为。

- `void TakeDamage(float, FDamageEvent const&, AController*, AActor*)` — 承受伤害
- `int GetCurrentCombatState()` — 当前战斗状态
- `int GetTeamID()` — 阵营ID
- `void OnAttackEventCall()` — 攻击事件
- `void SetCanExecuteState(bool)` — 处决状态
- `bool CanExecute()` / `bool CanBackStabs()` — 处决/背刺判定
- `void PerformAttack()` / `PerformDefence()` — 攻击/防御
- `void PerformExecuted(FName)` / `PerformBackStabbed(FName)` — 被处决/背刺
- `void MoveToLocationAndRotation(FVector, FRotator)` — 移动到位置

### IHealth_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/Health_IF.h`
**描述**: 生命值接口。

- `bool IsAlive()` — 存活判定
- `float GetCurrentHealthValue()` / `GetMaxHealthValue()` — HP查询
- `void ReduceCurrentHealth(float)` — 扣血
- `void ReviveCurrentHealth(float)` — 回血

### IInteraction_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/Interaction_IF.h`
**描述**: 交互接口。

- 交互/对话相关方法 (4个)

### ILockRotation_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/LockRotation_IF.h`
**描述**: 锁定旋转接口。

### IScreenWidget_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/ScreenWidget_IF.h`
**描述**: 屏幕空间Widget接口。

- `void SetOwningPawn(AActor*)` — 设置所属Pawn

### IStamina_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/Stamina_IF.h`
**描述**: 体力接口。

### IStateCalculate_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/StateCalculate_IF.h`
**描述**: 状态结算接口。

- `float DamageDealtCalculate(float)` — 造成伤害结算
- `float DamageReceivedCalculate(float)` — 承受伤害结算
- `float StaminaCostCalculate(float)` — 体力消耗结算

### IWeaponAccessory_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/WeaponAccessory_IF.h`
**描述**: 武器配件接口。

- `ASL_WeaponBase* GetLeftHandWeapon()` — 左手武器
- `ASL_WeaponBase* GetRightHandWeapon()` — 右手武器
- `ASL_WeaponBase* GetWeaponByHand(int32)` — 按手获取武器

### IWeaponAnimNotify_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/WeaponAnimNotify_IF.h`
**描述**: 武器动画通知接口。

- `void WeaponAnimNotifyResponse(int NotifyType)` — 武器动画通知

### IWeaponCoordinatorControl_IF : UInterface
**文件**: `SoulLikeDemo/Public/Interface/WeaponCoordinatorControl_IF.h`
**描述**: 武器连招协调控制接口。

- `float GetAttackComboDamage(int)` — 连段伤害
- `float GetAttackStateCost(int, int)` — 状态消耗

---

## GAS 模块

### AttributeSet

#### USL_StatusAttributeSet : UAttributeSet
**文件**: `SoulLikeDemo/Public/GAS/AS/SL_StatusAttributeSet.h`
**描述**: 状态属性集，管理血量、伤害、体力的GAS属性。

**公开方法:**
- `void SetOwningActor(AActor*)` — 设置持有者
- `void InitHealthAS(float, float)` — 初始化血量 (BlueprintNativeEvent)
- `void InitStaminaAS(float, float)` — 初始化体力 (BlueprintNativeEvent)
- `void OnRep_CurrentHealth()` / `OnRep_CurrentStamina()` — 网络复制回调

**公开属性:**
- `FGameplayAttributeData Health` — 当前血量 (ReplicatedUsing)
- `FGameplayAttributeData MaxHealth` — 最大血量 (Replicated)
- `FGameplayAttributeData Damage` — Meta属性: 暂存伤害值
- `FGameplayAttributeData Stamina` — 当前体力 (ReplicatedUsing)
- `FGameplayAttributeData MaxStamina` — 最大体力 (Replicated)
- `FGameplayAttributeData StaminaCost` — Meta属性: 暂存消耗值
- `FGameplayAttributeData StaminaRegen` — Meta属性: 暂存恢复值

**Accessor宏**: `ATTRIBUTE_ACCESSORS` — 为 Health, MaxHealth, Damage, Stamina, MaxStamina, StaminaCost, StaminaRegen 生成Getter/Setter

### AbilitySystemComponent

#### USL_AbilitySystemComponent : UAbilitySystemComponent
**文件**: `SoulLikeDemo/Public/GAS/ASC/SL_AbilitySystemComponent.h`
**描述**: 自定义ASC，提供蓝图友好的能力管理接口。

**公开方法:**
- `FGameplayAbilitySpecHandle GiveAbilityForBP(TSubclassOf<UGameplayAbility>, int32, int32, UObject*)` — 授予能力
- `FGameplayAbilitySpecHandle GiveAbilityAndActivateOnceForBP(...)` — 授予并激活
- `TArray<FGameplayAbilitySpec>& GetActivatableAbilitiesForBP()` — 获取可激活能力
- `bool TryActivateAbilityByHandle(FGameplayAbilitySpecHandle)` — 按Handle激活
- `void ClearAbilityByHandle(FGameplayAbilitySpecHandle)` — 清理能力
- `TArray<UGameplayTask*> GetCurrentlyActiveTasks()` — 获取活跃Task
- `bool TryActivateAbilityByTag(const FGameplayTag&)` — 按Tag激活 (供BTTask)
- `FGameplayAbilitySpecHandle FindAbilitySpecHandleByTag(const FGameplayTag&)` — 按Tag查找
- `UGameplayAbility* GetActiveAbilityInstanceByTag(const FGameplayTag&)` — 按Tag获取实例
- `FGameplayAbilitySpec* FindAbilitySpecFromTag(const FGameplayTag&)` — 按Tag查找Spec
- `void SetAliveTag()` — 设置存活Tag

### AbilityTask

#### UAbilityTask_ComboMontage : UAbilityTask
**文件**: `SoulLikeDemo/Public/GAS/AT/AbilityTask_ComboMontage.h`
**描述**: 连招专用蒙太奇播放Task，管理动画播放和混合时机。

**公开方法:**
- `static UAbilityTask_ComboMontage* CreateComboMontageTask(UGameplayAbility*, UAnimMontage*, float BlendOutTime, float PlayRate)` — 工厂方法
- `bool IsReadyToBlend()` — 是否到达AllowBlend位置
- `void OnAllowBlendReached(FGameplayTag)` — 标记已到达
- `EComboInputHandledResult OnInputReceived(EComboInputActionType)` — 输入处理

**公开属性:**
- `FOnComboMontageCompleted OnCompleted` — 正常播完委托
- `FOnComboMontageInterrupted OnInterrupted` — 被连招打断委托

**枚举:**
- `EComboInputHandledResult` — Accepted, AcceptedAndBlended, Rejected

### GameplayAbility

#### USL_GameplayAbilityBase : UGameplayAbility
**文件**: `SoulLikeDemo/Public/GAS/GA/SL_GameplayAbilityBase.h`
**描述**: GA基类，支持Lua脚本和GE应用。

**公开方法:**
- `void OnAbilityActivatedForLua(...)` — Lua激活事件 (BlueprintImplementableEvent)
- `void EndAbilityForBP(...)` — 结束能力
- `void PlayMontageForAbility(UAnimMontage*, ...)` — 播放蒙太奇
- `void ApplyEffectToTarget(TSubclassOf<UGameplayEffect>, AActor*, float)` — 应用GE

**公开属性:**
- `FString LuaFilePath` — Lua文件路径
- `float ManaCost = 10.0f` / `CooldownDuration = 2.0f` — 配置数据

#### USL_GameplayAbilityComboBase : UGameplayAbility
**文件**: `SoulLikeDemo/Public/GAS/GA/SL_GameplayAbilityComboBase.h`
**描述**: 连招GA基类，集成ComboMontageTask和连招窗口。

**公开方法:**
- `void OnAbilityActivatedForLua(...)` — Lua激活事件
- `void EndAbilityForBP(...)` — 结束能力
- `void ApplyEffectToTarget(TSubclassOf<UGameplayEffect>, const AActor*, float)` — 应用GE

**公开属性:**
- `UAnimMontage* AttackMontage` — 攻击蒙太奇
- `float BlendOutTime = 0.15f` — 被打断混合时间
- `bool bRespectBlendWindow = true` — 是否等待AllowBlend
- `FString LuaFilePath` — Lua文件路径
- `float ManaCost = 10.0f` / `CooldownDuration = 2.0f`

#### USL_GameplayAbilityNPCBase : UGameplayAbility
**文件**: `SoulLikeDemo/Public/GAS/GA/SL_GameplayAbilityNPCBase.h`
**描述**: NPC GA基类，支持蒙太奇完成委托供BTTask监听。

**公开方法:**
- `EMontagePlayState GetMontagePlayState()` — 蒙太奇播放状态
- `void EndAbilityForBP(...)` — 结束能力
- `void PlayMontageForAbility(UAnimMontage*, ...)` — 播放蒙太奇
- `void ApplyEffectToTarget(TSubclassOf<UGameplayEffect>, AActor*, float)` — 应用GE

**公开属性:**
- `FString LuaFilePath` / `float ManaCost` / `float CooldownDuration`
- `FGameplayTag ActivationTag` — 激活所需的Tag (供BTTask)
- `FOnMontageCompletedDelegate OnMontageCompletedDelegate` — 蒙太奇完成委托
- `TSubclassOf<UGameplayEffect> EffectClass` — GE类
- `UAnimMontage* AttackMontage` — 攻击蒙太奇

**枚举:**
- `EMontagePlayState` — None, Playing, Completed, Interrupted

#### USL_GameplayAbilityUseItem : USL_GameplayAbilityBase
**文件**: `SoulLikeDemo/Public/GAS/GA/SL_GameplayAbilityUseItem.h`
**描述**: 通用道具使用能力，通过事件传递的道具ID查表应用效果。

**公开属性:**
- `UAnimMontage* UseItemMontage` — 使用道具蒙太奇

---

## Manager 模块

### UDataTableManager : UGameInstanceSubsystem
**文件**: `SoulLikeDemo/Public/Manager/DataTableManager.h`
**描述**: 数据表管理器，统一管理所有数据表的加载和访问。

**公开方法:**
- `static UDataTableManager* Get(const UObject*)` — 获取单例
- `void InitializeManager(UDataTable* = nullptr)` — 初始化
- `UBaseDataTable* GetDataTable(EDataTableType)` — 获取数据表
- `bool IsDataTableLoaded(EDataTableType)` — 检查加载状态
- `void RegisterTableClass(EDataTableType, TSubclassOf<UBaseDataTable>)` — 注册类型

### UGlobalDelegatesManager : UGameInstanceSubsystem
**文件**: `SoulLikeDemo/Public/Manager/GlobalDelegatesManager.h`
**描述**: 全局委托管理器，提供跨系统的事件通信。

**公开方法:**
- `static UGlobalDelegatesManager* Get(const UObject*)` — 获取单例
- `void BroadcastDialogShow(const FString&, const FString&)` — 广播对话框
- `void BroadcastOnClickMainMenuButton(const int)` — 广播菜单点击
- `void BroadcastItemUsed(AActor*, FName)` — 广播道具使用
- `void BroadcastItemEffectTriggered(AActor*, FName, AActor*)` — 广播效果触发
- `void BroadcastItemCountChanged(AActor*, FName, int32)` — 广播数量变更

**公开属性 (委托):**
- `FOnClickInterActBtnToDialog OnClickInterActBtnToDialog`
- `FOnClickInterMainMenuButton OnClickInterMainMenuButton`
- `FAttributeHealthChangedEvent OnAttributeHealthChanged` — 血量变动
- `FAttributeStaminaChangedEvent OnAttributeStaminaChanged` — 体力变动
- `FOnCharacterDiedEvent OnCharacterDied` — 死亡事件
- `FOnCharacterRevivedEvent OnCharacterRevived` — 复活事件
- `FOnItemUsedEvent OnItemUsed` — 道具使用
- `FOnItemEffectTriggeredEvent OnItemEffectTriggered` — 道具效果
- `FOnItemCountChangedEvent OnItemCountChanged` — 道具数量

### ALevelManager : AActor
**文件**: `SoulLikeDemo/Public/Manager/LevelManager.h`
**描述**: 关卡管理器，处理波次流程和关卡状态。

**公开方法:**
- `void OnWaveStarted(int32)` / `OnWaveCompleted(int32)` / `OnAllWavesCompleted()` — 波次回调
- `void OnPlayerDied()` — 玩家死亡回调
- `void StartLevel(int32, int32 = 1)` — 开始关卡
- `void RetryLevel()` / `GoToNextLevel()` — 关卡控制

### UNotifyMessageManager : UObject
**文件**: `SoulLikeDemo/Public/Manager/NotifyMessageManager.h`
**描述**: 通知消息管理器。

**公开方法:**
- `void FakeInit()` — 单元测试初始化
- `void ShowNotification(const FText&)` — 显示通知
- `void ClearAllNotifications()` — 清除所有通知

### USL_GameSaveSubsystem : UGameInstanceSubsystem
**文件**: `SoulLikeDemo/Public/Manager/SL_GameSaveSubsystem.h`
**描述**: 游戏存档管理子系统。

**公开方法:**
- `bool SaveGame(int32, int32)` — 保存游戏
- `bool LoadGame(int32&, int32&)` — 加载游戏
- `bool HasSaveData()` — 是否有存档
- `bool DeleteSaveData()` — 删除存档
- `USL_GameSaveData* GetSaveData()` — 获取存档对象
- `static USL_GameSaveSubsystem* Get(const UObject*)` — 获取单例

### UUIManagerSubsystem : UGameInstanceSubsystem
**文件**: `SoulLikeDemo/Public/Manager/UIManagerSubsystem.h`
**描述**: UI管理器，统一管理屏幕空间和世界空间UI。

**公开方法:**
- `static UUIManagerSubsystem* Get(const UObject*)` — 获取单例
- `void RegisterWidgetFromBPPath(EWidgetType, const FString&)` — 按路径注册
- `void RegisterWidget(EWidgetType, TSubclassOf<UUserWidget>)` — 注册Widget
- `void UnregisterWidget(EWidgetType)` — 注销Widget
- `void OpenWidget(const FUICreateParams&)` — 打开界面
- `void OpenScreenWidget(EWidgetType)` — 打开屏幕空间UI
- `void OpenWorldWidgetWithActor(const FUICreateParams&)` — 打开世界空间UI
- `void CloseWidget(EWidgetType)` / `CloseAllWidgets()` — 关闭
- `void SetWidgetVisible(EWidgetType)` / `ToggleWidgetVisible(EWidgetType)` — 可见性
- `UUserWidget* GetWidget(EWidgetType)` / `bool IsWidgetOpen(EWidgetType)` — 查询
- `void PushWidget(EWidgetType)` / `PopWidget(EWidgetType)` — 页面栈
- `void SetFocusToWidget(EWidgetType)` / `FName GetFocusedWidgetName()` — 焦点
- `void UpdateHealthUI(float)` — 更新血量UI

**附带结构体:**
- `FUICreateParams` — UI创建参数: Type, WorldContextObject, TargetActor, WorldOffset, bEnableDistanceCulling, DestroyDistance

### UWaveManagerSystem : UWorldSubsystem
**文件**: `SoulLikeDemo/Public/Manager/WaveManagerSystem.h`
**描述**: 波次管理系统，控制关卡怪物波次的生成和状态。

**公开方法:**
- `void StartLevel(int32)` — 开始关卡
- `void StartNextWave()` — 开始下一波
- `void PauseWave()` / `ResumeWave()` / `ResetLevel()` — 流程控制
- `int32 GetCurrentWaveID()` / `GetTotalWaveCount()` / `GetRemainingEnemyCount()` — 查询
- `EWaveState GetCurrentWaveState()` / `bool IsAllWavesCompleted()` — 状态查询

**公开属性:**
- `FOnWaveStateChanged OnWaveStarted` / `OnWaveCompleted` — 波次委托
- `FOnAllWavesCompleted OnAllWavesCompleted` — 全部完成委托

**枚举:**
- `EWaveState` — None, Preparing, Spawning, Active, Completed, Failed

---

## Notify 模块

所有通知类继承自 `UAnimNotifyState`。

### USL_ActiveParryWindow_NS : UAnimNotifyState
**文件**: `SoulLikeDemo/Public/Notify/SL_ActiveParryWindow_NS.h`
**描述**: 激活弹反窗口通知状态。

**公开属性:**
- `float SuccessWindowExtension = 0.05f` — 弹反成功额外帧数
- `bool bIsLHActive` / `bIsRHActive` — 左右手激活标记

### USL_ActiveParry_CB_NS : UAnimNotifyState
**文件**: `SoulLikeDemo/Public/Notify/SL_ActiveParry_CB_NS.h`
**描述**: 激活弹反碰撞盒通知状态。

**公开属性:**
- `bool bIsLHActive` / `bIsRHActive` — 左右手激活标记

### USL_ActiveWeapon_CB_NS : UAnimNotifyState
**文件**: `SoulLikeDemo/Public/Notify/SL_ActiveWeapon_CB_NS.h`
**描述**: 激活武器碰撞盒通知状态。

**公开属性:**
- `bool bIsLHActive` / `bIsRHActive` — 左右手激活标记

### USL_Attack_JumpSection_NS : UAnimNotifyState
**文件**: `SoulLikeDemo/Public/Notify/SL_Attack_JumpSection_NS.h`
**描述**: 攻击跳段通知状态。

**公开属性:**
- `FName JumpSectionName` — 跳转段名
- `bool bIsLHActive` / `bIsRHActive` — 左右手激活标记

### USL_BackStabbing_CA_NS : UAnimNotifyState
**文件**: `SoulLikeDemo/Public/Notify/SL_BackStabbing_CA_NS.h`
**描述**: 背刺通知状态。

### USL_CharacterAnim_NS : UAnimNotifyState
**文件**: `SoulLikeDemo/Public/Notify/SL_CharacterAnim_NS.h`
**描述**: 角色动画通知状态，通过动画事件触发角色行为。

**公开属性:**
- `EAnimNotifyType AnimStartNotify` — 开始通知类型
- `EAnimNotifyType AnimEndNotify` — 结束通知类型

### USL_ComboWindow_ANS : UAnimNotifyState
**文件**: `SoulLikeDemo/Public/Notify/SL_ComboWindow_ANS.h`
**描述**: 连击窗口动画通知状态。

### USL_Executing_CA_NS : UAnimNotifyState
**文件**: `SoulLikeDemo/Public/Notify/SL_Executing_CA_NS.h`
**描述**: 处决（主动方）通知状态。

### USL_WaitExecuted_CA_NS : UAnimNotifyState
**文件**: `SoulLikeDemo/Public/Notify/SL_WaitExecuted_CA_NS.h`
**描述**: 等待被处决（被动方）通知状态。

---

## Struct 模块

### FStatusEffectInfo : USTRUCT
**文件**: `SoulLikeDemo/Public/Struct/StatusEffectInfo.h`
**描述**: 状态效果信息。

**公开属性:**
- `int IconIndex` — 图标索引
- `UTexture2D* Icon` — 图标
- `float Duration` — 持续时间
- `float RemainingTime` — 剩余时间
- `int32 Stacks` — 层数

### FWeaponData : USTRUCT
**文件**: `SoulLikeDemo/Public/Struct/WeaponData.h`
**描述**: 武器运行时数据结构，支持网络序列化。

**公开属性:**
- `int32 WeaponID` / `WeaponUniqueID` — 武器标识
- `FString Mesh` / `SocketName` / `AnimClass` — 资源路径
- `FString AttackMentageName` / `ComboSkillMentageName` / `ExecuteMentageName` / `BackStabMentageName` — 蒙太奇路径
- `FVector WeaponCollisionBoxSize` — 碰撞盒大小
- `TMap<EWeaponModeTyoe, FComboCoordinatorInfo> ComboCoordinatorInfoMap` — 连招映射
- `TMap<EAttackType, float> APCostMap` / `MPCostMap` — 消耗映射
- `TMap<EWeaponComponentType, bool> NeedLoadComponentInfoMap` — 组件加载标记
- `bool NetSerialize(FArchive&, UPackageMap*, bool&)` — 网络序列化

---

## Table 模块

所有Table类继承自 `UBaseDataTable`。

### UBaseDataTable : UObject
**文件**: `SoulLikeDemo/Public/Table/BaseDataTable.h`
**描述**: 数据表基类，定义加载生命周期。

**公开方法:**
- `virtual bool InitializeFromAsset(TSoftObjectPtr<UDataTable>)` — 从资产初始化
- `virtual bool PostExecuteData()` — 数据后处理
- `ETableLoadState GetLoadState()` / `bool IsLoaded()` — 状态查询
- `virtual UBaseDataTable* GetDataTable()` — 获取数据表

**枚举:**
- `ETableLoadState` — NotLoaded, Loading, Loaded, Failed

### UClassConfigInfoTable : UBaseDataTable
**文件**: `SoulLikeDemo/Public/Table/ClassConfigInfoTable.h`
**描述**: 职业配置表。

- `bool GetClassConfig(int32, FClassConfigInfo&)` — 获取职业配置
- `TArray<int32> GetAllClassIDs()` — 获取所有职业ID

### UComboInfoTable : UBaseDataTable
**文件**: `SoulLikeDemo/Public/Table/ComboInfoTable.h`
**描述**: 连击信息表。

- `bool FindNextComboInfo(const FGameplayTagContainer&, EComboInputActionType, FComboInfo&)` — 查找下一招

### UEnemyConfigInfoTable : UBaseDataTable
**文件**: `SoulLikeDemo/Public/Table/EnemyConfigInfoTable.h`
**描述**: 敌人配置表。

- `bool GetEnemyConfig(int32, FEnemyConfigInfo&)` — 获取敌人配置
- `TArray<FEnemyConfigInfo> GetEnemiesByType(EEnemyType)` — 按类型获取
- `TArray<int32> GetAllEnemyIDs()` — 获取所有敌人ID

### UItemDataTable : UBaseDataTable
**文件**: `SoulLikeDemo/Public/Table/ItemDataTable.h`
**描述**: 道具数据表。

- `bool GetItemData(FName, FItemDataRow&)` — 获取道具数据
- `TArray<FName> GetAllItemIDs()` — 获取所有道具ID

### UWaveConfigInfoTable : UBaseDataTable
**文件**: `SoulLikeDemo/Public/Table/WaveConfigInfoTable.h`
**描述**: 波次配置表。

- `bool GetWavesForLevel(int32, TArray<FWaveConfigInfo>&)` — 按关卡获取波次
- `bool GetWaveConfig(int32, FWaveConfigInfo&)` — 按波次ID获取
- `int32 GetWaveCountForLevel(int32)` — 关卡波次数

### UWeaponDataTable : UBaseDataTable
**文件**: `SoulLikeDemo/Public/Table/WeaponDataTable.h`
**描述**: 武器数据表。

- `bool GetWeaponData(int32, FWeaponDataInfo&)` — 获取武器数据
- `TArray<int32> GetAllWeaponIDs()` — 获取所有武器ID

### FDataTableConfig : FTableRowBase
**文件**: `SoulLikeDemo/Public/Table/DataTableConfig.h`
**描述**: 数据表配置结构。

- `FString TableName` / `EDataTableType TableType`
- `TSoftObjectPtr<UDataTable> DataTableAsset`
- `bool bAutoLoad = true` / `int32 Priority = 0`

**枚举:**
- `EDataTableType` — DT_ComboInfo, DT_WeaponDataInfo, DT_ClassConfigInfo, DT_WaveConfigInfo, DT_EnemyConfigInfo, DT_ItemConfigInfo

---

## UMG 模块

### HUD 子模块

#### UHUD_PlayerStatusBar : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_PlayerStatusBar.h`
**描述**: 玩家状态栏，显示血量/体力/魔法。

**公开方法:**
- `void BindGlobalDelegatesEvent()` — 绑定全局委托
- `void SetProgressBarLimit(EPlayerStatusAttributeType, float, float)` — 设置进度条范围
- `void UpdateProgressInfo(EPlayerStatusAttributeType, float, float)` — 更新进度
- `void ChangePlayerStatus(EPawnStatusOperation, TArray<FStatusEffectInfo>&)` — 变更状态

**枚举:**
- `EPlayerStatusAttributeType` — Health, Magic, Stamin

#### UHUD_PawnStatusBarInScreen : UUserWidget, IScreenWidget_IF
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_PawnStatusBarInScreen.h`
**描述**: 屏幕空间Pawn状态栏，显示跟随角色的血量条。

**公开方法:**
- `void BindGlobalDelegatesEvent()` — 绑定委托
- `void SetProgressBarLimit(...)` / `UpdateProgressInfo(...)` / `ChangePlayerStatus(...)`
- `void SetOwningPawn(AActor*)` — IScreenWidget_IF 实现

#### UHUD_BossStatusBar : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_BossStatusBar.h`
**描述**: Boss状态栏。

**公开方法:**
- `void SetBossProgressBarLimit(EBossStatusAttributeType, float, float)` — 设置进度条
- `void UpdateBossProgressInfo(EBossStatusAttributeType, float)` — 更新进度
- `void ChangeBossStatus(EPawnStatusOperation, TArray<FStatusEffectInfo>&)` — 变更状态
- `void SetBossName(FString)` — 设置Boss名称

**枚举:**
- `EBossStatusAttributeType` — Health, Magic, Stamin

#### UHUD_Dialog : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_Dialog.h`
**描述**: 对话界面，支持智能分页。

**公开方法:**
- `void SetDialogText(const FString&, const FString&)` — 设置对话文本 (角色名+内容)
- `void ShowDialog()` / `CloseDialog()` — 显示/关闭

#### UHUD_EquipmentBar : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_EquipmentBar.h`
**描述**: 装备栏，显示上下左右四个道具/武器槽。

**公开方法:**
- `void InitializeEquipmentBar()` — 初始化
- `void UpdateTargetSlot(EHUDEquipmentSlotType, FStatusEffectInfo)` — 更新槽位
- `void StartChangeEquipment(UWidgetAnimation*)` — 切换装备动画

#### UHUD_InterActBtnPanel : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_InterActBtnPanel.h`
**描述**: 交互按钮面板，支持虚拟化滚动列表。

**公开方法:**
- `void UpdateBatch(const TArray<FInterActOptionInfo>&)` — 批量更新
- `void ClearAllOptions()` — 清空
- `void SetTargetOptionSelected(int32)` — 选中
- `void OnButtonClicked(int32)` — 按钮点击 (BlueprintNativeEvent)
- `void SetVisible(bool)` — 可见性
- `void InitializeVirtualization(int32)` — 初始化虚拟化

**附带结构体:**
- `FInterActOptionInfo` — 交互选项: Index, OptionIcon, OptionText

#### UHUD_Inventory : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_Inventory.h`
**描述**: 主背包界面。

**公开方法:**
- `void SetInventoryComponent(USL_InventoryComponent*)` — 设置库存组件
- `void RefreshInventory()` — 刷新显示
- `void ToggleInventory()` — 开关背包

#### UHUD_ItemIcon : UUI_DefaultSlot
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_ItemIcon.h`
**描述**: 装备栏道具图标槽。

- `void SetData(FStatusEffectInfo)` / `void ClearData()`

#### UHUD_Main_PC : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_Main_PC.h`
**描述**: PC主HUD容器。

#### UHUD_ProgressBar : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_ProgressBar.h`
**描述**: 通用进度条组件。

**公开方法:**
- `void SetProgressBarLimit(float, float)` — 设置范围
- `void UpdateProgressBar(float)` — 更新当前值
- `void GetProgressBarLimit(float&, float&)` — 获取范围

#### UHUD_StatusBar : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_StatusBar.h`
**描述**: 状态图标栏，管理Buff/Debuff图标。

**公开方法:**
- `void AddStatus(FStatusEffectInfo)` / `RemoveStatus(int)` / `UpdateStatus(FStatusEffectInfo)`
- `void OnStatusIconAdded(UUserWidget*)` / `OnStatusIconRemoved(...)` / `OnStatusIconUpdated(...)` — 蓝图动画事件

#### UHUD_StatusIcon : UUI_IconSlot
**文件**: `SoulLikeDemo/Public/UMG/HUD/HUD_StatusIcon.h`
**描述**: 状态图标控件。

### Pop 子模块

#### UPop_MainMenu : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/Pop/Pop_MainMenu.h`
**描述**: 主菜单弹窗。

**公开方法:**
- `void InitMainMenu()` — 初始化
- `void SetButtonInfos(const TArray<FMenuButtonInfo>&)` — 设置按钮
- `void ClearAllButtonInfos()` / `AddButtonInfo(...)` / `RemoveButtonInfo(...)` / `UpdateButtonInfo(...)`
- `void RefreshMenuToUI()` — 刷新UI

**附带结构体:**
- `FMenuButtonInfo` — 菜单按钮: ButtonText, ButtonImg, ButtonTag, linkWidgetIndex

#### UPop_NotifyMessage : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/Pop/Pop_NotifyMessage.h`
**描述**: 通知消息弹窗。

**公开方法:**
- `void FakeInit()` — 测试初始化
- `void SetNotificationText(const FText&)` — 设置文本
- `void StartFadeOut()` — 开始淡出
- `void MoveToPosition(float)` — 移动到位置

### SharedComponent 子模块

#### UUI_BaseCoin : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/SharedCompoent/UI_BaseCoin.h`
**描述**: 基础货币显示控件。

- `void InitializeUIComp()` / `SetImageBrush(UTexture2D*)` / `SetDynamicStr(FString)`

#### UUI_BaseSlot : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/SharedCompoent/UI_BaseSlot.h`
**描述**: 基础槽位控件。

- `void SetImageBrush(UTexture2D*)` / `void SetStacksNum(int)`

#### UUI_CoinShowBar : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/SharedCompoent/UI_CoinShowBar.h`
**描述**: 货币展示条。

#### UUI_DefaultSlot : UUI_BaseSlot
**文件**: `SoulLikeDemo/Public/UMG/SharedCompoent/UI_DefaultSlot.h`
**描述**: 默认槽位控件。

- `void SetNewAcquireImageVisible(bool)` — 新获取标记

#### UUI_IconSlot : UUI_BaseSlot
**文件**: `SoulLikeDemo/Public/UMG/SharedCompoent/UI_IconSlot.h`
**描述**: 图标槽位控件。

- `void SetData(FStatusEffectInfo)` — 设置状态数据

#### UUI_InterActButton : UUserWidget
**文件**: `SoulLikeDemo/Public/UMG/SharedCompoent/UI_InterActButton.h`
**描述**: 交互按钮控件。

- `void UpdateInterActBtnInfo(int32, UTexture2D*, FString)` — 更新按钮信息
- `void SetSelected(bool, bool = true)` — 选中状态
- `int32 GetAssignedIndex()` — 获取索引

#### UUI_InventorySlot : UUI_BaseSlot
**文件**: `SoulLikeDemo/Public/UMG/SharedCompoent/UI_InventorySlot.h`
**描述**: 背包格子控件。

- `void SetNewAcquireImageVisible(bool)` / `SetCenterTitle(FString)` / `FName GetButtonFlag()`

#### UUI_MenuItem : UUI_BaseSlot
**文件**: `SoulLikeDemo/Public/UMG/SharedCompoent/UI_MenuItem.h`
**描述**: 菜单项控件。

- `void SetNewAcquireImageVisible(bool)` / `SetCenterTitle(FString)` / `FName GetButtonFlag()`
