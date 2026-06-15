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

## 10 权威性数据分布：按 UE 类层次分解

> 本节从 UE 主体类的高层到底层，逐一说明哪些数据需要建立权威性、谁拥有最终写入权、以及为什么这么设计。

### 10.1 UWorld

**权威主体**：服务器（唯一权威）

**需要权威性的数据**：
- 所有 Actor 的生成与销毁（`SpawnActor` / `DestroyActor` 仅在服务器调用才有效）
- 关卡流加载（`LoadStreamLevel` / `UnloadStreamLevel`）
- 世界时间（`TimeSeconds`、`DeltaTime`）
- 世界状态标签（如 `MatchStarted` 等全局状态）

**原因**：UWorld 是所有 Actor 的容器。如果客户端能生成 Actor，就会出现"客户端凭空造怪"的作弊。服务器是唯一能决定"这个世界里有什么东西在"的节点。

### 10.2 UGameInstance

**权威主体**：混合
- **跨关卡持久化数据**（玩家存档、匹配队列状态）：服务器权威
- **本地设置**（画质、音频）：客户端本地权威

**需要权威性的数据**：
- 玩家在线 ID 和身份令牌
- 跨关卡保留的游戏进度
- 房间 / 匹配状态

**原因**：UGameInstance 跨越关卡存活，服务器的 GameInstance 持有"这个服务器上到底在发生什么"的全局权威状态。客户端的 GameInstance 只持有自己的本地缓存。身份认证类的数据必须在服务器端验证，不能信任客户端上报的 ID。

### 10.3 AGameModeBase / AGameMode

**权威主体**：仅服务器（客户端不存在此实例）

**需要权威性的数据**：
- 玩家生成规则（`ChoosePlayerStart`、`RestartPlayer`）
- 游戏阶段转换（等待 → 进行中 → 结束）
- 计分规则、胜利条件判定
- 是否允许重生、重生延迟
- 禁止行为的判定

**原因**：GameMode 是"游戏规则"的化身。它的权威性是绝对的——客户端**根本没有这个类的实例**，意味着玩家完全无法从本地篡改游戏规则。任何"可以做什么、不可以做什么"的逻辑必须放这里。

### 10.4 AGameStateBase / AGameState

**权威主体**：服务器（自动复制到所有客户端）

**需要权威性的数据**：
- 当前游戏阶段（`MatchState`：EnteringMap → WaitingToStart → InProgress → WaitingPostMatch → LeavingMap）
- 所有 PlayerState 数组（`PlayerArray`）
- 倒计时（`RemainingTime`）
- 团队得分、全局排行榜

**原因**：GameState 是"所有人都必须看到一致的全局状态"。如果客户端的 GameState 和服务器不一致（比如一个客户端认为比赛还在进行而服务器认为结束了），就会导致不同步。UE 通过自动属性复制（`Replicated`）保证了服务器→所有客户端的单向权威流。

### 10.5 APlayerController

**权威主体**：层级化权威——分两层

**需要权威性的数据**：

**（A）客户端→服务器的 RPC 数据**
- 输入动作（`ServerMove`、`ServerRPC` 等）
- 交互操作（打开门、拾取物品）
- UI 操作触发的游戏逻辑

**（B）服务器→客户端的复制数据**（`Replicated` 属性）
- 当前控制的 Pawn 引用
- 摄像机视角参数
- 该玩家特有的 HUD 提示状态
- 观战状态

**权威划分原则**：
- 客户端对自己的输入有**提议权威**——你按下按键，你的 PC 发送 `ServerMove` 提议移动，但服务器会验证这个提议是否合法（比如是否在移动速度限制内、是否穿墙）
- 服务器对交互结果有**最终权威**——你客户端说"我攻击了那个敌人"，服务器验证后才扣血
- 客户端对自己的 PlayerController 状态有**本地权威**（玩家名称、UI 偏好），但这些如果影响游戏公平性（比如改名为冒充他人），就必须由服务器验证后通过复制覆盖

### 10.6 APlayerState

**权威主体**：服务器（属性自动复制到所有客户端，包括拥有者客户端和其他客户端）

**需要权威性的数据**：
- `PlayerName`（UE 原生：`ReplicatedUsing=OnRep_PlayerName`）
- `PlayerId`（唯一标识）
- `Score`、`Kills`、`Deaths`
- 队伍索引（`TeamId`）
- 自定义竞技状态（如"是否处于无敌状态"、"拥有哪些 buff"）

**原因**：PlayerState 是"这个玩家在游戏世界的公开身份和状态"。一个玩家可以试图将自己的 `Score` 改成 9999——但服务器会在 `OnRep_Score` 上直接覆盖回去。即便是拥有者客户端，也对 `Score` 没有写权威。只有服务器能改。

### 10.7 APawn / ACharacter

**权威主体**：服务器（但有客户端预测机制）

**需要权威性的数据**：

**（A）移动数据**（UCharacterMovementComponent 自动处理）
- `ReplicatedMovement`（位置、旋转、速度、跳跃状态）
- 服务器端每帧收到 `ServerMove` 提议，运行移动模拟，然后比较客户端预测 vs 服务器结果，如果偏差超过阈值则发送 `Correction` 纠正客户端
- 这个机制叫做**客户端预测 + 服务器回滚验证**——客户端先动再说，但服务器保留最终裁量权

