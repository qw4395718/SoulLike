# 联机召唤系统技术架构设计

> 设计目标：为 SoulLike 项目实现类似《黑暗之魂》风格的召唤符联机系统
> 当前阶段：Phase 1 完成，Phase 2 规划中
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
                           │  中间匹配服务（Lobby）     │
                           │  Socket / HTTP 本地服务   │
                           └──────────┬───────────────┘
                                      │ 标记注册 / 查询 / 转发请求
                     ┌────────────────┼───────────────┐
                     │                │                │
           ┌─────────▼────────┐ ┌────▼─────┐ ┌───────▼────────┐
           │  UE4 实例 A      │ │ 匹配服务  │ │  UE4 实例 B    │
           │  (A 的世界)      │ │ (状态层)  │ │  (B 的世界)    │
           │                  │ │          │ │                │
           │  ListenServer    │ │ 标记列表  │ │  ListenServer  │
           │   + 客户端A      │ │ 匹配队列  │ │   + 客户端B    │
           └──────────────────┘ └──────────┘ └────────────────┘
```

### 2.2 当前阶段拓扑（单服 PIE 调试，已完成）

```
PIE 进程 × 1
  └── 监听服务器（加载一张图）
       ├── 客户端A（操作 PlayerA）
       ├── 客户端B（操作 PlayerB）
       └── SignManager（GameInstanceSubsystem，内存管理标记）
```

### 2.3 目标拓扑（跨世界联机）

```
UE4 实例 A（A 的世界）          中间服务（本地）          UE4 实例 B（B 的世界）
  ListenServer + 客户端A          Socket 服务端         ListenServer + 客户端B
       │                              │                      │
       │── ① 放置标记 ──────────────→│                      │
       │                              │── ② 注册标记 ──────→│
       │                              │←─ ③ 查询标记 ──────│
       │                              │                      │
       │                              │←─ ④ 请求召唤 ──────│
       │←─ ⑤ 转发召唤请求 ──────────│                      │
       │── ⑥ 确认召唤 ─────────────→│                      │
       │                              │── ⑦ 通知确认 ─────→│
       │                              │                      │
       │ ⑧ A 断开当前进程            │                      │
       │ ⑨ A 的客户端连接到 B 的 IP  │                      │
       │───────────────────────────────────────────→│
       │                              │                      │
       │                              │←─ ⑩ 拉取 PhantomData│
       │                              │                      │
       │                              │                      │ ⑪ 生成灵体
```

### 2.4 方案对比

| 方案 | 优势 | 劣势 | 适用阶段 |
|------|------|------|---------|
| **A: 单服多世界分区** | PIE 即可调试，架构简单 | 不是真正的跨世界 | Phase 1（已完成） |
| **B: 多实例 + 中间服务** | 真实跨世界联机，扩展性好 | 需要两个进程调试 | Phase 2（当前阶段） |
| **C: P2P 直连 + 信令** | 无需独立服务器 | NAT 穿透复杂，防作弊困难 | 后续考虑 |

---

## 3 核心概念

### 3.1 数据结构

```cpp
// ===== 召唤标记数据 =====
USTRUCT(BlueprintType)
struct FSummonSignInfo
{
    GENERATED_BODY()

    UPROPERTY()
    FGuid SignID;

    UPROPERTY()
    FUniqueNetIdRepl OwnerPlayerID;
    UPROPERTY()
    FString OwnerPlayerName;
    UPROPERTY()
    int32 OwnerLevel;
    UPROPERTY()
    int32 OwnerWeaponLevel;

    UPROPERTY()
    FName CurrentLevelName;
    UPROPERTY()
    FTransform SignTransform;

    UPROPERTY()
    float TimeRemaining;

    UPROPERTY()
    ESummonSignState State;

    // 所属世界实例信息（用于跨世界通信定位）
    UPROPERTY()
    FString WorldInstanceID;
};

// ===== 召唤状态枚举 =====
UENUM(BlueprintType)
enum class ESummonSignState : uint8
{
    Active,
    BeingSummoned,
    Expired,
    Removed
};

