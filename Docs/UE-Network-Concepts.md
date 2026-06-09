# UE 网络概念指导

> 适用版本：UE 4.26
> 定位：UE4 网络底层概念的逐步讲解材料，配合单机→联机迁移指南使用
> 说明：这份文档随着我对 UE 网络理解的加深会持续补全和修正

---

## 目录

1. [从实例数量理解网络角色](#1-从实例数量理解网络角色)
2. [函数执行的端范围](#2-函数执行的端范围)
3. [动画同步的真相](#3-动画同步的真相)
4. [NetGUID——网络层的寻址方式](#4-netguid网络层的寻址方式)
5. [属性复制的路径追踪](#5-属性复制的路径追踪)
6. [常见认知误区](#6-常见认知误区)

---

## 1 从实例数量理解网络角色

### 一个最简单的场景

假设 Listen Server + 两个客户端，三人联机：

```
服务器（PIE窗口1）—— 运行在本地，同时扮演"服务器"和"一个玩家"
客户端A（PIE窗口2）
客户端B（PIE窗口3）
```

每个玩家控制一个角色：PlayerA、PlayerB、PlayerC。

### 每个端上有几个角色？

| 端 | PlayerA（我自己） | PlayerB（别人） | PlayerC（别人） |
|---|-----------------|----------------|----------------|
| 服务器 | Authority（本体） | SimulatedProxy（副本） | SimulatedProxy（副本） |
| 客户端A | AutonomousProxy（自己的副本） | SimulatedProxy（别人的副本） | SimulatedProxy（别人的副本） |
| 客户端B | SimulatedProxy（别人的副本） | AutonomousProxy（自己的副本） | SimulatedProxy（别人的副本） |

数量上每个端确实能看到三个角色，但性质完全不同：

- **Authority（服务器·本体）**：有决策权。碰撞判断、伤害计算、AI 行为——所有游戏规则在这里执行
- **AutonomousProxy（自己·副本）**：有即时反馈。按下 WASD 立刻移动，按下攻击立刻播动画——但最终以服务器为准
- **SimulatedProxy（别人·副本）**：没有决策权。只是一个"空壳"，跟随服务器推过来的位置/状态做插值展示

### 核心直觉

```
服务器上的 PlayerA → 本体，管决策
客户端A上的 PlayerA → 自己的代理人，管即时反馈
客户端B上的 PlayerA → 别人的空壳，管展示
```

你看别人屏幕上的你的角色，和别人自己屏幕上的你的角色，是两种完全不同的东西。

---

## 2 函数执行的端范围

### 一张表总结

| 调用方 | 函数类型 | 执行端 |
|--------|---------|--------|
| 客户端 | 普通本地函数 | 仅本客户端 |
| 客户端 | Server_RPC (Reliable) | 仅服务器（1次） |
| 服务器 | 普通服务器函数 | 仅服务器 |
| 服务器 | Client_RPC (Reliable) | 指定的那一个客户端（1次） |
| 服务器 | Multicast_RPC (NetMulticast) | 服务器 + 所有客户端上的同一个 Actor 副本（N次） |
| 服务器 | 修改 Replicated 属性 | 所有客户端自动同步数据，不执行函数（除非有 OnRep） |

### 典型攻击流程举例

玩家 A（客户端 A）按攻击键：

```
① 客户端A：输入检测
   → 本地立刻播放攻击动画（预测，让玩家感觉即时响应）
   → 同时发送 Server_Attack(TargetID) RPC 到服务器

② 服务器：执行 Server_Attack_Implementation()
   ├── 用服务器上 PlayerA 的碰撞盒做攻击判定
   ├── 如果命中 PlayerB：
   │   ├── PlayerB.Health -= 伤害
   │   │   （Health 标记了 Replicated → 自动同步到所有客户端）
   │   └── 如果需要全网表现 → PlayerB->Multicast_PlayHitReact()
   └── 如果没命中：
       └── 客户端A的本地攻击动画会被服务器"拉回"到空闲状态

③ 同步结果：
   ├── Health 属性复制 → 所有客户端更新 PlayerB 的血条
   ├── PlayerB->Multicast_PlayHitReact() → PlayerB 在所有客户端的副本播放受击
   └── 客户端A 可能收到服务器的位置修正
```

### 关键认知

决策永远在服务器。客户端只是请求（Server RPC）+ 展示结果（属性复制 / Multicast RPC）。

---

## 3 动画同步的真相

### 核心回答

> 客户端B上的PlayerA是服务器通知然后播了动画，还是直接同步了服务器端PlayerA的骨骼状态？

**两者都不是。** 更接近"服务器同步了几个驱动因子，客户端B上的AnimBP根据这些因子本地计算动画"。

### SimulatedProxy 动画的四个驱动层次

#### 层次一：移动数据（自动，无需代码）

```
服务器上 PlayerA 的速度 = 600（正在跑步）
    ↓ CharacterMovementComponent 自动复制
客户端B上 PlayerA_SimulatedProxy 的速度 = 600
    ↓ 本地 AnimBP 根据速度值 → 过渡到跑步动画
```

CharacterMovementComponent 会自动复制位置、旋转、速度、加速度中的关键帧。不需要写任何代码，走/跑/停的动画过渡就已经能工作了。

#### 层次二：AnimMontage 复制（通过 Multicast RPC）

```cpp
// 服务器上，PlayerA 的实例
PlayerA->Multicast_PlayMontage(SlashMontage);
```

执行范围：
- 服务器端 PlayerA（Authority）→ 播放
- 客户端A 上的 PlayerA（AutonomousProxy）→ 播放
- 客户端B 上的 PlayerA（SimulatedProxy）→ 播放
- 客户端B 上的 PlayerB（AutonomousProxy）→ 不播放
- 客户端上所有其他 Actor → 不播放

**Multicast_RPC 是绑定在特定 Actor 实例上的，不是全客户端广播。**

#### 层次三：自定义状态变量（通过属性复制）

```cpp
UPROPERTY(ReplicatedUsing = OnRep_AttackState)
bool bIsAttacking;
```

服务器修改 bIsAttacking = true → 自动复制到所有客户端 → 各自的 OnRep_AttackState() → AnimBP 过渡到攻击

由于网络延迟，客户端B上这个状态切换比客户端A晚几帧。

#### 层次四：直接骨骼复制（极少数情况使用）

UE4 支持直接同步骨骼姿态，但消耗极大，通常不这么用。一般只在特定姿势同步（如攀爬抓取点）或极小规模游戏中才会考虑。

### 总结

| 驱动方式 | 同步什么 | 客户端B上的效果 |
|---------|---------|---------------|
| 移动数据（自动） | 位置/速度/加速度 | AnimBP 根据速度切走/跑/停 |
| Montage RPC（手动） | "播放这个 Montage" | 本地播放同一套 Montage |
| 状态变量（属性复制） | bIsAttacking / bIsBlocking | AnimBP 根据布尔值过渡 |
| 骨骼复制（极少用） | 每一根骨骼的变换 | 精确复制姿态 |

最典型的项目做法：Montage 播放攻击/受击/处决等一次性关键动画，通过 Multicast_PlayMontage 同步；走/跑/跳等循环动画由复制的移动数据自动驱动 AnimBP。

---

## 4 NetGUID——网络层的寻址方式

### 核心问题

> UE 是怎么找到同一个 Actor 在所有客户端上的副本的？

### 答案：NetGUID

#### 第一步：分配身份证

服务器上每个 bReplicates = true 的 Actor 在生成时，被分配一个唯一的 NetGUID（uint32，从 1 递增）：

```
服务器上映射表：
  NetGUID 1 → PlayerA（指针）
  NetGUID 2 → PlayerB（指针）
  NetGUID 3 → EnemyBoss（指针）
```

#### 第二步：第一次告知客户端

当客户端连接时，服务器把这个 Actor 的类信息、初始属性和 NetGUID 一起发给客户端。客户端在本地生成副本，也建一张映射表。

同一个 Actor 在所有端上的 NetGUID 相同。

#### 第三步：通过 NetGUID 做 RPC 寻址

```
服务器上：PlayerA->Multicast_PlayMontage()
  ↓
UE4 查出 PlayerA 的 NetGUID = 1
  ↓
序列化成网络包：{ TargetNetGUID: 1, RPC: "Multicast_PlayMontage", Args: [...] }
  ↓
发送到所有客户端
  ↓
客户端A收到：查表 NetGUID 1 → 我的 PlayerA_副本 → 调用 PlayMontage
客户端B收到：查表 NetGUID 1 → 我的 PlayerA_副本 → 调用 PlayMontage
```

### 支持这个过程的核心组件

| 组件 | 职责 |
|------|------|
| UNetDriver | 网络驱动顶层，管理整个连接 |
| FNetworkObjectList | 服务器上所有 bReplicates=true 的 Actor 列表 |
| FNetGUIDCache | 每个连接一份的 GUID → Actor 映射表 |
| FRepLayout | 每个 Actor 的属性布局表，指导序列化 |
| FObjectReplicator | 每个 Actor + 每个连接一份，跟踪同步状态 |
| Channel | 每个连接 + 每个 Actor 一个通道，序列化/RPC 走这里 |

---

## 5 属性复制的路径追踪

以 PlayerB->Health 为例：

```
① 服务器修改了标记了 UPROPERTY(Replicated) 的属性
② FRepLayout 检测到 Health 变化
③ NetDriver 在每帧的复制循环中遍历所有 Actor 和连接
④ 对有变化的属性序列化差值，通过网络发送
⑤ 客户端收到后，查表找到本地 PlayerB 副本，设置 Health = 90
   如果标记了 ReplicatedUsing=OnRep_Health，则调用 OnRep_Health()
```

### 优化机制

| 机制 | 作用 |
|------|------|
| Delta 压缩 | 只传变化值，不传整个属性表 |
| 条件复制（COND_*） | 只在满足条件时复制 |
| DORMANT | Actor 暂时无变化时标记为休眠 |
| NetUpdateFrequency | 控制每秒最大复制次数 |

---

## 6 常见认知误区

### 误区 1："客户端A按了攻击键，所以三个端都播放了攻击动画"

实际：客户端A本地立刻播（预测），服务器收到 RPC 处理逻辑后通过属性复制或 Multicast 推送结果。客户端B上播放攻击动画的原因是"服务器通过 RPC 让 PlayerA 在所有端上的副本执行了操作"，不是"客户端A通知了客户端B"。

### 误区 2："Multicast RPC 是通知所有客户端的所有 Actor"

实际：Multicast 绑定在特定 Actor 上。PlayerA->Multicast_PlayMontage() 只有 PlayerA 的副本们收到。要让所有 Actor 都看到某事件，需要通过 GameState 上的 Multicast 或遍历 PlayerController 发 Client RPC。

### 误区 3："客户端B上的PlayerA的动画，是从服务器复制了骨骼姿态"

实际：大多数情况下，客户端B上的 PlayerA 的 AnimBP 是本地实时计算的。它根据复制的移动数据、状态变量、Montage 指令等驱动因子在本地 BlendSpace 中计算姿态，不是"播放服务器的录像"。

### 误区 4："Server RPC 是服务器发给客户端的"

实际：Server_ 前缀的 RPC 是客户端发给服务器的。服务器发给客户端的用 Client_ 前缀。

### 误区 5："复制属性 = 复制变量值"

实际：UPROPERTY(Replicated) 只标记"需要网络同步"。UE4 的复制系统用的是脏标记 + 快照对比，只有属性值发生变化时才被序列化和发送，且实际传输的是压缩后的二进制数据。

---

## 7 Server RPC 的执行与反馈模式

### Server RPC 没有返回值

Server RPC 是客户端→服务器的单向通知，**没有 return 值**。

```
客户端调用 Server_RequestNewGame()
    ↓ UE4 拦截并序列化（函数名 + 参数）
    ↓ 通过 NetGUID 标记调用者
    ↓ 发给服务器
服务器收到并反序列化
    ↓ 先执行 _Validate()（也在服务器上）
    │  ├─ true → 继续执行
    │  └─ false → 断开客户端（UE4 默认安全策略）
    ↓ 执行 _Implementation()
```

_cpp 代码里 `Server_` 前缀的调用写在客户端，但 `_Implementation` 里的逻辑永远在服务器上执行。所以 `_Implementation` 里可以放心用 `GetAuthGameMode()`。

### 什么时候需要回复结果

Server RPC 把结果传回客户端有三种方式：

| 方式 | 谁收到 | 适用场景 |
|------|--------|---------|
| `Replicated` 属性 | 所有客户端自动同步 | 血量、状态变量—「客户端需要知道最新值」 |
| `Client_` RPC | 发起者那一台客户端 | 单个玩家的通知—「你的攻击没中」「你被召唤了」 |
| `Multicast_` RPC | 所有客户端 | 全网表现—「Boss 死亡爆炸」「群伤动画」 |

**判断标准**：操作结果是否需要改变客户端当前显示的内容？

不需要→纯执行模式，客户端不等回复（如 `SetUseSaveData`）。  
需要→通过 Replicated 或 RPC 把结果推回去（如攻击判定、道具拾取）。

### 纯执行 vs 需要判定

纯执行（不需要回复）：
```
客户端 → Server_SetUseSaveData(false)
服务器直接改开关，客户端不需要知道结果。
后续服务器关卡加载时自己读这个开关。
```

需要判定（必须通知）：
```
客户端 → Server_Attack(TargetID)
服务器碰撞判定：
  ├─ 命中 → Health -= Damage（Replicated 同步）
  │          Client_PlayHitConfirm()（通知发起者）
  └─ 没中 → Client_ResetToIdle()（拉回预测动画）
```

---

## 8 GameMode 与 PlayerState 的实例分布

### 常见误解

> "每个客户端都有一个在服务器上记录着的 GameMode"

**不对。** 服务器上只有一个 GameMode，客户端上没有 GameMode。

### 实际分布

```
【服务器进程】
  ├── GameMode × 1         ← 全局规则，唯一
  ├── PlayerState_A        ← 玩家 A 的个人数据
  ├── PlayerState_B        ← 玩家 B 的个人数据
  ├── PlayerController_A   ← 管玩家 A
  └── PlayerController_B   ← 管玩家 B

【客户端A进程】
  ├── GameMode → nullptr   ← 没有！客户端拿不到
  ├── PlayerState_A        ← 自己的数据（从服务器复制）
  ├── PlayerState_B        ← 别人的数据（从服务器复制）
  └── PlayerController_A   ← 自己的控制器

【客户端B进程】
  ├── GameMode → nullptr   ← 同样没有
  ├── PlayerState_A        ← 别人的数据
  ├── PlayerState_B        ← 自己的数据
  └── PlayerController_B   ← 自己的控制器
```

### 数据放哪里

| 数据类型 | 放哪里 | 原因 |
|---------|--------|------|
| 关卡切换、AI 生成规则 | GameMode | 全局规则，只需服务器执行 |
| 玩家等级、血量、装备 | PlayerState | 每玩家独立，需复制到所有客户端 |
| 当前输入、按键状态 | PlayerController / Pawn | 每玩家一份，不参与复制 |
| 是否使用存档这类开关 | GameMode | 只有服务器关卡加载逻辑需要读 |

**一句话**：GameMode 管"世界的规则"，PlayerState 管"玩家的状态"。客户端不拿 GameMode 做事——通过 PlayerState/GameState 读数据，通过 Server RPC 发起操作。

---

## 9 服务器权威原则

### 核心思想

```
客户端可以做的事（本地，无需服务器确认）：
  显示 UI、播放本地音效、输入采集、预测动画
  这些不影响游戏规则

服务器才有权做的事（必须 Server RPC）：
  扣血、判定命中、刷怪、切换关卡、记录存档
  这些改变游戏状态
```

### 实际应用对比

| 场景 | 单机思维 | 联机思维 |
|------|---------|---------|
| 点"新游戏" | GetGameMode()->SetUseSaveData() | PC->Server_RequestNewGame() |
| 攻击敌人 | 本地碰撞判定+扣血 | 发 Server RPC，服务器判定 |
| 捡道具 | 本地销毁+加入背包 | 发 Server RPC，服务器执行 |
| 播放开场动画 | 直接调 UMG | 纯本地，不用改 |
| 显示血量 | 从 Character 读 | 从复制的 PlayerState 读 |

### 每次写代码前问自己

```
① 这个操作会影响游戏状态吗？（扣血、生成、删除、切换）
   是 → 必须经过服务器
   否 → 可在本地执行

② 这个数据所有玩家都需要看到吗？
   是 → UPROPERTY(Replicated) 放 PlayerState/GameState
   否 → 本地变量

③ 这个判断客户端自己能决定吗？
   能 → 直接执行（UI 切换、特效）
   不能 → Server RPC 委托服务器
```

### 最终原则

**让服务器做所有涉及游戏规则的决策，客户端只负责展示结果和收集输入。** 这不是优化选项，是联机游戏正确运作的前提。

---

> 文档版本：v1.0
> 最后更新：2026-06-09
> 适用项目：SoulLike (UE 4.26)
