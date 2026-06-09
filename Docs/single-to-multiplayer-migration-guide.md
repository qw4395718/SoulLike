# 单机→联机迁移思维指南

> 适用版本：UE 4.26
> 目标：从一个"没考虑过联机"的单机 UE4 项目，迁移为可靠的联机项目
> 场景：Listen Server + 多个客户端 / DedicatedServer

---

## 目录

1. [核心思想](#1-核心思想)
2. [五个维度的思维转变](#2-五个维度的思维转变)
3. [路由判断模型](#3-路由判断模型)
4. [迁移优先级清单](#4-迁移优先级清单)
5. [常见陷阱速查](#5-常见陷阱速查)

---

## 1 核心思想

### 一条最根本的线

```
Server 拥有 Authority（权威）
   ↓  Replication（复制）推送到客户端
Client 只有"副本"，不是本体
```

**所有迁移决策都围绕这一个思想展开。**

这条线决定了三条铁律：

| 铁律 | 解释 |
|------|------|
| **谁有 Authority 谁做决定** | 伤害判定、物品掉落、AI 行为决策——服务器说了算 |
| **客户端只能请求，不能决定** | 客户端想说"我攻击了"→ 必须发 Server RPC 让服务器判定 |
| **数据在服务器是本体，客户端是副本** | 客户端看到的血量/位置/状态，都是服务器复制过来的快照 |

### 单机 vs 联机的默认姿势

```
单机代码的默认姿势："我相信我能做所有事"
联机代码的默认姿势："先问我是谁，再决定做什么"
```

这个转变不是在代码里加一个 `if` 就完了——是要在每个函数、每个变量、每个 Actor 上都过一遍。

---

## 2 五个维度的思维转变

### 2.1 GameMode vs GameState

| 单机思维 | 联机思维 |
|----------|---------|
| GameMode 是"游戏逻辑总管"，到处都是 `GetGameMode()` | GameMode **只在服务器存在**，客户端 `GetGameMode()` 永远是 `nullptr` |
| 一切逻辑往里塞 | GameMode 只负责**服务器侧的流程控制**（生成AI、判定胜负、关卡切换） |
| Widget、UI 直接拿 GameMode 做事 | UI 通过复制的 **GameState / PlayerState** 获取数据 |

#### GameState 的角色

GameState **会复制到所有客户端**。所有玩家都需要看到的游戏全局状态，放这里：

| 应放 GameState | 不应放 GameState |
|---------------|-----------------|
| 当前关卡名、游戏阶段 | 只和某个具体 Actor 有关的临时状态 |
| Boss 是否存活 | 只在服务器执行的配置参数 |
| 总游戏时间 | 每帧都在变的临时运算中间值 |
| 所有已连接的玩家列表 | |

#### PlayerState 的角色

PlayerState 也复制，每个玩家一个。所有其他玩家都能看到：

| 应放 PlayerState | 不应放 PlayerState |
|-----------------|-------------------|
| 玩家名、等级、装备索引 | 位置、速度（这些在 Pawn 上同步） |
| 存活/死亡状态 | 输入采集 |
| 队伍信息 | 只在本地显示的 UI 状态 |

#### 迁移动作

```
搜 GetGameMode → 逐个检查：
  ├─ 如果这段代码只会在服务器执行 → 安全，保持不动
  ├─ 如果在 Widget/UI/客户端组件中 → 必须改：
  │   ├─ 如果能从 GameState/PlayerState 获取 → 改用它们
  │   └─ 如果不能 → 通过 Server RPC 委托到服务器
  └─ 如果不确定在哪执行 → 加 HasAuthority() 分支
```

---

### 2.2 Actor 的网络角色

单机项目里所有 Actor 都是 `ROLE_Authority`。联机后分为三种：

| 角色 | 谁 | 含义 |
|------|----|------|
| 🟢 Authority | 服务器端 | 这是**本体**——有决策权 |
| 🔵 Autonomous Proxy | 自己控制的 Pawn 的客户端 | 有即时反馈（移动预测、攻击立即播放） |
| 🟠 Simulated Proxy | 别人控制的 Pawn 的客户端 | 只做插值模拟，不做决策 |

#### 需要做的决定

每增加一个 Actor，问三个问题：

```
① 这个 Actor 需要在客户端存在吗？
   否 → 设 bReplicates = false（仅服务器，如纯逻辑触发器）
   是 → 继续

② 它应该在服务器上生成后自动复制到客户端吗？
   是 → 设 bReplicates = true，在服务器 Spawn
   否 → 每个客户端独立生成（纯视觉效果、本地音效Actor）

③ 它的哪些属性需要同步？
   用 UPROPERTY(Replicated) / ReplicatedUsing=OnRep_XXX 标记
```

#### 常见 Actor 类型示例

| Actor 类型 | 复制策略 |
|-----------|---------|
| 敌人 AI | 服务器生成 + `bReplicates = true`，客户端显示 SimulatedProxy |
| 投掷物 | 服务器生成 + `bReplicates = true`，客户端插值位置 |
| 召唤标记 | 服务器生成 + `bReplicates = true`，仅在本世界复制 |
| 灵体角色 | 服务器生成 + `bReplicates = true`，在召唤者世界中可见 |
| UI 特效 Actor | 客户端本地生成（`bReplicates = false`），不占用复制带宽 |
| 捡起的道具 | 服务器生成 → 复制位置 → 捡起后服务器 Destroy |

---

### 2.3 RPC 的三条通路

单机代码：`obj->DoSomething()`
联机代码：需要想"这个函数应该在哪里执行？"

| RPC 类型 | 调用端 | 执行端 | 用途 | 例子 |
|---------|-------|-------|------|------|
| `Server` | 客户端 | 服务器 | 客户端请求做某事 | "我想攻击这个敌人" |
| `Client` | 服务器 | **特定**客户端 | 服务器通知某人 | "你被召唤了，播放确认弹窗" |
| `Multicast` | 服务器 | **所有**客户端 | 广播事件 | "Boss 死亡，所有人播放动画" |

#### 可靠性分类

| 类别 | RPC 类型 | 用途 |
|------|---------|------|
| Reliable | `Server_`、`Client_`、`Multicast_` 流程控制 | 生成、销毁、状态切换——**保证送达** |
| Unreliable | `Server_`/`Multicast_` 移动 | 位置同步——丢了就丢了，下一帧新的覆盖 |

#### 重要限制

```
Server RPC（客户端→服务器）：
  - 只在 Autonomous Proxy 上有效
  - Simulated Proxy 不能发 Server RPC
  - 需要有 _Validate 校验函数（UE4 强制 WithValidation）

Client RPC（服务器→客户端）：
  - 只能发给特定客户端，不能发给 Simulated Proxy 的 Owner
  - 客户端断开后，已发送但未处理的 Client RPC 会被丢弃

Multicast RPC（服务器→所有人）：
  - 不能传大块数据（可能超过 UObject 传输限制）
  - 调用方的客户端（如果是 Listen Server）会立即执行
```

#### _Validate 规则

UE4 要求 `WithValidation` 的 RPC 必须提供校验函数：

```cpp
// .h
UFUNCTION(Server, Reliable, WithValidation)
void Server_Attack(FName AttackID);

// .cpp
bool ASL_PlayerController::Server_Attack_Validate(FName AttackID)
{
    // 校验：防止作弊客户端传非法参数
    return AttackID.IsValid() && !AttackID.IsNone();
}

void ASL_PlayerController::Server_Attack_Implementation(FName AttackID)
{
    // _Validate 返回 true 才会走到这里
    ExecuteAttackOnServer(AttackID);
}
```

---

### 2.4 UI 和输入——最大的陷阱

单机项目的 UI 往往直接调用 GameMode、Character 的方法。**联机后这行不通。**

#### 数据流向

```
服务器端 (GameState / PlayerState)
    ↓ Replication（网络复制）
客户端 (副本)
    ↓ 读取
UI (Widget) —— 显示数据，收集用户输入
    ↓ 用户点击按钮
PC->ServerRPC() 发回服务器
    ↓
服务器执行真正的逻辑
```

#### 迁移检查清单

```
□ Widget 里有没有 GetGameMode()？
   → 删掉，改从 PlayerState/GameState 拿数据
   → 如果必须调 GameMode，通过 GetOwningPlayer()->ServerRPC()

□ Widget 里有没有 SpawnActor()？
   → 如果 Spawn 的是需要复制的 Actor，必须让服务器去 Spawn
   → 本地 VFX 角色可以继续在客户端 Spawn

□ Widget 里有没有修改游戏状态的逻辑？
   → 通过 Server RPC 发回服务器
   → UI 只做"展示"和"收集意图"，不做"决定"

□ Widget 的数据源是否可靠？
   → 从复制的 PlayerState/GameState 拿
   → 不要相信客户端本地存储的"当前值"
```

#### 典型的"按钮事件"模式

```cpp
// 错误：客户端调用 GetGameMode 永远 nullptr
void UMyWidget::OnStartGameClicked()
{
    if (AGameMode* GM = GetGameMode(GetWorld()))  // ❌ 客户端 nullptr
    {
        GM->StartGame();
    }
}

// 正确：通过 PlayerController 发 RPC
void UMyWidget::OnStartGameClicked()
{
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        Cast<AMyPlayerController>(PC)->RequestStartGame();
    }
}
```

#### PC 上的"路由"模式

```cpp
// PlayerController 上的统一入口
void AMyPlayerController::RequestStartGame()
{
    if (HasAuthority())
    {
        // 服务器直接执行
        if (AGameMode* GM = GetWorld()->GetAuthGameMode())
        {
            GM->StartGame();
        }
    }
    else
    {
        // 客户端发 RPC 到服务器
        Server_RequestStartGame();
    }
}
```

---

### 2.5 移动与物理

#### 默认行为

UE4 的 `CharacterMovementComponent` 自带网络同步，但需要正确配置：

```
客户端的本地移动（即时反馈 —— 对按下 WASD 的第一时间有响应）
    ↓  每帧向服务器发送打包的输入（SavedMove）
服务器做权威移动（验证并修正位置）
    ↓  每帧将权威位置复制回客户端
客户端的插值显示（如果服务器位置和本地预测不一致，角色会被"拉回"）
```

#### 常见问题

| 问题 | 原因 | 修复方向 |
|------|------|---------|
| 角色在空中卡顿 | 客户端的预测和服务器不一致 | 检查 `Network Prediction` 相关参数 |
| 敌人位置一跳一跳 | 复制频率太低或插值没开 | `NetUpdateFrequency`、`bReplicateMovement` 配置 |
| 物理模拟的物体不动 | 物理模拟只在服务器跑 | 开启 `bReplicatePhysicsToAutonomousProxy` 或手动同步 |
| 角色穿墙 | 客户端本地移动未验证 | 加服务器端位置校验，或缩小最大移动距离 |

#### 关键参数

```
CharacterMovementComponent:
  - NetworkMaxSmoothUpdateDistance      — 平滑修正的最大距离
  - NetworkNoSmoothUpdateDistance       — 超过此距离直接跳跃修正
  - NetworkMinTimeBetweenClientAdjustments — 客户端修正冷却（防抖动）

Character:
  - bReplicateMovement = true
  - NetUpdateFrequency — 越高越精确，但带宽消耗越大
```

---

## 3 路由判断模型

一个可以贴在工位上的流程图：

```
① 这段逻辑需要 Authority（服务器权威）吗？
   ↓                               ↓
   需要                            不需要
   ↓                               ↓
   继续问问题②                →  本地执行
   ↓                              （VFX、音效播放、UI动画、
  ② 这个结果所有客户端               纯客户端输入处理）
      都需要看到吗？
   ↓               ↓
   是              否
   ↓               ↓
   用复制的数据：   用 Client RPC
   GameState、     通知特定客户端
   PlayerState、   或不通知（服务器静默执行）
   Replicated 属性
```

### 实战例子

| 场景 | 路由分析 |
|------|---------|
| 扣血 | 需要 Authority → 服务器判定 → PlayerState 同步 → 所有客户端看到 |
| 播放受伤动画 | 需要 Authority → 服务器判定 → Multicast / ReplicatedUsing OnRep |
| 拾取道具 | 需要 Authority → 服务器判定 → 服务器 Destroy → 复制同步到客户端 |
| 按下攻击键 | 本地立刻播动画（预测）→ 同时发 Server RPC |
| 打开背包界面 | 不需要 Authority → 纯客户端 UI 操作 |
| 显示"你被召唤"弹窗 | 需要通知指定客户端 → Client RPC |
| Boss 死亡广播 | 需要 Authority 判定 → Multicast RPC |

---

## 4 迁移优先级清单

从你的项目现状出发，按风险排序：

### P0 🔴 立即改

- [ ] **所有 Widget 中的 `GetGameMode()`**
  - 典型位置：按钮事件、NativeConstruct、NativeTick
  - 症状：客户端点击无反应
  - 修复：改为通过 PlayerController 的 RPC 路由

- [ ] **所有 `Server_` RPC 的 `_Validate` 函数**
  - 典型位置：有 `UFUNCTION(Server, Reliable, WithValidation)` 的类
  - 症状：编译警告或运行时断线
  - 修复：每个带 `WithValidation` 的 RPC 补 `_Validate()` 实现

### P1 🟡 尽快处理

- [ ] **Widget 中直接修改游戏状态的逻辑**
  - 道具使用、装备切换、属性加点等
  - 客户端不能直接操作数据，必须通过 Server RPC

- [ ] **PlayerController::BeginPlay 中依赖 GameMode 的逻辑**
  - 网络时序：客户端 PC 的 BeginPlay 时 GameMode 可能还没准备好
  - 改为延时执行或依赖事件驱动

- [ ] **`GetWorld()->SpawnActor` 的调用位置审查**
  - 需要复制的 Actor 只能在服务器 Spawn
  - 本地特效/音效可以在客户端 Spawn

### P2 🟢 逐一排查

- [ ] **Actor 的 `bReplicates` 标记**
  - 需要被所有客户端看到的 Actor → `bReplicates = true`

- [ ] **属性的 `UPROPERTY(Replicated)` / `ReplicatedUsing` 标记**
  - 只有标记了的变量才会同步
  - `ReplicatedUsing` 可以绑定 OnRep 函数做同步后处理

- [ ] **`GetLifetimeReplicatedProps` 是否覆盖了所有需要复制的变量**
  - 每加一个 `Replicated` 属性，都必须在这里注册

- [ ] **PlayerController 的 `bReplicates`**
  - 默认 true，但如果手动设了 false 会导致客户端看不到自己的 Controller

### P3 🔵 优化与完善

- [ ] **角色移动预测与修正参数调优**
- [ ] **GAS 的复制配置**（ASC 复制模式、GE 复制策略、GA 激活规则）
- [ ] **带宽控制**（NetUpdateFrequency 调优、移除不必要的复制属性、DORMANT 优化）
- [ ] **断线重连**（状态保存、全量同步）

---

## 5 常见陷阱速查

### 编译/代码层

```
□ "No Validate function" 编译错误
  解：所有 WithValidation 的 RPC 必须实现 _Validate() 函数

□ Server RPC 调了但没执行
  检：调用方是 Autonomous Proxy 吗？
  检：PC/Pawn 的 bReplicates 是否为 true？

□ Client RPC 没收到
  检：RPC 对端的 NetDriver 配置正确吗？
  检：Client RPC 是否在服务器上调用？

□ Cast<AGameMode> 在客户端返回 nullptr
  解：这是正常的。用 GetWorld()->GetAuthGameMode() 确保只在服务器调
```

### 运行时层

```
□ 属性改了但客户端没看到更新
  检：是否加了 UPROPERTY(Replicated)？
  检：是否在 GetLifetimeReplicatedProps 里注册了？

□ 两个客户端看到的血量/位置不一样
  检：是否所有状态修改都在服务器进行？
  检：客户端是否在本地"猜测"修改了复制的属性？

□ 角色在客户端上穿模/瞬移
  检：CharacterMovementComponent 复制参数是否配置？
  检：是否有自定义移动逻辑未处理预测？

□ 敌人不动
  检：敌人的 AI 逻辑在服务器上运行吗？
  检：敌人 Actor 的 bReplicates = true？
```

### 架构层

```
□ 是否所有 GameMode 操作都通过 RPC 路由了？
  检：所有 HasAuthority 分支是否正确覆盖了"客户端该做什么"？

□ 是否所有游戏状态数据都来自复制的源，而不是客户端本地缓存？
  检：血条显示的数据源是 PlayerState 还是本地缓存的浮点数？

□ 是否考虑了 Listen Server 的双重角色？
  注意：Listen Server 既是服务器也是客户端
  它在服务器侧有 Authority，同时也会走客户端的渲染/输入路径

□ 是否考虑过"如果同时两个玩家调用同一个函数"？
  例：两个客户端同时交互同一个标记 → 需要服务器加锁或状态校验
```

---

## 附：代码审查自查清单

加一个函数之前，逐条排查：

```
□ 这个函数是否应该仅在服务器上执行？
  是 → 函数体最顶部加 if (!HasAuthority()) return;

□ 这个函数是否修改了游戏状态？
  是 → 确保只在服务器的 HasAuthority() 分支中修改

□ 这个函数是否在某些情况下需要通知客户端？
  是 → 修改后用 Multicast_ / Client_ RPC 通知

□ 这个函数接收的参数是否可能被作弊客户端伪造？
  是 → Server RPC 的 _Validate 加校验
```

---

> 文档版本：v1.0
> 最后更新：2026-06-09
> 适用项目：SoulLike (UE 4.26)