// ===== 玩家联机状态 =====
UENUM(BlueprintType)
enum class EOnlinePlayerState : uint8
{
    Solo,
    PlacingSign,
    SignActive,
    BeingSummoned,
    SummonedAsPhantom,
    SummoningOther,
    HasPhantom,
};

// ===== 灵体数据（跨世界传输） =====
USTRUCT(BlueprintType)
struct FPhantomData
{
    GENERATED_BODY()

    UPROPERTY()
    FString CharacterMeshPath;
    UPROPERTY()
    TArray<FString> EquipmentPaths;

    UPROPERTY()
    int32 Level;
    UPROPERTY()
    int32 WeaponLevel;
    UPROPERTY()
    float HealthPercent;

    UPROPERTY()
    FString OwnerName;
    UPROPERTY()
    FGuid SummonSessionID;
};

// ===== 匹配配置 =====
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

### 3.2 关键 Actor 类型

```cpp
ASL_SummonSign : public AActor
{
    UPROPERTY(Replicated)
    FSummonSignInfo SignInfo;

    UPROPERTY(Replicated)
    bool bIsBeingSummoned;

    UPrimitiveComponent* InteractCollision;
    UStaticMeshComponent* VisualMesh;
    UWidgetComponent* NameWidget;
};

ASL_PhantomCharacter : public ACharacter
{
    UPROPERTY(ReplicatedUsing = OnRep_PhantomData)
    FPhantomData PhantomData;

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
│  │  • 注册/注销标记   │   │  • 发起放置标记         │      │
│  │  • 查询可用标记    │   │  • 发起召唤请求         │      │
│  │  • 标记过期清理    │   │  • 响应召唤邀请         │      │
│  │  • 匹配范围校验    │   │  • 状态管理             │      │
│  └──────────────────┘   └────────────────────────┘      │
│                                                          │
│  ┌──────────────────┐   ┌────────────────────────┐      │
│  │  SummonSignActor   │   │  PhantomController     │      │
│  │  (World Actor)    │   │  (Remote世界控制器)    │      │
│  │  • 标记可视化      │   │  • 灵体操作映射        │      │
│  │  • 交互触发        │   │  • 输入转发            │      │
│  │  • 生命周期        │   │  • 缩放属性管理        │      │
│  └──────────────────┘   └────────────────────────┘      │
│                                                          │
│  ┌──────────────────┐   ┌────────────────────────┐      │
│  │  MatchClientComp  │   │  中间匹配服务           │      │
│  │  (GameInstance)   │   │  (独立进程)            │      │
│  │  • Socket 通信     │   │  • 标记列表管理        │      │
│  │  • 消息序列化      │   │  • 请求转发            │      │
│  │  • 远程查询        │   │  • 世界实例注册        │      │
│  └──────────────────┘   └────────────────────────┘      │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

### 4.2 各组件职责

**SignManager**（已有，Phase 1 完成）
- 维护全局标记列表
- 处理标记的注册/查询/移除
- Phase 2 改造目标：改为从中间匹配服务远程查询

**SummonSessionComponent**（已有，Phase 1 完成）
- 管理玩家联机状态机
- 处理放置标记/被召唤流程
- 世界切换前后的数据保存/恢复

**MatchClientComponent**（新增，Phase 2）
- 与中间匹配服务的 Socket 连接
- 消息序列化/反序列化
- 连接状态管理（心跳/重连）

**中间匹配服务**（新增，独立进程）
- 世界实例注册/注销
- 标记列表管理
- 召唤请求转发

---

## 5 核心流程

### 5.1 召唤流程（跨世界版）

```
[实例B - 召唤者]        [中间服务]         [实例A - 放置者]
     │                      │                    │
     │ 1. 交互标记           │                    │
     │ 2. —请求召唤───────→│                    │
     │                      │ 3. —转发请求───────→│
     │                      │                    │ 4. 确认弹窗
     │                      │                    │ 5. —接受─────→│
     │                      │                    │    (或拒绝)
     │                      │←── 接受通知 ──────│
     │ 6. ←─收到接受通知──│                    │
     │                      │                    │ 7. 打包 PhantomData
     │                      │                    │ 8. 数据→中间服务 │
     │                      │←── PhantomData ──│                    │
     │                      │                    │
     │ 9. A的客户端断开A服务器                    │
     │ 10. A的客户端连接到B服务器                  │
     │ 11. B服务器根据PhantomData生成灵体          │
     │ 12. A现在操作灵体                           │
