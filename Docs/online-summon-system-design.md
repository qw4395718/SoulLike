# 联机召唤系统技术架构设计

> 设计目标：为 SoulLike 项目实现类似《黑暗之魂》风格的召唤符联机系统
> 当前阶段：架构设计（具体实现前的蓝图）
> 关联系统：道具系统、GAS、世界管理

---

## 目录

1. [系统概述](#1-系统概述)
2. [网络拓扑](#2-网络拓扑)
3. [核心概念](#3-核心概念)
4. [组件架构](#4-组件架构)
5. [核心流程](#5-核心流程)
6. [RPC 接口设计](#6-rpc-接口设计)
7. [复制策略](#7-复制策略)
8. [数据存储与匹配](#8-数据存储与匹配)
9. [状态机](#9-状态机)
10. [异常与边界](#10-异常与边界)
11. [分阶段实施建议](#11-分阶段实施建议)

---

## 1 系统概述

### 1.1 玩法描述

- 玩家 A 使用「召唤符」道具，在当前位置放置一个召唤标记
- 该召唤标记出现在其他符合条件的玩家世界中
- 玩家 B 与标记交互，发起召唤请求
- 玩家 A 收到请求并确认后，从自身世界传送至玩家 B 的世界
- 玩家 A 以「灵体」身份出现在 B 的世界中，协助战斗
- 目标达成（首领击败/区域清除）或灵体死亡后，A 返回自身世界

### 1.2 核心约束

| 约束 | 说明 |
|------|------|
| 跨世界可见性 | A 放置的标记必须能被 B 看到，但 A 和 B 不在同一个游戏实例 |
| 角色迁移 | A 的角色需要从自己的世界「移动」到 B 的世界实例中 |
| 数据隔离 | A 带入 B 世界的数据仅限于角色属性/装备/状态，不携带 B 世界的进度 |
| 战斗同步 | 灵体与 B 世界中的敌人交互需要完整的伤害/位置同步 |
| 公平性 | 灵体属性可能需要缩放（防止碾压或过于酱油） |
| 安全退出 | 网络断线/崩溃时，双方能安全回到各自世界 |

---

## 2 网络拓扑

### 2.1 逻辑架构

```
                          ┌──────────────────────────┐
                          │     Matchmaking / Lobby   │
                          │   (OnlineSubsystem / 后端) │
                          └──────────┬───────────────┘
                                     │ 会话匹配/召唤符注册
                    ┌────────────────┼───────────────┐
                    │                │                │
          ┌─────────▼────────┐ ┌────▼─────┐ ┌───────▼────────┐
          │  GameInstance A   │ │  Backend  │ │  GameInstance B │
          │  (Player A 世界)  │ │  (可选)   │ │  (Player B 世界)│
          │                   │ │           │ │                 │
          │  Server A         │ │ 状态服务  │ │  Server B        │
          │   ┌─────────┐    │ │           │ │   ┌─────────┐   │
          │   │SignActor│    │ │ 标记列表  │ │   │SignActor│   │
          │   └─────────┘    │ │ 匹配队列  │ │   └─────────┘   │
          └──────────────────┘ └───────────┘ └─────────────────┘
```

### 2.2 拓扑选型

**推荐方案：集中式匹配 + 对等世界实例**

| 组件 | 模式 | 职责 |
|------|------|------|
| **匹配服务** | GameInstanceSubsystem 或独立后端 | 维护当前所有活跃召唤标记的列表，处理标记的注册/查询/移除 |
| **世界实例** | 每个世界的独立 DedicatedServer | 每个玩家（或每组玩家）运行自己的世界实例，负责该世界内的全部游戏逻辑 |
| **客户端** | 对应各自世界实例 | 玩家操作、渲染、输入采集 |

### 2.3 备选方案对比

| 方案 | 优势 | 劣势 | 适用场景 |
|------|------|------|---------|
| **A: 单服多世界分区** | 架构简单，数据共享方便 | 单服压力大，扩展性受限 | 原型验证阶段 |
| **B: 多服 + 中心匹配** | 扩展性好，隔离性强 | 架构复杂，需要跨服通信 | 正式运营阶段 |
| **C: P2P 直连 + 信令** | 无需服务器成本 | NAT 穿透复杂，防作弊困难 | 小型验证项目 |

**建议：从方案 A 开始，预留方案 B 的扩展接口。**

---

## 3 核心概念

### 3.1 数据结构

```cpp
// ===== 召唤标记数据 =====
USTRUCT(BlueprintType)
struct FSummonSignInfo
{
    GENERATED_BODY()

    // 标记唯一ID
    UPROPERTY()
    FGuid SignID;

    // 放置者信息
    UPROPERTY()
    FUniqueNetIdRepl OwnerPlayerID;      // 放置者的平台唯一ID
    UPROPERTY()
    FString OwnerPlayerName;
    UPROPERTY()
    int32 OwnerLevel;                    // 放置者等级（用于匹配范围）
    UPROPERTY()
    int32 OwnerWeaponLevel;              // 放置者当前的武器强化等级

    // 位置信息（放置者世界中的位置）
    UPROPERTY()
    FName CurrentLevelName;              // 放置者所在的关卡名
    UPROPERTY()
    FTransform SignTransform;            // 标记在世界中的变换

    // 时间信息
    UPROPERTY()
    float TimeRemaining;                 // 剩余存在时间

    // 状态
    UPROPERTY()
    ESummonSignState State;
};

// ===== 召唤状态枚举 =====
UENUM(BlueprintType)
enum class ESummonSignState : uint8
{
    Active,             // 标记可交互
    BeingSummoned,      // 正在被召唤（锁定中）
    Expired,            // 已过期
    Removed             // 已手动移除
};

// ===== 玩家联机状态 =====
UENUM(BlueprintType)
enum class EOnlinePlayerState : uint8
{
    Solo,               // 单人模式
    PlacingSign,        // 正在放置标记
    SignActive,         // 标记已放置，等待被召
    BeingSummoned,      // 正在被召唤到别的世界
    SummonedAsPhantom,  // 已作为灵体在别的世界中
    SummoningOther,     // 正在召唤别的玩家到自己的世界
    HasPhantom,         // 自己的世界中有其他玩家
};
```

### 3.2 关键 Actor 类型

```cpp
// ===== 召唤标记 Actor =====
ASL_SummonSign : public AActor
{
    UPROPERTY(Replicated)
    FSummonSignInfo SignInfo;

    UPROPERTY(Replicated)
    bool bIsBeingSummoned;       // 防止多人同时交互

    UPrimitiveComponent* InteractCollision;
};

// ===== 灵体角色 =====
ASL_PhantomCharacter : public ACharacter
{
    UPROPERTY(ReplicatedUsing = OnRep_PhantomData)
    FPhantomData PhantomData;     // 外观、装备、属性等

    UAbilitySystemComponent* PhantomASC;
    bool bCanInteractWithWorld;
    bool bCanBeDamaged;
};
```

---

## 4 组件架构

### 4.1 模块划分

```
┌─────────────────────────────────────────────────────────┐
│                  联机召唤系统                              │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  ┌──────────────────┐   ┌────────────────────────┐      │
│  │  SignManager      │   │  SummonSessionComponent │      │
│  │  (GameInstance)   │   │  (挂在PlayerController) │      │
│  │                   │   │                        │      │
│  │  • 注册/注销标记   │   │  • 发起放置标记         │      │
│  │  • 查询可用标记    │   │  • 发起召唤请求         │      │
│  │  • 标记过期清理    │   │  • 响应召唤邀请         │      │
│  │  • 匹配范围校验    │   │  • 世界穿梭状态管理     │      │
│  └──────────────────┘   └────────────────────────┘      │
│                                                          │
│  ┌──────────────────┐   ┌────────────────────────┐      │
│  │  SummonSignActor   │   │  PhantomController     │      │
│  │  (World Actor)    │   │  (Remote世界控制器)    │      │
│  │                   │   │                        │      │
│  │  • 标记可视化      │   │  • 灵体操作映射        │      │
│  │  • 交互触发        │   │  • 输入转发            │      │
│  │  • 生命周期        │   │  • 缩放属性管理        │      │
│  └──────────────────┘   └────────────────────────┘      │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

### 4.2 SignManager（核心管理类）

```
位置: GameInstanceSubsystem
职责:
  - 维护全局标记列表 TArray<FSummonSignInfo>
  - 处理标记的注册（ServerOnly）
  - 处理标记的查询（按关卡/等级范围过滤）
  - 处理标记的移除（超时/手动取消/已被召唤）

对外接口:
  RegisterSign(OwnerInfo, Transform) → FGuid
  UnregisterSign(FGuid)
  QuerySigns(QueryFilter) → TArray<FSummonSignInfo>
  ReportSignInteraction(FGuid, InteractorID) → bool
```

### 4.3 SummonSessionComponent

```
位置: PlayerController 上的 ActorComponent
职责:
  - 管理当前玩家的联机状态机
  - 处理放置标记的道具使用
  - 管理"正在被召唤"的流程
  - 处理世界切换前后的数据保存/恢复

状态:
  EOnlinePlayerState CurrentState

关键方法:
  // 放置者侧
  Server_PlaceSummonSign()
  Client_OnSummonRequest(SignID, SummonerInfo)
  Server_AcceptSummon(SignID)
  Server_CancelSign()

  // 召唤者侧
  Server_InteractWithSign(SignActor)
  Client_OnPhantomArrived(PhantomInfo)
```

### 4.4 世界穿梭流程

```
放置者 (Player A)                        召唤者 (Player B)
─────────────────                        ─────────────────
  1. 使用召唤符道具
  2. → 调 Server_PlaceSummonSign()
  3. 服务器生成 ASL_SummonSign
  4. → SignManager.RegisterSign()
                                              5. B 看到标记
                                              6. B 交互标记
                                              7. → Server_InteractWithSign()
  8. ← 收到召唤请求
  9. A 确认召唤
  10. → Server_AcceptSummon()
  11. 保存当前世界状态
  12. 服务器切换 A 到 B 的世界
  13. 在 B 的世界中生成灵体
  14. → 同步 PhantomData
                                              15. 灵体现在 B 的世界中可见
                                              16. B 和 A 的灵体协作战斗
```

---

## 5 核心流程

### 5.1 放置召唤标记

```
 [Player A]          [Server A]            [SignManager]
    │                     │                     │
    │ 使用召唤符道具       │                     │
    │────────────────────>│                     │
    │                验证条件:                    │
    │                • 当前区域允许放置          │
    │                • 不是BOSS战状态           │
    │                • 没有已存在的标记          │
    │                • 道具消耗成功             │
    │                     │                     │
    │                SpawnActor(SummonSign)      │
    │                     │                     │
    │                     │── RegisterSign() ──>│
    │                     │                     │
    │                     │<── SignID ──────────│
    │                     │                     │
    │<── 放置成功确认 ────│                     │
    │                     │                     │
```

### 5.2 召唤流程

```
 [Player B]        [Server B]      [SignManager]     [Server A]      [Player A]
    │                   │                │                │              │
    │ 交互召唤标记       │                │                │              │
    │──────────────────>│                │                │              │
    │              校验:                 │                │              │
    │              • 标记状态=Active     │                │              │
    │              • B 等级允许范围内    │                │              │
    │              • B 当前可召唤        │                │              │
    │                   │                │                │              │
    │              LockSign()           │                │              │
    │                   │── MarkUsed ──>│                │              │
    │                   │                │── NotifyA ──>│              │
    │                   │                │                │── 确认弹窗 │
    │                   │                │                │<── 确认 ── │
    │                   │                │<─ Accept ──── │              │
    │                   │                │                │ 保存状态     │
    │                   │                │                │ 切换到B世界  │
    │                   │<─ SpawnPhantom│                │              │
    │ 生成灵体角色       │                │                │              │
    │<── PhantomSpawned │                │                │              │
    │                   │                │                │              │
```

### 5.3 灵体返回/遣返

```
触发条件:
  1. 灵体死亡
  2. 召唤者死亡
  3. BOSS 被击杀
  4. 灵体主动返回
  5. 网络断线

流程:
  1. 服务器检测到遣返条件
  2. 通知灵体客户端播放返回特效
  3. 销毁灵体角色
  4. 放置者恢复到自己世界
  5. 从 SignManager 移除标记
  6. 恢复到最后一个安全位置
```

---

## 6 RPC 接口设计

### 6.1 接口清单

```
// ===== SignManager 接口 (Server Only) =====
Server_RegisterSign(Owner, Transform)
Server_UnregisterSign(SignID)
Server_RequestSummon(SignID, Requester)        [WithValidation]
Server_AcceptSummon(SignID)                    [WithValidation]
Server_DeclineSummon(SignID)

// ===== 放置者侧回调 =====
Client_OnSummonRequested(SignID, SummonerInfo)
Client_OnSummonAccepted(SignID)
Client_OnReturnToOwnWorld(ReturnReason)

// ===== 召唤者侧回调 =====
Client_OnPhantomSpawned(PhantomData)
Client_OnPhantomDeparted()

// ===== 灵体同步 =====
Server_PhantomMove(Movement)                   [Unreliable]
Multicast_PhantomAttack(AbilityHandle)         [Reliable]
```

### 6.2 可靠性分类

| 类别 | RPC | 可靠性 |
|------|-----|--------|
| 流程控制 | 注册/召唤/确认/遣返 | Reliable |
| 状态同步 | 灵体位置/动画 | Unreliable |
| 战斗事件 | 伤害/技能触发 | Reliable |
| 加载同步 | 世界切换完成通知 | Reliable |

---

## 7 复制策略

### 7.1 复制范围

| Actor 类型 | 复制范围 |
|-----------|---------|
| `ASL_SummonSign` | 仅放置者世界，通过 SignManager 暴露给其他世界 |
| `ASL_PhantomCharacter` | 仅召唤者世界 |
| `ASL_PlayerState` | 不跨世界，穿梭时打包传输 |

### 7.2 属性缩放

```
缩放后伤害 = 原始伤害 × (召唤者等级 / 放置者等级) × 缩放曲线
缩放后血量 = 原始血量 × 世界人数缩放系数
```

---

## 8 数据存储与匹配

### 8.1 匹配条件

| 条件 | 说明 |
|------|------|
| 同关卡 | 放置者和召唤者必须在同一关卡 |
| 等级范围 | 放置者等级在召唤者 ±10 级 |
| 武器等级范围 | 最高武器强化等级在 ±2 级 |
| 区域状态 | 召唤者区域尚未击败 Boss |
| 玩家状态 | 双方均处于可联机状态 |

### 8.2 匹配配置

```cpp
USTRUCT(BlueprintType)
struct FSummonMatchConfig
{
    UPROPERTY(EditAnywhere)
    int32 LevelRange = 10;

    UPROPERTY(EditAnywhere)
    int32 WeaponLevelRange = 2;

    UPROPERTY(EditAnywhere)
    bool bEnablePassword = true;

    UPROPERTY(EditAnywhere)
    FString SummonPassword;
};
```

---

## 9 状态机

### 9.1 放置者

```
     ┌──────────────────────────────────────────────┐
     │                  Solo                         │
     └────┬────────────┬──────────────┬──────────────┘
          │            │              │
          │ 使用召唤符  │  被召唤请求  │  从灵体返回
          ▼            ▼              ▼
   ┌────────────┐ ┌──────────┐ ┌──────────────┐
   │PlacingSign │ │Being     │ │  返回过渡     │
   └─────┬──────┘ │Summoned  │ └──────┬───────┘
         │ 完成    └────┬─────┘        │ 完成
         ▼              │ 确认         ▼
   ┌────────────┐ ┌────▼─────┐   ┌────────┐
   │SignActive  │ │Summoned  │   │  Solo  │
   │(等待被召唤) │ │AsPhantom │   └────────┘
   └─────┬──────┘ └──────────┘
         │ 被召唤
         └──────────────┘
```

### 9.2 召唤者

```
     ┌────────────────────────────────────────────┐
     │                  Solo                       │
     └────┬───────────────────┬───────────────────┘
          │                   │
          │ 交互标记           │  灵体返回/死亡
          ▼                   ▼
   ┌──────────────┐   ┌──────────────┐
   │SummoningOther│   │  Departed    │
   └──────┬───────┘   └──────┬───────┘
          │ 成功              │ 完成
          ▼                   ▼
   ┌──────────────┐   ┌──────────────┐
   │  HasPhantom  │──→│    Solo      │
   └──────────────┘   └──────────────┘
```

---

## 10 异常与边界

### 10.1 异常场景

| 场景 | 处理方式 |
|------|---------|
| 放置者断线 | 自动移除标记，灵体状态则遣返 |
| 召唤者断线 | 灵体遣返，标记移除 |
| 同时交互同一标记 | `bIsBeingSummoned` 锁 + 服务器校验 |
| Boss 被击杀 | 遣返所有灵体，清除区域标记 |
| 世界切换加载失败 | 回退自己世界 |
| 属性变化 | 以穿梭瞬间快照为准 |

### 10.2 边界条件

| 边界 | 策略 |
|------|------|
| 同时放置多个标记 | 每人同时只能一个活跃标记 |
| 标记存在时限 | 默认 5 分钟 |
| 放置区域限制 | Boss 雾门前允许，Boss 战房间内不允许 |
| 灵体活动半径 | 超过 100m 强制拉回或遣返 |
| 灵体交互限制 | 不能拾取、不能对话、不能激活机关 |
| 断线重连 | 回到自己世界，不保留灵体状态 |

---

## 11 分阶段实施建议

### Phase 1 — 基础设施

```
目标: 可跑通的网络基础架构

状态: ✅ 代码结构已完成，待内容端配置

需要实现:
  ✅ SignManager GameInstanceSubsystem
  ✅ SummonSessionComponent（状态机框架 + 放置标记逻辑）
  ✅ ASL_SummonSign Actor（基础生成/销毁）
  □ 联机道具（召唤符）接入道具系统（数据表行 + 新增配置属性 道具功能类型 根据功能类型来确定执行流程）
  ✅ 基础的 RPC 框架（注册/交互/召唤）
  ✅ 等级/武器匹配校验

验证:
  - ✅ 能通过 PlaceSummonSign() 放置标记并注册到 SignManager
  - ✅ 能通过 QueryAvailableSigns() 查询符合条件的标记
  - ✅ 能通过 InteractWithSign() 锁定标记发起召唤
  - □ 正确消耗召唤符道具（待 GAS 能力接入）

待办:
  - 创建 GA_PlaceSummonSign GameplayAbility
  - 在道具数据表中添加召唤符条目
  - 配置 UseAbilityTag 关联 GA
  - 在 SummonSessionComponent 中监听道具使用事件自动调用 PlaceSummonSign
```

### Phase 2 — 世界穿梭

```
目标: 玩家能被传送到另一个世界并操作

需要实现:
  □ 世界切换机制（ServerTravel / SeamlessTravel）
  □ ASL_PhantomCharacter 灵体角色类
  □ 属性缩放计算
  □ 灵体输入映射（操作限制）
  □ PhantomData 数据打包与同步
  □ 召唤确认/拒绝流程 UI

验证:
  - A 成功进入 B 的世界并操作
  - 双方都能看到对方灵体
  - 灵体可以正常移动/攻击
```

### Phase 3 — 战斗同步

```
目标: 灵体与世界的完整交互

需要实现:
  □ 灵体的 GAS 能力复制
  □ 伤害同步（灵体→敌人 / 敌人→灵体）
  □ AI 对灵体的感知和反应
  □ 死亡同步（灵体/召唤者/Boss）
  □ 遣返流程与过渡 UI

验证:
  - 灵体可以攻击敌人造成伤害
  - 敌人可以攻击灵体
  - 各种死亡条件都能正确遣返
```

### Phase 4 — 完善与优化

```
目标: 稳定性、体验优化、扩展

需要实现:
  □ 密码匹配系统（好友召唤）
  □ 召唤范围可视化（标记高亮/光环）
  □ 断线重连与异常恢复
  □ 性能优化（复制带宽控制）
  □ 反作弊基础（属性校验）
  □ 所有边界条件处理
```

---

> 文档版本：v1.0
> 最后更新：2026-06-09
> 状态：架构设计稿（待评审）