**（B）游戏状态数据**
- 当前生命值（`Health`）
- 当前体力值（`Stamina`）
- 死亡 / 存活状态
- 装备的武器
- Buff / Debuff 列表

**（C）动画状态**
- 如果有纯客户端动画预测，不影响游戏逻辑的（如飘带物理），可以客户端本地权威
- 影响游戏判定的动画状态（攻击命中窗口），必须服务器权威

**权威划分原则**：
- 视觉 / 手感层面：客户端可以"先跑起来"（预测），服务器后验证
- 数值 / 判定层面：**服务器最终说了算**。客户端可以预测自己在跑，但服务器说"你已经被别人打死了"，客户端就必须停
- 移动是 UE 网络模型最复杂的地方，UCharacterMovementComponent 本身实现了完整的权威验证机制，不要去自己重写移动验证

### 10.8 AController（含 AIController）

**权威主体**：服务器独占

**需要权威性的数据**：
- AIController 的 `BrainComponent`（行为树状态、黑板值）
- `MoveTo` 目标位置
- 感知数据（`AIPerceptionComponent` 检测到的敌人）
- 攻击决策（"选择攻击谁、用哪一招"）

**原因**：AIController **只在服务器上存在**，客户端根本没有这个实例。这意味着 AI 的所有决策天然具有服务器权威，玩家无法篡改 AI 行为。这是 UE 网络架构设计中最优雅的地方之一——AI 逻辑零额外防护成本。

### 10.9 AActor / UActorComponent（通用层）

**权威主体**：服务器（除非标记为 `bReplicates=false` 的纯视觉 Actor）

**需要权威性的数据**：
- 所有标记为 `Replicated` 的 UPROPERTY
- 命中的伤害值（`TakeDamage` 必须在服务器调用，或从客户端 `ServerRPC` 报告后由服务器执行）
- 触发器状态（开关门、按钮机关）
- 投射物（子弹、火球）的轨迹和碰撞——服务器权威的 `ReplicatedMovement`
- 物品掉落和拾取状态

**权威原则**：
- `RPC` 三类型决定了权威流向：
  - `Server` —— 客户端→服务器（提议权威，服务器验证）
  - `Client` —— 服务器→特定客户端（通知权威）
  - `NetMulticast` —— 服务器→所有客户端（广播权威）
- 任何影响游戏结果的逻辑，只在服务器上执行 `Replicated` 属性到客户端

### 10.10 总结：权威性层次分布

| 层级 | 类 | 权威类型 | 关键原则 |
|------|------|---------|---------|
| **规则层** | UWorld / AGameModeBase | 服务器独占 | 规则不可见不可篡改 |
| **全局状态层** | AGameStateBase / APlayerState | 服务器权威→复制 | 所有人必须看到一致的全局状态 |
| **交互层** | APlayerController | 客户端提议→服务器验证 | 输入客户端提交，结果服务器决定 |
| **实体层** | APawn / ACharacter / AActor | 服务器权威+客户端预测 | 先动后验，服务器保留裁量权 |
| **逻辑层** | AIController | 服务器独占 | AI 决策天然不可篡改 |
| **实例层** | UActorComponent | 服务器权威→复制 | 任何 `Replicated` 属性最终写入权在服务器 |

一句话总结：**客户端可以"提建议"，但服务器是唯一能"做决定"的节点。** 任何需要一致性的数据，写入权必须在服务器；客户端只做两件事——提议（RPC）和接收（Replicated Properties）。

---

> 文档版本：v1.0
> 文档版本：v2.0
> 文档版本：v2.1
> 最后更新：2026-06-15
> 适用项目：SoulLike (UE 4.26)

---

## 11 连接建立与消息路由底层机制

> 本节补充 UE4 网络骨架层的细节：连接如何建立、标识体系如何分层、消息如何路由到正确的 Actor。

### 11.1 连接建立过程

一个客户端连入 ListenServer 时的完整步骤：

```
客户端                                 ListenServer
  │                                         │
  │── UDP（连接请求）─────────────────────→│
  │   来源 IP:Port（OS 随机分配高位端口）     │
  │                                         │  new UNetConnection
  │                                         │  存入 TMap<FInternetAddr, UNetConnection*>
  │←── UDP（接受连接）─────────────────────│
  │                                         │
  │  双方各自在内存中持有 UNetConnection     │
  │  （不通过网络传输，是纯内存对象）         │
```

**关键约束：**

- UE4 默认网络传输走 **UDP**，不是 TCP。可靠性（Ack/重传/排序）由引擎的 PacketHandler 层保证
- 连接标识是 UDP 包自带的**来源 IP:Port**，不是 NetConnection 对象本身。双方的 NetConnection 内存指针从来不通过网络传输
- 每个客户端在服务器上有一个唯一的 UNetConnection 实例

### 11.2 三层标识体系

UE4 用三个层级的标识符来路由消息，各自解决不同的问题：