```

### 5.2 灵体返回/遣返

触发条件：灵体死亡、召唤者死亡、Boss 击杀、灵体主动返回、网络断线

流程：
1. 检测到遣返条件（B 服务器上）
2. 通知灵体客户端播放返回特效
3. 销毁灵体角色
4. 断开灵体客户端连接
5. 灵体客户端重新连接到自己的世界（A 服务器）
6. 恢复到最后一个安全位置

---

## 6 RPC 接口设计

### 6.1 阶段一接口（单服，已有）

```
SignManager (Server Only):
  Server_RegisterSign(Owner, Transform)
  Server_UnregisterSign(SignID)
  Server_RequestSummon(SignID, Requester)        [WithValidation]
  Server_AcceptSummon(SignID)                    [WithValidation]
  Server_DeclineSummon(SignID)

放置者侧回调:
  Client_OnSummonRequested(SignID, SummonerInfo)
  Client_OnSummonAccepted(SignID)
  Client_OnReturnToOwnWorld(ReturnReason)

召唤者侧回调:
  Client_OnPhantomSpawned(PhantomData)
  Client_OnPhantomDeparted()

灵体同步:
  Server_PhantomMove(Movement)                   [Unreliable]
  Multicast_PhantomAttack(AbilityHandle)         [Reliable]
```

### 6.2 阶段二新增接口（跨世界通信，JSON/Socket）

```
世界实例管理:
  {"type":"register_instance","instance_id":"...","map":"...","ip":"...","port":...}
  {"type":"unregister_instance","instance_id":"..."}

标记管理:
  {"type":"register_sign","owner_name":"...","level":...,"map":"...","transform":{...}}
  {"type":"unregister_sign","sign_id":"..."}
  {"type":"query_signs","map":"...","level":...,"weapon_level":...}

召唤流程:
  {"type":"request_summon","sign_id":"...","requester_name":"...","requester_id":"..."}
  {"type":"accept_summon","sign_id":"..."}
  {"type":"decline_summon","sign_id":"..."}

数据传输:
  {"type":"transfer_phantom","sign_id":"...","data":{...},"target_instance":"..."}
```

### 6.3 可靠性分类

| 类别 | 通信方式 | 可靠性 |
|------|---------|--------|
| 流程控制 | Reliable RPC + Socket | 保证送达 |
| 状态同步 | Unreliable RPC | 丢了就丢了 |
| 战斗事件 | Reliable RPC | 保证送达 |
| 加载同步 | Reliable RPC | 保证送达 |
| 跨世界消息 | Socket（TCP） | TCP 保证 |

---

## 7 复制策略

| Actor 类型 | 复制范围 |
|-----------|---------|
| ASL_SummonSign | 仅放置者世界，通过中间服务暴露 |
| ASL_PhantomCharacter | 仅召唤者世界 |
| ASL_PlayerState | 不跨世界，穿梭时打包传输 |

```
缩放后伤害 = 原始伤害 × (召唤者等级 / 放置者等级) × 缩放曲线
缩放后血量 = 原始血量 × 世界人数缩放系数
```

---

## 8 匹配条件

| 条件 | 说明 |
|------|------|
| 同关卡 | 相同 MapName |
| 等级范围 | ±10 级 |
| 武器等级范围 | ±2 级 |
| 区域状态 | 尚未击败 Boss |
| 玩家状态 | 双方可联机 |

---

## 9 状态机

### 放置者

```
     Solo ──→ PlacingSign ──→ SignActive ──→ BeingSummoned ──→ SummonedAsPhantom ──→ Solo
                                        ↑                                            │
                                        └────────────────────────────────────────────┘
