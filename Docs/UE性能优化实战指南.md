# UE4 性能优化实战指南：从工具到实战

> 本文档基于 SoulLikeDemo（UE 4.26.2）项目中"敌人攻击命中玩家卡顿"问题的完整排查过程整理而成，涵盖工具使用、排查流程、踩坑记录和修复方案。
>
> **文档特点：** 每个环节都标注了实战中**犯过的错误**和**踩过的坑**，避免走同样的弯路。

---

## 目录

- [一、性能优化工具箱概览](#一性能优化工具箱概览)
  - [1.1 核心工具](#11-核心工具)
  - [1.2 何时用哪个](#12-何时用哪个)
- [二、Unreal Insights 使用指南](#二unreal-insights-使用指南)
  - [2.1 打开方式](#21-打开方式)
  - [2.2 追踪原理：两种调用方式](#22-追踪原理两种调用方式)
  - [2.3 捕获追踪数据](#23-捕获追踪数据)
  - [2.4 定位流程](#24-定位流程)
  - [2.5 如何读懂 stat dumphitches 输出](#25-如何读懂-stat-dumphitches-输出)
  - [2.6 Unreal Insights 面板导航](#26-unreal-insights-面板导航)
  - [2.7 自定义函数监控](#27-自定义函数监控)
- [三、实战回放：受击卡顿排查全过程](#三实战回放受击卡顿排查全过程)
  - [3.1 问题现象](#31-问题现象)
  - [3.2 stat unit 宏观查看](#32-stat-unit-宏观查看)
  - [3.3 stat dumphitches 抓取](#33-stat-dumphitches-抓取)
  - [3.4 定位根因](#34-定位根因)
  - [3.5 问题定性](#35-问题定性)
- [四、修复方案](#四修复方案)
  - [4.1 立即修复](#41-立即修复)
  - [4.2 验证方法](#42-验证方法)
- [五、踩坑汇总](#五踩坑汇总)
- [六、附录](#六附录)
  - [6.1 关键日志](#61-关键日志)
  - [6.2 常用命令速查](#62-常用命令速查)
  - [6.3 参考文章与外部链接](#63-参考文章与外部链接)
  - [6.4 术语对照](#64-术语对照)

---

## 一、性能优化工具箱概览

### 1.1 核心工具

| 工具 | 用途 | 引入版本 |
|:---|:---|:---|
| **Unreal Insights** | 全维度性能"CT机"，精确到毫秒级的调用链追踪 | UE 4.23+（4.25 增强） |
| **stat dumphitches** | 捕获卡顿帧的完整调用栈统计，定位耗时函数 | UE 内置 |
| **stat unit / stat unitgraph** | 实时查看 Game/Render/GPU 线程耗时 | UE 内置 |
| **stat streaming** | 监控资源流式加载状态 | UE 内置 |

### 1.2 何时用哪个

- **宏观卡顿/帧率波动** → `stat unit` 先看整体瓶颈在哪条线程
- **偶发卡顿（Hitch）** → `stat dumphitches` 抓取卡顿帧详情
- **需要深度函数级分析** → Unreal Insights 抓 `.utrace` 文件
- **怀疑资源加载问题** → `stat streaming` + Insights 的 Asset Loading Insights

---

## 二、Unreal Insights 上手指南

### 2.1 打开方式

直接运行引擎目录下的程序：

```
Engine\Binaries\Win64\UnrealInsights.exe
```

### 2.2 追踪原理：两种调用方式

Unreal Insights 有两种工作模式：

**在线模式：** 游戏/编辑器运行时实时传输追踪数据到 Insights。在游戏启动参数中添加：

```
UE4Editor.exe "Project.uproject" -tracehost=127.0.0.1 -trace=CPU,Frame,File -statnamedevents
```

启动后 Insights 通常会自动识别并连接。如果未自动连接，在 Insights 界面中点击 `Session` 面板 → 输入 `127.0.0.1` → `Connect` 手动连接。

**离线模式：** 游戏运行时不连接 Insights，用控制台命令录制 .utrace 文件，事后拖入 Insights 分析。适合无法连接远程工具的环境。

### 2.3 捕获追踪数据

Unreal Insights 通过 trace 通道系统决定捕获哪些数据。**选错通道会导致数据不足或文件过大**，以下是 UE 4.26 实际可用的通道列表：

| 通道 | 用途 | 你的场景 |
|:---|:---|:---:|
| **CPU** | 捕获所有 CPU 线程的调用栈。没有它，Insights 看不到任何函数执行时间线 | ✅ **必须** |
| **GPU** | 捕获 GPU 渲染管线（DrawCall、材质、后处理等） | ❌ CPU 问题不需要 |
| **Frame** | 标记帧边界，方便在时间线上定位长帧 | ✅ 建议 |
| **File** | 文件 I/O 操作跟踪（读盘、写盘、**资产加载**等） | ✅ **核心需要** |
| **LoadTime** | 关卡加载时间、Level Streaming 相关事件。**开启后可在 Insights 中启用 Asset Loading 面板**，查看每个资产的加载详情 | ✅ 资源加载场景 |
| **Net** | 网络同步数据 | ❌ 不需要 |
| **Log** | 把 UE_LOG 输出也捕获进 .utrace | ⭕ 可选 |
| **Bookmark** | 手动标记点 | ⭕ 辅助用 |

> `stats`、`assetload`、`counters` 不是有效的 trace 通道名，以实际列表为准。

在控制台中使用以下命令开始追踪：

```
trace.start CPU,Frame,File
```

参数含义：
- **CPU,Frame,File** → 开启三个通道：CPU 调用栈 + 帧边界 + 文件 I/O（覆盖资产加载）。



停止追踪：

```
trace.stop                           // 停止并保存 .utrace 文件
```

.utrace 文件保存在 `Saved\Profiling\UnrealInsights\` 目录下。

### 2.4 定位流程

```
宏观审视（Frames面板找长帧）
    ↓
定位瓶颈线程（GPU / GameThread / RenderThread）
    ↓
深度下钻（Timers面板展开调用树）
    ↓
专项排查（资源加载 / 网络同步 / 特定系统）
```


### 2.5 如何读懂 stat dumphitches 输出

`stat dumphitches` 的输出是纯文本格式的性能剖面，第一次接触容易看不懂。按这个顺序读就不会被噪声淹没。

#### 2.7.1 输出结构

一段完整的 dumphitches 输出由三部分组成：

```
① 标题行 → 确认严重程度

② 树状调用栈统计 → 找最贵的那个分支

③ 触发链（Trigger Chain）→ 定位谁发起的
```

#### 2.7.2 第一步：看标题行

```
------------------ Thread Hitch 1, Frame 11604  2403.2ms ---------------
```

这告诉你：**第 1 个卡顿，在第 11604 帧，花了 2403ms**。正常情况下 60fps 一帧是 16.67ms，2403ms = 0.4fps，画面卡住不动了。

#### 2.7.3 第二步：看树状统计（找最贵的）

```
2384.420ms (   2)  -  ObjectLibrary/Engine/Transient.ObjectLibrary    ← 最顶层，资源类型
  └─ 2126.242ms ( 143)  -  LoadObject                               ← 其中 2126ms 花在 LoadObject 上
       ├─ 1349.969ms ( 143)  -  Self                                 ← LoadObject 自身开销
       ├─ 551.563ms (  17)  -  Create Function List                  ← KismetCompiler（蓝图编译）
       ├─ 464.300ms (  17)  -  Process uber                          ← KismetCompiler
       └─ 269.919ms ( 280)  -  StaticDuplicateObject                 ← PIE 复制 CDO
```

**读法：找那个最贵的叶子节点。** 缩进越深，粒度越细。找到最深的、耗时最长的项，就是主要瓶颈方向。

**格式解读：**
- `2384.420ms` → 该统计项的总耗时
- `( 143)` → 该函数被调用了 143 次
- `Self` → 函数自身开销（不含子函数调用）
- `Create Function List`、`Process uber` → 这是 KismetCompiler（蓝图编译器）在运行

**关键信号：** 看到 `KismetCompiler` 或 `StaticDuplicateObject` 出现，说明这是在**编辑器 PIE 中**打包后的游戏不会有这两项。如果打包后测试没有卡顿，问题不大。

#### 2.7.4 第三步：看触发链（从下往上读）

```
Trigger: ... ← LoadObject ← ObjectLibrary ← Loading Library ← PostGameplayEffectExecute
  ← InternalExecuteMod ← ExecuteActiveEffectsFrom ← AbilitySystemComp ApplyGameplayEffectSpecToSelf
  ← PC Process Input Stack ← PC Tick Input ← PlayerTick
  ← PC Tick Actor ← PlayerController Tick ← ALS_Player_Controller_C/...
  ← FTickFunctionTask ← Game TaskGraph Tasks ← FrameTime ← GameThread
```

**从下往上读**——下层是原因，上层是结果：

| 方向 | 内容 | 含义 |
|:---:|:---|:---|
| 最底 | `PlayerController Tick` + `ALS_Player_Controller_C` | 当前在 PlayerController 的 tick 中 |
| ↑ | `PC Tick Input` → `PC Process Input Stack` | 正在处理玩家输入 |
| ↑ | `ApplyGameplayEffectSpecToSelf` | 输入触发了 GAS，给自己施加了一个 GE |
| ↑ | `PostGameplayEffectExecute` | GE 执行完成后触发了回调 |
| ↑ | `Loading Library` → `LoadObject` | 回调里触发了资源加载 |
| 最顶 | `EnqueueUniqueRenderCommand` | 加载中跨越了线程边界 |

#### 2.7.5 哪些是噪声，可以跳过

在 dumphitches 输出中，这些信息在第一次分析时**可以直接忽略**：

1. **`Duration: Xms` + `Wait: RenderThread` 的小条目**——它们是大卡顿帧内部的微等待事件，Xms 相对于主卡顿（上千 ms）没有分析价值
2. **`There is no thread with id: XXXXX` 的警告**——引擎内部线程 ID 映射问题，与你无关
3. **`OtherChildren`**——聚合统计中的"其他"分类，粒度不够细，先看有名字的项

#### 2.7.6 总结：读数据的三步法

```
1. 看标题 → 确认严重程度（Xms 是否异常）
2. 看树状统计 → 找最贵的那个分支（哪个函数吃了最多时间）
3. 看触发链 → 从下往上读，定位谁发起的
```

> **核心原则：** 不要在第一遍读数据时被细节（6ms 条目、线程警告）吸引走注意力。
> 先宏观找最贵的，再微观定位根因。

---

### 2.6 Unreal Insights 面板导航：从长帧到具体资产

拿到 .utrace 文件后，在 Insights 中找到根因资产的操作顺序：

#### 2.7.1 第一步：Frames 面板找长帧

打开 .utrace 后，左上角的 **Frames 面板**显示每帧耗时，每根竖条代表一帧。正常帧是细线（16ms），卡顿帧是一根巨柱。点击选中它。

#### 2.7.2 第二步：Timers 面板看调用树

左下角的 **Timers 面板**显示按耗时从高到低的函数调用树。展开 LoadObject 等顶层项，看子节点中有没有 LoadPackageInternal。如果有，展开看 PackageName 字段。

#### 2.7.3 第三步：Timing View 看时序

中间的 **Timing View** 显示线程时间线。展开 GameThread 行，找到最长的那根耗时条，鼠标悬停看 Tooltip 中的 Object: 或 PackageName:。

#### 2.7.4 第四步：Asset Loading 面板找请求者

在左侧标签栏找到 **Asset Loading** 面板（如果录入了 File 通道）。其中有一个表格，按 Load Time 降序排序后，排第一的包就是单次加载最贵的。它的 Requested By 列显示谁触发了这次加载。

#### 2.7.5 第五步：Callers / Callees 面板验证因果关系

在 Timers 中选中某一项后，**Callers 面板**（通常在界面右侧或底部）显示它的调用链。**从下往上读**——下层是原因，上层是结果。**Callees 面板**显示它内部调用了什么。

#### 2.7.6 各面板用途速查

| 面板 | 用途 | 数据来源 |
|:---|:---|:---|
| Frames | 找长帧，宏观确认卡顿位置 | CPU 通道 |
| Timers | 函数级调用树，按耗时排序——找"什么函数最贵" | -statnamedevents 启动参数 |
| Timing View | 线程时间线，时序关系——看"谁先谁后" | CPU 通道 |
| Asset Loading | 包加载详情——看"具体哪个资产被加载" | File 通道 |
| Callers / Callees | 调用/被调用链——验证因果关系 | 基于 Timers 选中项 |

### 2.7 为什么看不到自己的函数 + 如何让代码出现在调用链中

#### 2.7.1 stat named events 的局限

Insights 的 Timers 面板和 Timing View 中，默认只显示**有 SCOPE_CYCLE_COUNTER 宏标记的函数**。引擎函数（如 LoadObject、PostGameplayEffectExecute）都有这个宏，但你的自定义函数（如 HandleInputPressed、ActivateAbility）没有加，所以不会出现。

这不是 bug，是 UE4 stat 系统的设计：**只有你声明了要统计的函数，它才会出现**，避免无关函数淹没有用信息。

#### 2.7.2 两种数据类型对比

- **Stat Named Events**：只显示加了宏的函数，精确计时（纳秒级），开销低。通过 -statnamedevents 或 stat namedevents 开启
- **CPU Sampling（cputrace）**：显示完整的 CPU 调用栈（包括你的自定义函数），采样近似（毫秒级），开销较高。通过 -cputrace 或 trace.start ... cputrace 开启

两者互补，不是互斥。同时开启后 Insights 中既有精确计时数据，也有完整调用栈。

#### 2.7.3 方法一：加 SCOPE_CYCLE_COUNTER 宏（推荐）

在你想监控的函数体第一行加入：

```cpp
void USL_ComboManagerComponent::HandleInputPressed(EComboInputActionType InputType)
{
    SCOPE_CYCLE_COUNTER(STAT_Combo_HandleInput);   // ← 加这一行
    // 原有逻辑...
}
```

如果编译器提示找不到 STAT_Combo_HandleInput，需要在 cpp 文件顶部声明：

```cpp
#include "Stats/Stats.h"
DECLARE_CYCLE_STAT(TEXT("HandleInputPressed"), STAT_Combo_HandleInput, STATGROUP_Game);
```

三要素：
- TEXT("HandleInputPressed") → Insights 中显示的名字
- STAT_Combo_HandleInput → 代码中引用的标识符
- STATGROUP_Game → 所属 stat 分组

#### 2.7.4 方法二：用 SCOPED_NAMED_EVENT（轻量方案）

```cpp
{
    SCOPED_NAMED_EVENT(TEXT("Combo_HandleInput"), FColor::Green);
    // 你的逻辑
}
```

优点：不需要提前 DECLARE，自动出现在 CPU profiling 调用栈中。
缺点：精度不如 SCOPE_CYCLE_COUNTER，且只出现在 CPU Sampling 数据中。

---

## 三、实战回放：受击卡顿 1.77 秒排查全过程

### 3.1 问题现象

**敌人攻击命中玩家 → 约 1 秒严重卡顿**

初步直觉怀疑方向：
- ❌ 碰撞检测太复杂
- ❌ AI 决策开销大
- ❌ 物理模拟太重


### 3.2 第一步：stat unit 宏观查看

```
stat unit
```

观察到卡顿时 Game 线程耗时骤升，Render/GPU 正常。
确定瓶颈在 **GameThread**，需要进一步深挖。

### 3.3 第二步：stat dumphitches 抓取卡顿详情（关键）

```
stat dumphitches
```

捕获到关键日志（见附录 6.1），核心调用链：

```
FTimerManager
  └── SL_WeaponBase
       └── AbilitySystemComp::ApplyGameplayEffectSpecToSelf
            └── PostGameplayEffectExecute
                 └── Loading Library (1757ms)
                      └── LoadObject (1507ms)
                           ├── Create Function List (574ms)
                           └── StaticDuplicateObject (271ms)
```

> ✅ **这是关键转折点。** `stat dumphitches` 直接定位到了根因。


### 3.4 第三步：定位根因

卡顿的直接原因是 **`LoadObject`**（1507ms）触发了同步资源加载。

| 触发链路 | 说明 |
|:---|:---|
| 敌人攻击命中 → GAS ApplyGameplayEffect | 触发伤害处理 |
| PostGameplayEffectExecute → Loading Library | 伤害处理中引用了未加载的资源 |
| LoadObject → ALS_AnimBP | 首次同步加载并实时编译动画蓝图 |
| **结果：主线程阻塞 1.77 秒** | **卡顿** |

加载的具体对象：`/Game/AdvancedLocomotionV4/CharacterAssets/MannequinSkeleton/ALS_AnimBP`


### 3.5 问题定性

- ❌ **不是** 代码逻辑问题（碰撞、AI、物理都没问题）
- ❌ **不是** 硬件性能不足
- ✅ **是** 资源管理问题：核心资产在战斗关键时刻**按需同步加载**

---

## 四、修复方案

### 4.1 立即修复（推荐）

将资源的**同步加载**改为**提前异步预加载**：

**方案 A：在角色 BeginPlay 中异步加载**
```cpp
// 使用 FStreamableManager 或 LoadPackageAsync 提前加载
UAssetManager::GetStreamableManager().RequestAsyncLoad(
    TSoftObjectPtr<UBlueprintGeneratedClass>(FSoftObjectPath(TEXT("/Game/.../ALS_AnimBP.ALS_AnimBP"))),
    FStreamableDelegate::CreateLambda([]() {
        // 加载完成回调
    })
);
```

**方案 B：通过 Asset Manager 配置预加载**

在 `Project Settings` → `Asset Manager` 中将被依赖的核心资产加入 Primary Asset Labels，让引擎在启动时主动加载。

**方案 C：配置 Use Object Library**

在 `Project Settings` → `Gameplay Abilities` → `Ability System Global` 中启用 `Use Object Library`，配置加载路径以减少 GAS 相关蓝图的运行时编译开销。

### 4.2 验证方法

修复后运行以下命令确认：
```
stat dumphitches
```
观察 `LoadObject` 和 `Create Function List` 的耗时是否大幅降低或消失。

---

## 五、踩坑汇总

以下为本次排查过程中记录的所有踩坑记录：

| #  | 错误 | 后果 | 正确做法 |
|:--:|:---|:---|:---|
| 1 | 在 `Programs` 目录下找 UnrealInsights | 找不到程序，浪费时间 | 路径是 `Engine\Binaries\Win64\` |
| 2 | 只用默认通道抓取 | 数据颗粒度不够，看不到函数名 | 加参数 `-trace=CPU,Frame,File -statnamedevents` |
| 3 | 被 Timers 面板的零耗时项淹没 | 不知道从哪看起 | 点击 `!0` 过滤 |
| 4 | 凭经验猜测瓶颈（碰撞/物理） | 方向错误，浪费精力 | 先上工具看数据，不要猜 |
| 5 | 用 Insights 抓偶发卡顿 | 多次抓不到 | 偶发卡顿用 `stat dumphitches` |
| 6 | 看到 LoadPackageInternal 以为是资源太大 | 差点走偏去优化资源大小 | 问题不是资源大小，是**加载时机** |

### 核心教训

> **性能优化的第一原则：不要猜，上工具。**
>
> **偶发卡顿用 `stat dumphitches`，持续性能问题用 Unreal Insights。**
>
> **分清"什么慢"和"为什么现在慢"。**

---

## 六、附录

### 6.1 关键日志 — stat dumphitches 捕获的卡顿帧

```
LogStats: ------------------Thread Hitch 1, Frame 2491  1768.9ms ---------------
LogStats:   1767.970ms (   6)  -  Thread_2184_0 - GameThread
LogStats:     1767.964ms (   2)  -  FrameTime
LogStats:       1761.103ms (   1)  -  FrameTime
LogStats:         1761.047ms (   1)  -  World Tick Time
LogStats:           1759.390ms (   1)  -  GT Tickable Time
LogStats:             1759.376ms (   1)  -  FTimerManager
LogStats:               1759.358ms (   1)  -  SL_WeaponBase
LogStats:                 1758.923ms (   1)  -  AbilitySystemComp ApplyGameplayEffectSpecToSelf
LogStats:                   1758.892ms (   1)  -  AbilitySystemComp ExecuteGameplayEffect
LogStats:                     1758.890ms (   1)  -  ExecuteActiveEffectsFrom
LogStats:                       1758.707ms (   1)  -  InternalExecuteMod
LogStats:                         1758.598ms (   1)  -  PostGameplayEffectExecute
LogStats:                           1757.673ms (   2)  -  Loading Library
LogStats:                             1757.631ms (   2)  -  ObjectLibrary/Engine/Transient.ObjectLibrary
LogStats:                               1507.994ms ( 138)  -  LoadObject
LogStats:                                 738.781ms ( 138)  -  Self
LogStats:                                 574.160ms (  17)  -  Create Function List
LogStats:                                   478.516ms (  17)  -  Process uber
LogStats:                                     271.380ms ( 280)  -  STAT_StaticDuplicateObject
```

### 6.2 常用命令速查

**宏观监控**
| 命令 | 用途 |
|:---|:---|
| `stat unit` | 实时查看 Game/Render/GPU 三线程耗时 |
| `stat unitgraph` | 图形化显示各线程耗时曲线 |
| `stat streaming` | 监控资源流式加载状态 |
| `stat dumphitches` | 捕获卡顿帧的详细调用栈统计（自动触发，阈值默认 300ms） |
| `stat dumphitches -threshold=50` | 设置卡顿捕获阈值为 50ms，抓更微小的卡顿 |
| `stat namedevents` | 开启 stat 命名事件（运行时启用，与 -statnamedevents 启动参数效果相同） |

**Unreal Insights 追踪**
| 命令 / 启动参数 | 用途 |
|:---|:---|
| `UE4Editor.exe ... -trace=CPU,Frame,File -statnamedevents` | 启动时带完整通道 |
| `trace.start CPU,Frame,File` | 控制台开始追踪 |
| `trace.stop` | 停止并保存 .utrace 文件 |
| `UE4Editor.exe ... -tracehost=127.0.0.1 -trace=CPU,Frame,File -statnamedevents` | 启动时连接 Insights 在线分析 |
| `UE4Editor.exe ... -cputrace` | 开启 CPU 采样（可在 Timers/Timing View 中看到完整的 CPU 调用栈） |

**GAS 调试**
| 命令 | 用途 |
|:---|:---|
| `log LogAbilitySystem Verbose` | 开启 GAS 详细日志，查看 GE/Cue 触发记录 |
| `log LogGameplayCueManager Verbose` | 开启 GameplayCue 日志 |
| `showdebug abilitysystem` | 画面左上角叠加 GAS 实时信息（Ability/Effects/Tags） |

### 6.3 参考文章与外部链接

| # | 标题 | 链接 | 说明 |
|:---:|:---|:---|:---|
| 1 | Unreal Insights Android/PC 配置 | [cnblogs.com](https://www.cnblogs.com/Shaojunping/p/16694711.html) | 安卓和 PC 端的 Insights 配置指南 |
| 2 | SCOPE_CYCLE_COUNTER 性能统计宏 | [zhihu.com](https://www.zhihu.com/search?type=content&q=SCOPE_CYCLE_COUNTER) | 性能统计宏相关文章汇总 |
| 3 | UE4 Profiler 性能分析工具原理和实现机制 | [zhuanlan.zhihu.com](https://zhuanlan.zhihu.com/p/416863993) | Profiler 底层原理 |
| 4 | 基于 Stat 和 Trace 扩展代码性能统计 | [zhuanlan.zhihu.com](https://zhuanlan.zhihu.com/p/716644594) | Stat + Trace 扩展实践 |

### 6.4 Unreal Insights 术语对照

| 中文 | 英文 | 说明 |
|:---|:---|:---|
| 时序洞察 | Timing Insights | 主界面，显示各线程时间线 |
| 帧面板 | Frames Panel | 显示每帧耗时，找长帧用 |
| 计时器面板 | Timers Panel | 调用树统计，按耗时排序 |
| 资源加载洞察 | Asset Loading Insights | 针对资源加载卡顿 |
| 网络洞察 | Networking Insights | 针对网络同步问题 |

---

> **文档编制日期：** 2026-06-04（持续更新中）
> **相关项目：** SoulLikeDemo (UE 4.26.2) / GAS + ALSv4 + UnLua
> **原始记录来源：** DeepSeek Web 端对话记录