```
┌──────────────────────────────────────────┐
│         UDP 包头                          │
│  来源 IP:Port                             │  ← 找到 UNetConnection
├──────────────────────────────────────────┤
│         UE4 协议包头                      │
│  ChannelId → 找到 UActorChannel           │  ← 哪个复制通道
│  PacketId   → 排序 / Ack                  │
├──────────────────────────────────────────┤
│         RPC / 属性载荷                    │
│  ActorGUID → 找到 AActor*                │  ← 操作哪个游戏对象
│  函数名 + 参数                            │
└──────────────────────────────────────────┘
```

#### 11.2.1 IP:Port（传输层）

- 解决**"谁发的这个包"**问题
- 服务器 recvfrom() 拿到来源地址 → 查 TMap<FInternetAddr, UNetConnection*> → 找到对应的连接
- 不是用来查 PlayerId 的键。PlayerId 是 NetConnection 上挂载的属性

#### 11.2.2 FNetworkGUID（Actor 层）

- 解决**"这个包操作哪个 Actor"**问题
- 是一个内部自增的 uint32（不是 FGuid 的 UUID 格式），作为 Actor 的网络句柄
- **仅服务器有权分配**。服务器 Spawn Actor 时分配，纳入自己的 GuidCache。客户端收到 Spawn 包时存到自己的 GuidCache
- 映射关系：TMap<FNetworkGUID, AActor*>（服务器和客户端各自维护）

所以 IP:Port 不能替代 ActorGUID——IP:Port 找到"谁在发信"，ActorGUID 找到"信里说的是哪个对象"。一个客户端可以产生多个 RPC 操作不同的 Actor（自己的 Pawn、目标怪物、掉落物等），必须靠 ActorGUID 区分。

#### 11.2.3 PacketId 与 ChannelId

| 标识 | 权威方 | 用途 |
|------|--------|------|
| PacketId | **双方各自独立** | 发送计数器。A 发的包用 A 的序号，B 用 B 的。互不干扰，只用于 Ack 和排序 |
| ChannelId | **服务器** | 服务器决定对哪些 Actor 开复制通道，分配递增的 ID。客户端被动接受 |

### 11.3 消息路由全流程

以灵体玩家 A 在 B 世界中按下攻击键为例：

```
① A 的键盘输入
      ↓
② A 的 PlayerController（在 B 的进程中，作为远程客户端）
   → OnLightAttackPressed()
   → ComboManager / GAS 决定执行哪个能力
      ↓
③ 调用 Server RPC
   → UFUNCTION(Server, Reliable)  Server_DoLightAttack()
   → 序列化：ActorGUID + 函数签名 + 参数
      ↓
④ UDP 打包发送
   → A 的 UNetConnection（指向 B 的地址）
   → 包头含来源 IP:Port、PacketId、ChannelId
      ↓
⑤ B 的 ListenServer 收到 UDP 包
   → recvfrom() → IP:Port
   → 查 TMap → 找到 A 的 UNetConnection
   → 拿出 A 的 PlayerController / PlayerId
      ↓
⑥ 反序列化 RPC 载荷
   → ActorGUID → 查 GuidCache → Phantom_A
   → 在 Phantom_A 上执行 Server_DoLightAttack()
      ↓
⑦ 服务器权威执行
   → 播放 Montage（Multicast RPC → 所有客户端）
   → 碰撞检测 → 伤害计算
   → 属性复制（位置、血量等自动推送给各客户端）
```

### 11.4 移动同步（特殊通道）

移动不走普通 RPC，走 **UCharacterMovementComponent** 自己的专用通道——客户端本地预测 + 服务器权威校正：

```
A 的客户端每帧
  → PlayerController::PlayerTick()
  → AddMovementInput()
  → CharacterMovementComponent 计算期望移动（本地预测）
  → A 的客户端先动起来（低延迟反馈）
  → 打包 FSavedMove_Character（方向、时间戳）
  → 每 ~30-60ms 调用 ServerMove() RPC

B 的服务器
  → 权威验证移动是否合法（防加速外挂）
  → 应用移动
  → 如有偏差，回复 ClientAdjustPosition()
  → 位置通过属性复制推给所有客户端
```

这是 UE4 网络最巧妙的设计之一：在保持手感的同时防止作弊。

### 11.5 标识符对照表

| 标识符 | 谁生成 | 客户端知道吗 | 作用域 | 用途 |
|--------|--------|------------|--------|------|
| IP:Port | OS 分配 | 知道 | 传输层 | UDP 路由到 NetConnection |
| FUniqueNetId | 服务器 Login() | 知道 | 逻辑层 | 玩家身份（名字、等级、数据） |
| FNetworkGUID | 服务器 SpawnActor() | 知道 | Actor 层 | Actor 网络句柄，查 AActor* |
| UNetConnection* | 双方各自 new | 只知道自己这端 | 连接层 | 内存对象，存对方地址和通道列表 |
| ChannelId | 服务器 | 被动接受 | 通道层 | 标识 UActorChannel |
| PacketId | 双方各自计数 | 知道 | 传输层 | 包序号，可靠性保障 |