```

### 召唤者

```
     Solo ──→ SummoningOther ──→ HasPhantom ──→ Solo
                                                      │
                                                      └── Departed
```

---

## 10 异常与边界

| 场景 | 处理方式 |
|------|---------|
| 放置者断线 | 移除标记，遣返灵体 |
| 召唤者断线 | 遣返灵体，移除标记 |
| 同时交互 | bIsBeingSummoned 锁 + 服务端校验 |
| Boss 击杀 | 遣返所有灵体 |
| 中间服务断连 | 本地缓存，定期重试 |
| 灵体重连失败 | 强制遣返 |

---

## 11 分阶段实施建议

### Phase 1 — 基础设施（已完成 ✅）

```
目标: 可跑通的单服基础架构
耗时: 已完成

已实现:
  ✅ SignManager GameInstanceSubsystem
  ✅ SummonSessionComponent（状态机 + 放置标记）
  ✅ ASL_SummonSign Actor（基础生成/销毁/网络复制）
  ✅ 联机道具接入（数据表配置 + 道具功能类型）
  ✅ 基础 RPC 框架（注册/交互/召唤）
  ✅ 等级/武器匹配校验
  ✅ UI 到服务器的 RPC 路由修复
  ✅ 道具使用 Server RPC 改造

已验证:
  - ✅ PlaceSummonSign 放置并注册
  - ✅ QueryAvailableSigns 查询
  - ✅ InteractWithSign 锁定交互
  - ✅ 正确消耗道具
```

### Phase 2 — 跨世界联机基础设施（已完成 ✅）

```
目标: 两个独立的 UE4 实例能通过中间服务互相发现和通信

需要实现:
  ■ 中间匹配服务（独立进程，Python/C#）
    - 世界实例注册/注销
    - 标记列表管理（增删查）
    - 召唤请求转发
    - JSON 文本协议

  ■ MatchClientComponent（UE4 端 GameInstanceSubsystem）
    - TCP Socket 连接
    - 消息序列化/反序列化
    - 心跳与重连

  ■ 标记跨世界可见性
    - SignManager 改造为远程查询
    - 远程标记在本地生成可交互的 SignActor

  ■ 召唤请求/确认流程
    - 中间服务转发到放置者
    - 确认/拒绝弹窗
    - 结果返回召唤者

验证:
  - 实例A放置标记 → 实例B客户端看到标记
  - 实例B交互 → 实例A收到请求
  - 实例A确认 → 实例B收到通知
```

### Phase 3 — 角色穿梭与灵体生成

```
目标: A 的角色能从自己世界转移到 B 的世界

需要实现:
  ■ FPhantomData 数据打包
  ■ 客户端网络切换（断开→连接 B 的服务器）
  ■ ASL_PhantomCharacter 灵体角色
  ■ 灵体输入映射
  ■ 召唤确认/拒绝 UI

验证:
  - A 确认后断开自己世界，连接到 B
  - A 操作 PhantomCharacter
  - B 看到 A 的灵体
  - 灵体外观与 A 一致（半透明）
  - 灵体不能捡道具/开机关
```

### Phase 4 — 战斗同步与遣返

```
目标: 灵体与 B 世界完整交互

需要实现:
  ■ 灵体 GAS 能力复制
  ■ 伤害同步
  ■ AI 感知与仇恨
  ■ 死亡/遣返流程

验证:
  - 灵体攻击敌人造成伤害
  - 敌人攻击灵体
  - 各种遣返条件正确执行
```

### Phase 5 — 完善与优化

```
  □ 密码匹配
  □ 视觉效果
  □ 断线重连
  □ 性能优化
  □ 反作弊
  □ 边界条件
```

---

> 文档版本：v2.0
> 最后更新：2026-06-10
> 状态：Phase 1 完成，Phase 2 规划中
