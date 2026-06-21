# 权威数据迁移计划
## — 从单机架构到 CS 架构的系统迁移

> 目标项目：SoulLike (UE 4.26)
> 关联系统：联机召唤系统 Phase 4 前置工作
> 文档版本：v1.1（基于实际代码修正）
> 创建日期：2026-06-15

---

## 目录

1. [路径判断](#1-路径判断)
2. [现状快照](#2-现状快照)
3. [迁移总纲](#3-迁移总纲)
4. [P0 — 数据复制](#4-p0--数据复制)
5. [P1 — 执行 Authority](#5-p1--执行-authority)
6. [P2 — RPC 路由审查](#6-p2--rpc-路由审查)
7. [P3 — UI 数据源改造](#7-p3--ui-数据源改造)
8. [迁移完成状态](#8-迁移完成状态)
9. [风险与回退](#9-风险与回退)

---

## 1 路径判断

### 1.1 你的判断是正确的

Phase 4 的战斗同步要求灵体与 B 世界的敌人、系统完整交互。这要求**所有对战斗有影响的数据**都具备以下 CS 属性：

```
数据在服务器上是本体 → Replication 复制到客户端
修改数据的操作只能在服务器执行 → HasAuthority() 守卫
客户端只能请求不能决定 → Server RPC → _Validate
UI 的绑定基于复制的数据源 → 不依赖 GetGameMode()
```

当前项目不是从零开始——GAS 层已经正确做了血量/体力的网络复制和服务器端伤害处理。但在 Inventory、事件广播、AI 决策等领域仍然是单机架构。

### 1.2 为什么排序是 P0 → P1 → P2 → P3

```
P0: 数据复制      →  不复制则客户端永远看不到正确数据
P1: Authority守卫  →  不守卫则客户端能改服务器数据
P2: RPC路由       →  不好路由则操作到不了服务器/回不到客户端
P3: UI数据源      →  数据源错了则客户端显示异常
```

每层**依赖**前一层。不能跳过。

### 1.3 推荐路径

```
Phase 3 ✅
    ↓
P0: InventoryComponent、EquipmentComponent、CombatComponent 状态复制
    ↓
P1: StaminaComponent Authority、AI Authority、伤害/死亡事件跨网络路由
    ↓
P2: 攻击连招 Server RPC、_Validate 补全
    ↓
P3: UI 数据链路确认 + Lua 侧绑定调整
    ↓
Phase 4 战斗同步与遣返
```

---

## 2 现状快照

### 2.1 已正确处理的模块

| 模块 | 现状 | 说明 |
|------|------|------|
| 血量/体力/最大血量/最大体力 | ✅ GAS AttributeSet 已 Replicated | `SL_StatusAttributeSet` 中 Health、MaxHealth、Stamina、MaxStamina 均已标记 `ReplicatedUsing` |
| GAS 伤害处理路径 | ✅ 服务器 Authority 路径正确 | `PostGameplayEffectExecute`（服务器）+ `OnRep`（客户端）两段式处理 |
| 召唤系统流程 | ✅ 完全 CS | Phase 1-3 已完成 |
| PlayerController RPC 框架 | ✅ 部分 Server RPC 就位 | `Server_RequestNewGame`、`Server_RequestLoadGame`、`Server_UseItem` |
| GameMode 联机支持 | ✅ 部分 CS | PreLogin/PostLogin 覆盖、PendingPhantoms 追踪 |

### 2.2 需要迁移的模块

| 模块 | 优先级 | 问题 |
|------|--------|------|
| `SL_InventoryComponent` | **P0 🔴** | `ItemInventory`(TMap) 无 `Replicated`；`SelectedItemID` 无 `Replicated` |
| `SL_EquipmentComponent` | **P0 🔴** | 装备数据无复制（具体需确认） |
| `SL_CombatantComponent` | **P0-P1 🟡** | `bWaitingForExecuted`、`bAllowedBackStabsed` 无复制；`TakeDamage` 已废弃（全注释） |
| `SL_StaminaComponent` | **P1 🟡** | `ConsumeStamina` 无 Authority 守卫；`CurrentRegenState`/`bWasOverdraw` 本地状态 |
| 伤害浮字事件广播 | **P1 🟡** | `GlobalDelegatesManager::BroadcastDamageFloatingText` 仅在服务器触发，不跨网络 |
| 死亡事件广播 | **P1 🟡** | `OnCharacterDied` 仅在服务器触发，需 Client RPC 通知 |
| AI Controller | **P1 🟡** | 需要确认 `RunBehaviorTree` 和 Blackboard 操作只在服务器执行 |
| Combo/Attack 输入路由 | **P2 🟢** | 需要确认客户端攻击输入走 Server RPC 还是本地直接调用 |
| Server RPC `_Validate` | **P2 🟢** | 补全所有缺省的 `_Validate` 实现 |
| UI 数据源 | **P3 🔵** | 审查 Lua Widget 中 `GetGameMode()` 和直接读非复制属性的情况 |

### 2.3 关键发现 — GlobalDelegatesManager 的网络断裂

这是最核心的架构问题。`GlobalDelegatesManager` 是一个 `UGameInstanceSubsystem`——服务端和客户端有不同的实例。

```
服务器端:
  PostGameplayEffectExecute
    → OnAttributeHealthChanged.Broadcast()  ✔  仅服务器端监听者收到
    → BroadcastDamageFloatingText()         ✘  客户端监听者收不到
    → OnCharacterDied.Broadcast()           ✘  客户端监听者收不到

客户端:
  OnRep_CurrentHealth
    → OnAttributeHealthChanged.Broadcast()  ✔  客户端监听者收到
  → 但 OnRep 中调用了同样的委托广播 → 这是正确的 ✅
```

**结论**：
- `OnAttributeHealthChanged` 被正确复制到客户端触发的 ✅（因为 `OnRep_CurrentHealth` 会在客户端执行）
- `BroadcastDamageFloatingText` 只在服务器触发，需要改为 RPC ✘
- `OnCharacterDied` 只在服务器触发，需要改为 RPC ✘

---

## 3 迁移总纲

```
P0 ─── 数据复制（Inventory / Equipment / CombatState 复制）
P1 ─── Authority（Stamina / AI / 跨网络事件路由）
P2 ─── RPC 路由（Combo 输入 / _Validate 补全）
P3 ─── UI 数据源（Lua Widget 审查）
```

---

## 4 P0 — 数据复制

### 4.1 P0-S1: SL_InventoryComponent 复制

**问题**：`ItemInventory` (TMap\<FName, int32\>) 和 `SelectedItemID` (FName) 均无 `Replicated` 标记。每个客户端拥有自己的库存副本，服务器修改不通知客户端。

**改动内容**：

```cpp
// SL_InventoryComponent.h
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Inventory")
TMap<FName, int32> ItemInventory;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Inventory")
FName SelectedItemID;
```

```cpp
// SL_InventoryComponent.cpp — 新增 GetLifetimeReplicatedProps
void USL_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USL_InventoryComponent, ItemInventory);
    DOREPLIFETIME(USL_InventoryComponent, SelectedItemID);
}
```

**涉及文件**：
- `Public\Component\Character\SL_InventoryComponent.h`
- `Private\Component\Character\SL_InventoryComponent.cpp`

**单元检验**：
1. ✅ 编译通过，无 `GetLifetimeReplicatedProps` 缺少注册警告
2. ✅ 服务器 `AddItemByID` → 客户端 `GetItemCount` 返回相同值
3. ✅ 服务器 `SetSelectedItemID` → 客户端 `GetSelectedItemID` 返回相同值
4. ✅ 客户端本地修改 `ItemInventory` 为脏数据 → 服务器数据不被影响

### 4.2 P0-S2: SL_EquipmentComponent 复制

**问题**：先确认 `SL_EquipmentComponent` 的具体属性和当前复制状态。

**待办**：
- 读取 `SL_EquipmentComponent.h` 确认所有 UPROPERTY 的 Replicated 状态
- 标记需要客户端可见的属性为 `Replicated`
- 注册 `GetLifetimeReplicatedProps`

**涉及文件**：
- `Public\Component\Character\SL_EquipmentComponent.h`
- `Private\Component\Character\SL_EquipmentComponent.cpp`

**单元检验**（待确定具体属性后细化）：
1. ✅ 服务器装备/卸下武器 → 客户端看到变化
2. ✅ 客户端看到的装备数据与服务器一致

### 4.3 P0-S3: SL_CombatantComponent 状态复制

**问题**：`bWaitingForExecuted` 和 `bAllowedBackStabsed` 这两个状态影响处决/背刺的 UI 显示，但没有 `Replicated`。

**改动内容**：

```cpp
// SL_CombatantComponent.h
UPROPERTY(Replicated)
int TeamID;

UPROPERTY(Replicated)
bool bWaitingForExecuted;

UPROPERTY(Replicated)
bool bAllowedBackStabsed;
```

**涉及文件**：
- `Public\Component\Character\SL_CombatantComponent.h`
- `Private\Component\Character\SL_CombatantComponent.cpp`

注意：`SL_CombatantComponent::TakeDamage` 已全部注释，此步骤只涉及状态复制，不涉及伤害逻辑。

**单元检验**：
1. ✅ 服务器标记敌人可处决 → 客户端看到处决提示
2. ✅ 服务器标记敌人可背刺 → 客户端看到背刺提示

### 4.4 P0-S4: 其他组件统一扫描

**问题**：可能存在其他遗漏的属性。

**执行**：
```powershell
grep -rn "UPROPERTY(.*Replicated" Source\SoulLikeDemo\Public\ --include="*.h"
```
对比所有有 `Replicated` 属性的类，确认 `GetLifetimeReplicatedProps` 是否覆盖。

**单元检验**：
1. ✅ 所有标记 `Replicated` 的属性都已注册
2. ✅ 没有任何有 `Replicated` 标记的类缺少 `GetLifetimeReplicatedProps`

### 4.5 P0 整体验证

- 双实例：服务器端修改库存、装备、战斗状态 → 客户端同步显示
- 编译器无 `DOREPLIFETIME` 遗漏警告

---

### 4.5 P0-S5: SL_EnemyBase 晚加入同步 — Replicate EnemyID + OnRep 重构

**状态**：✅ 已完成（2026-06-21）

**问题**：客户端A晚加入客户端B的世界时，世界B中已存在的怪物（通过 `UWaveManagerSystem::SpawnWaveMonsters` 在服务器生成）缺少模型资源、BehaviorTree、BlackboardData 等配置数据。这是因为 `ASL_EnemyBase` 的初始化（`InitializeEnemy` → `ApplyEnemyConfig`）只在服务器执行，配置数据没有被复制到晚加入的客户端。

**根因分析**：

| 成员 | 原因 |
|------|------|
| `BehaviorTree` / `BlackboardData` | `UPROPERTY()` 无 `Replicated`，客户端为 null |
| `EnemyConfig` | `UPROPERTY(EditDefaultsOnly)` 无 `Replicated`，客户端拿不到配置 |
| `LeftHandWeapon` / `RightHandWeapon` | `UPROPERTY()` 无 `Replicated`，客户端不知武器存在 |
| 骨骼网格体 / AnimBP | `SetSkeletalMesh` / `SetAnimInstanceClass` 不保证复制到晚加入客户端 |

**方案：Replicate 最小标识符 + OnRep 客户端自重建**

```
服务器: InitializeEnemy 时保存 NetEnemyID = EnemyID
        ↓
        DOREPLIFETIME(ASL_EnemyBase, NetEnemyID)
        ↓
客户端晚加入 → OnRep_EnemyID 触发
        ↓
        查本地 DataTableManager → EnemyConfigInfoTable
        ↓
        重新调用:
            LoadEnemyAppearance  (骨骼网格体 + AnimBP)
            SpawnEnemyWeapons    (已有武器指针时跳过 SpawnActor)
            设置胶囊体/缩放/GAS属性/移速/队伍
```

**改动文件**：

- `Public/Class/SL_EnemyBase.h` — 3处改动
  - 新增 `GetLifetimeReplicatedProps` 声明
  - 新增 `UPROPERTY(ReplicatedUsing=OnRep_EnemyID) int32 NetEnemyID` + `UFUNCTION() void OnRep_EnemyID()`
  - `LeftHandWeapon` / `RightHandWeapon` 改为 `UPROPERTY(Replicated)`

- `Private/Class/SL_EnemyBase.cpp` — 5处改动
  - 新增 `#include "Net/UnrealNetwork.h"`
  - `InitializeEnemy` 中保存 `NetEnemyID = EnemyID`
  - 实现 `GetLifetimeReplicatedProps` — 注册 `NetEnemyID`、`LeftHandWeapon`、`RightHandWeapon`
  - 实现 `OnRep_EnemyID` — 客户端从 DataTable 重建视觉配置
  - `SpawnEnemyWeapons` 增加武器已存在检查，防止客户端重复生成

**不涉及改动的文件**：

- `SL_EnemyAIController.cpp` — `OnPossess` 已有 BT/BB 空指针检查，`InitializeAI` 内部有 `HasAuthority()` 守卫，客户端直接 return，不需要运行行为树

**关键注意事项**：

- DataTable 必须在客户端可用（`UDataTableManager` + `EnemyConfigInfoTable`），否则 `OnRep_EnemyID` 静默失效
- GAS 属性通过属性复制同步，不依赖 `OnRep_EnemyID`，`BeginPlay` 中 `InitAbilityActorInfo` 已在客户端初始化 ASC
- OnRep（复制回调）不等同于 RPC：OnRep 是属性收到新值后的副作用回调（只发生在接收端）；RPC 是远程函数调用（有发送方和接收方）
- 武器 Actor 本身已有 `bReplicates = true`，标记 `LeftHandWeapon`/`RightHandWeapon` 为 `Replicated` 只是让基类能在客户端通过成员指针访问到已存在的武器 Actor

**此模式在 UE4 网络编程中的定位**：

| 同步场景 | 推荐方案 | 原因 |
|---------|---------|------|
| 晚加入时还原已存在 Actor 的状态 | `ReplicatedUsing` + OnRep | 客户端根据最小标识符自重建，复制流量最小 |
| 一次性事件通知（伤害飘字、音效） | `NetMulticast RPC` | 事件不需要持久化状态 |
| 持续变化的状态（血量、位置） | 属性复制（引擎自动） | 引擎有插值和差值优化 |
| 客户端操作请求（攻击、使用道具） | `Server RPC` | 客户端触发、服务器裁决 |

**单元检验**：

1. ✅ 服务器生成怪物后→客户端看到正确的骨骼网格体、动画蓝图、武器模型
2. ✅ 服务器怪物已被击杀→晚加入客户端不显示已死亡的怪物
3. ✅ 客户端无法通过 NetEnemyID 自行修改服务器数据（int32 只读）
4. ✅ OnRep_EnemyID 触发的 `LoadSynchronous()` 在主线程阻塞加载，怪物数量较多时需关注卡顿

---

## 5 P1 — 执行 Authority

### 5.1 P1-S1: SL_StaminaComponent Authority 守卫

**问题**：`ConsumeStamina` 使用 GAS GE 消耗体力。GE 在 GAS 中默认只在服务器端完全生效，但如果在客户端调用 `ApplyGameplayEffectToSelf`，GE 会只在客户端本地执行——客户端体力值被修改（本地副本），服务器不知道。之后服务器复制回来的体力值会覆盖客户端修改。

**改动**：

```cpp
void USL_StaminaComponent::ConsumeStamina(float InAmount)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }
    // 原有逻辑：创建 CostGE 并应用到 ASC
}
```

**涉及文件**：
- `Private\Component\Character\SL_StaminaComponent.cpp`

**单元检验**：
1. ✅ 客户端调用 `ConsumeStamina` → 被静默忽略，体力不变
2. ✅ 服务器调用 → 正常消耗，`OnRep_CurrentStamina` 触发客户端更新

### 5.2 P1-S2: AI Controller Authority 确认

**问题**：当前 `SL_EnemyAIController` 需要确认没有在客户端执行 AI 逻辑。

**改动**：检查 `SL_EnemyAIController::BeginPlay()`、`Possess()` 及 `Tick()` 中是否调用了 `RunBehaviorTree` 或修改 Blackboard 值，确保这些只在 Authority 端执行。

```cpp
void ASL_EnemyAIController::BeginPlay()
{
    Super::BeginPlay();
    if (!HasAuthority())
    {
        // 客户端不运行行为树
        return;
    }
    RunBehaviorTree(BehaviorTreeAsset);
}
```

**涉及文件**：
- `Private\AI\SL_EnemyAIController.cpp`

**单元检验**：
1. ✅ 双实例：AI 行为树只在服务器运行，客户端只接收复制的 AI 位置/状态
2. ✅ 客户端 AI 不消耗性能（行为树 tick 不执行）

### 5.3 P1-S3: 伤害浮字事件跨网络路由

**问题**：`SL_StatusAttributeSet::PostGameplayEffectExecute`（服务器端）中调用了 `GlobalDelegatesManager::BroadcastDamageFloatingText`。但 `GlobalDelegatesManager` 是 `UGameInstanceSubsystem`，服务器和客户端是不同的实例。**服务器的广播不会到达客户端**。

**改动方案**：在服务器端收到伤害事件后，通过 `Client` RPC 或 `Multicast` RPC 通知客户端触发伤害浮字。

**方案 A（推荐）**——通过 Owner 的 PlayerController 发送 Client RPC：

```cpp
// SL_CharacterBase 或 SL_PlayerControllerBase 新增
UFUNCTION(Client, Reliable)
void Client_OnDamageFloatingText(const FDamageFloatingTextData& InData);

void ASL_PlayerControllerBase::Client_OnDamageFloatingText_Implementation(
    const FDamageFloatingTextData& InData)
{
    // 客户端本地触发 GlobalDelegatesManager 广播
    if (auto* DelegateMgr = UGlobalDelegatesManager::Get(this))
    {
        DelegateMgr->OnDamageFloatingText.Broadcast(InData);
    }
}
```

**涉及文件**：
- `Public\Class\SL_PlayerControllerBase.h`
- `Private\Class\SL_PlayerControllerBase.cpp`

**方案 B**——通过 Multicast RPC（所有客户端包括服务器都触发）：

```cpp
// SL_CharacterBase
UFUNCTION(NetMulticast, Reliable)
void Multicast_OnDamageFloatingText(const FDamageFloatingTextData& InData);
```

**单元检验**：
1. ✅ 双实例：服务器端造成伤害 → 客户端看到伤害飘字
2. ✅ 多个客户端都看到正确的飘字数值和位置

### 5.4 P1-S4: 死亡事件跨网络路由

**问题**：同伤害浮字——`OnCharacterDied` 只在服务器端通过 `GlobalDelegatesManager` 广播。

**改动方案**：

```cpp
// SL_PlayerControllerBase 新增
UFUNCTION(Client, Reliable)
void Client_OnCharacterDied(AActor* DeadActor, AActor* Instigator);

void ASL_PlayerControllerBase::Client_OnCharacterDied_Implementation(
    AActor* DeadActor, AActor* Instigator)
{
    if (auto* DelegateMgr = UGlobalDelegatesManager::Get(this))
    {
        DelegateMgr->OnCharacterDied.Broadcast(DeadActor, Instigator);
    }
}
```

在 `GlobalDelegatesManager` 中注册死亡事件的时机：
- 服务器 `PostGameplayEffectExecute` 触发死亡时 → 找到受害者的 Owner PlayerController → 调用 `Client_OnCharacterDied`

**涉及文件**：
- `Public\Class\SL_PlayerControllerBase.h`
- `Private\Class\SL_PlayerControllerBase.cpp`

**单元检验**：
1. ✅ 双实例：服务器判定角色死亡 → 客户端收到死亡事件 → 显示死亡 UI / 播放死亡动画
2. ✅ 仅在服务器上发生一次广播，不会被多次触发

### 5.5 P1 整体验证

- 双实例：客户端调用资源消耗操作（如体力消耗）被服务器忽略
- 双实例：服务器确认伤害后，客户端看到伤害浮字和死亡效果
- 双实例：AI 只在服务器决策

---

## 6 P2 — RPC 路由审查

### 6.1 P2-S1: Combo/Attack 输入路由审查

**问题**：确认攻击输入（轻击、重击、特殊攻击、Combo 判定）是否通过 Server RPC 发送到服务器。

**审查方法**：

1. 读取 `SL_PlayerControllerBase::OnLightAttackPressed()` 的实现
2. 读取 `SL_PlayerControllerBase::OnHeavyAttackPressed()` 的实现
3. 读取 `ProcessComboInput()` 的实现

**典型正确模式**：

```cpp
void ASL_PlayerControllerBase::OnLightAttackPressed()
{
    if (HasAuthority())
    {
        // 服务器直接执行
        HandleAttackOnServer(EAttackType::Normal_Combo_Phase_1);
    }
    else
    {
        // 客户端发 Server RPC
        Server_HandleAttack(EAttackType::Normal_Combo_Phase_1);
    }
}
```

**涉及文件**：
- `Private\Class\SL_PlayerControllerBase.cpp`

**单元检验**：
1. ✅ 双实例：客户端按下攻击键 → 服务器收到 Server RPC
2. ✅ 服务器执行攻击逻辑后，所有客户端看到正确的攻击动画和命中效果
3. ✅ 服务器拒绝非法的攻击请求

### 6.2 P2-S2: 所有 Server RPC _Validate 补全

**问题**：所有带 `WithValidation` 的 Server RPC 必须有配套的 `_Validate` 实现，否则编译警告或运行时断线。

**扫描命令**：

```powershell
grep -rn "WithValidation" Source\SoulLikeDemo\ --include="*.h"
```

**典型修复**：

```cpp
// 已有的声明
UFUNCTION(Server, Reliable, WithValidation)
void Server_UseItem(FName InItemID);

// 缺 _Validate 实现
bool ASL_PlayerControllerBase::Server_UseItem_Validate(FName InItemID)
{
    // 检查道具ID的合法性
    return !InItemID.IsNone();
}
```

**涉及文件**：扫描结果中所有缺少 `_Validate` 实现的文件

**单元检验**：
1. ✅ 编译无任何 `_Validate` 缺失警告
2. ✅ 每个 `_Validate` 有实际的参数校验逻辑（不仅仅是 `return true`）

### 6.3 P2 整体验证

- 双实例：所有输入端到端路由正确
- `_Validate` 覆盖所有声明

---

## 7 P3 — UI 数据源改造

### 7.1 P3-S1: Lua Widget 数据源审查

**审查范围**：
```
Content\Script\UI\HUD\*.lua
Content\Script\UI\Pop\*.lua
Content\Script\UI\*\*.lua
```

**审查内容**：
- 是否有 `GetGameMode()` 或 `GetAuthGameMode()` 调用
- 是否有对非 Replicated 属性的直接读取
- 触发 UI 更新的事件是否绑定到 `GlobalDelegatesManager` 的 `OnRep` 触发的委托

**典型问题模式**：

```lua
-- 存在问题：客户端可能拿不到 GameMode
local GM = self:GetWorld():GetAuthGameMode()
if GM then
    -- ...
end

-- 正确：从 PlayerState 或 Replicated 属性读取
local PC = self:GetOwningPlayer()
local PS = PC:GetPlayerState(ClassName)
```

**单元检验**：
1. ✅ 所有 Lua Widget 不直接依赖 GameMode
2. ✅ Lua UI 读取的数据来自 Replicated 属性

### 7.2 P3-S2: Lua 侧血条/体力条绑定确认

**审查内容**：确认血条和体力条如何获取数据。

当前 GAS 层已经通过 `OnRep_CurrentHealth` → `GlobalDelegatesManager::OnAttributeHealthChanged.Broadcast` 在客户端触发了健康值变化事件。如果在 Lua 中绑定了这个委托，血条应该已经能正确同步。

**单元检验**：
1. ✅ 双实例：客户端血条与服务器同步
2. ✅ 体力条变化正确响应

### 7.3 P3 整体验证

- 双实例模式下，所有 UI 模块不因为网络问题报错
- 客户端 UI 数据正确反应服务器状态

---

## 8 迁移完成状态

### 8.1 完成标志

```
P0 — Inventory、Equipment、CombatState 均已 Replicated
P1 — Stamina Authority、AI Authority、伤害/死亡事件路由全部就位
P2 — Combo 输入 RPC 路由审查通过，_Validate 全部补全
P3 — Lua Widget 数据源审查通过
```

### 8.2 迁移后的架构

```
服务器端（Authority）                              客户端（Proxy）
  GAS ASC (本体)            ---Attribute Replication-->  GAS ASC (镜像)
    Health                                                Health (副本)
    Stamina                                               Stamina (副本)
  Inventory (本体)          ---DOREPLIFETIME---------->  Inventory (副本)
  Equipment (本体)          ---DOREPLIFETIME---------->  Equipment (副本)
  CombatState (本体)        ---DOREPLIFETIME---------->  CombatState (副本)

  伤害判定 (GE)                                          OnRep 触发 UI
    死亡事件触发            ---Client RPC------------->  死亡 UI / 动画
    伤害浮字触发            ---Client/Multicast RPC--->  飘字生成

  AI 决策 (BT)                                           显示 AI 状态
                            <---Server RPC------------  Combo 输入
                            <---Server RPC------------  道具使用请求
```

---

## 9 风险与回退

### 9.1 风险

| 风险 | 影响 | 缓解 |
|------|------|------|
| Inventory 大量复制膨胀 | 增加网络带宽 | 考虑按需同步（`DOREPLIFETIME_CONDITION`）、增量更新 |
| RPC 过多造成队列阻塞 | 操作延迟 | 攻击输入用 Unreliable RPC，事件通知用 Reliable |
| AI Authority 误判 | 两边都跑 AI | 加 `HasAuthority()` 后做双实例测试 |
| Legacy 组件仍有调用链 | 新方案不覆盖旧逻辑 | 全局搜索 Legacy 组件的引用 |

### 9.2 回退策略

- 每个步骤增量提交，Git tag 可回退
- `Replicated` 标记添加后可去掉
- `HasAuthority()` 守卫只加代码不删逻辑，回退只需删除判断行

---

## 10 附录：UE4 网络复制机制原理

### 10.1 组件复制：运行时动态创建的组件如何同步

构造函数中 `CreateDefaultSubobject` 创建的组件，服务器和客户端各有一个同名实例，天然存在。不需要额外处理。

**运行时动态创建的组件**需要手动开启复制：

```cpp
// 服务器端：动态创建组件
UMyComponent* MyComp = NewObject<UMyComponent>(this, TEXT("RuntimeComp_Weapon"));
MyComp->bReplicates = true;      // 必须开复制
MyComp->RegisterComponent();     // 注册到世界，否则引擎不知道它的存在
```

引擎的 `ActorChannel` 会在下一个复制周期检测到新组件，自动在客户端创建对应的实例：

```
服务器:
  NewObject<UMyComp> + RegisterComponent()
    ↓
  ActorChannel 发现新组件
    ↓
  创建 FObjectReplicator 管理该组件的属性复制
    ↓
  将组件的创建指令 + 初始属性编码到网络包
    ↓
客户端:
  收到网络包 → 在 Actor 的组件列表中找到/创建同名组件
    ↓
  为该组件创建 FObjectReplicator
    ↓
  后续属性变更自动复制（与默认组件相同机制）
```

**约束条件**：

- `bReplicates = true` 和 `RegisterComponent()` **缺一不可**：前者告诉引擎要复制，后者让引擎知道组件存在
- 组件名称（`FName`）必须稳定：引擎靠组件名来匹配服务器和客户端的实例。`CreateDefaultSubobject` 时显式指定名称；动态创建时传一个明确的 `FName` 参数（如 `TEXT("RuntimeComp_Weapon")`），不要用 `NAME_None`
- **不要复制组件指针**：`UPROPERTY(Replicated) UMyComponent* Comp` 是错误做法。服务器上的指针地址指向的是服务器进程的地址空间，客户端拿这个地址毫无意义。组件实例的匹配靠的是**路径名**，不是**内存地址**

---

### 10.2 属性同步机制：不同进程地址不同的对象，数据怎么同步的？

UE4 的网络复制不依赖内存地址。它的工作逻辑分三步：

#### 第一步：对象识别 —— 用路径名，不用地址

```
服务器端对象路径：
  World[/Game/Map].PlayerCharacter_0.InventoryComponent

客户端对象路径（完全一致）：
  World[/Game/Map].PlayerCharacter_0.InventoryComponent
```

每个 Actor 和 Component 有一个全局唯一的路径名，由 UWorld 名 + Actor 名 + 组件树层次拼接而成。引擎通过网络包中的 `FNetworkGUID` 来传递这个对象引用，而不是用指针地址。

#### 第二步：属性定位 —— 用 RepLayout（类元数据），不用硬编码偏移

每个 UClass 在加载时生成一个 `FRepLayout`，这是一个描述类中所有 `Replicated` 属性的结构：

```
类 UInventoryComponent 的 RepLayout:
  Gold:    int32,         offset=0x120
  Items:   TArray<FItem>, offset=0x130
  SelectedItemID: FName,  offset=0x148
```

关键的属性：offset 是类编译期确定的，同一个类在所有进程中布局完全一致。引擎不直接写死偏移值，而是通过 `UPROPERTY()` 反射系统在运行时查询属性的 offset。

#### 第三步：值传输 —— 序列化，不是内存拷贝

```
服务器                               客户端
  │                                   
  ├── 读取 ServerComp->Gold = 500     
  ├── 编码为：属性名 + 序列化值        
  │   "Gold" → int32(500)             
  │   "Items[0]" → {ID=3, Count=2}    
  │────────────────────────────────►  
  │                                    ├── 用路径名找到 ClientComp
  │                                    ├── 查 RepLayout 找到 Gold 的 offset
  │                                    ├── memcpy(ClientComp + offset, 500)
  │                                    └── 触发 OnRep_Gold()
```

服务器不发送"地址"，只发送"属性名 + 值"。客户端根据 RepLayout 反查该属性在本地对象中的存储位置，然后写入。

**核心结论**：

```
对象 → 用路径名 + FNetworkGUID 匹配（不是地址）
属性 → 用 RepLayout 按名定位（不是固定偏移）
值   → 序列化传输（不是内存拷贝）
```

这就是为什么服务器 `@0x1000` 和客户端 `@0x7000` 的两个不同对象能正确同步数据——它们只是同一份数据的两个副本，引擎通过名称和布局来保持一致，不需要关心地址。

#### GAS 的特殊之处

GAS 不依赖标准的 `DOREPLIFETIME` 机制，它用自定义的 `FGameplayAttributeData::NetSerialize` 做批量属性同步：

```cpp
struct FGameplayAttributeData {
    float BaseValue;
    float CurrentValue;
    
    bool NetSerialize(FArchive& Ar, ...) {
        Ar << CurrentValue;  // 自定义序列化
        return true;
    }
};
```

`UAbilitySystemComponent` 用 `FMinimalReplicationTagCountMap` 跟踪哪些属性发生了变动，然后在 `OnRep_MinimalReplicationInfo` 中一次性批量更新。这比每个属性一个 `DOREPLIFETIME` 更高效，也是 GAS 层不需要开发者手动注册复制属性的原因。

---

### 10.3 组件数据同步的两种方案对比

| | 方案A：组件属性标记 Replicated | 方案B：Actor 上放 ID + OnRep 自初始化 |
|--|-------------------------------|--------------------------------------|
| 数据来源 | 服务器逻辑运算 | DataTable / 资产表配置 |
| 运行时是否变化 | 持续变化（血量、位置、库存数量） | 固定不变（骨骼网格体、行为树资产） |
| 复制内容 | 每条属性独立编码传输 | 仅传输一个 int32/FName |
| 复制开销 | 每条变化单独编码（带宽随数据量线性增长） | 极小（4字节的 EnemyID） |
| 客户端恢复方式 | 引擎自动写入目标属性 | 查 DataTable → 按结果自行重建 |
| 典型场景 | 血量/体力（GAS Attribute）、库存数量、战斗状态开关 | 怪物配置（EnemyBase）、玩家初始装备、关卡静态数据 |

**关键原则**：固定的配置走 ID + OnRep 自初始化，动态变化的值走组件属性直接 `Replicated`。两者可以混用——同一个组件可以同时有 `Replicated` 的实时变量和 OnRep 后查表加载的固定配置。

---

> 文档版本：v1.1（基于实际代码修正）
> 最后更新：2026-06-15
> 状态：已就绪