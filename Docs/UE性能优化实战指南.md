# UE4 性能优化实战指南：从工具到实战

> 本文档基于 SoulLikeDemo（UE 4.26.2）项目中"敌人攻击命中玩家卡顿"问题的完整排查过程整理而成，涵盖工具使用、排查流程、踩坑记录和修复方案。
>
> **文档特点：** 每个环节都标注了实战中**犯过的错误**和**踩过的坑**，避免走同样的弯路。

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

**方法一：编辑器菜单（最推荐）**

`Tools（工具）` → `Unreal Insights` → `Run Unreal Insights`

**方法二：直接运行程序**

引擎目录下：
```
Engine\Binaries\Win64\UnrealInsights.exe
```

> ❌ **踩坑记录 #1：找错路径**
>
> 一开始在 `Engine\Programs` 目录下找 UnrealInsights，找了半天没找到。
> 正确路径是 `Engine\Binaries\Win64\`，不是 `Programs`。

**方法三：从 VS 编译**

在 Visual Studio 解决方案中找到 `UnrealInsights` 项目，单独编译即可。

### 2.2 捕获追踪数据

在游戏/编辑器中打开控制台，使用以下命令：

```
trace.start                          // 开始追踪（默认通道）
trace.start -trace=cpu,gpu,stats     // 按需开启指定通道
trace.stop                           // 停止并保存 .utrace 文件
```

> ❌ **踩坑记录 #2：默认通道不够细**
>
> 第一次抓取时只用 `trace.start`，结果 Insights 里看不到具体的函数名，
> 只能看到 `GameThread` 占了多少时间，不知道具体是哪个函数耗时长。
>
> **修正：** 启动游戏时加参数 `-trace=default,cpu,gpu,stats -StatNamedEvents`，
> 才能展开调用树看到具体的函数名。

### 2.3 定位流程

```
宏观审视（Frames面板找长帧）
    ↓
定位瓶颈线程（GPU / GameThread / RenderThread）
    ↓
深度下钻（Timers面板展开调用树）
    ↓
专项排查（资源加载 / 网络同步 / 特定系统）
```


## 三、实战回放：受击卡顿 1.77 秒排查全过程

### 3.1 问题现象

**敌人攻击命中玩家 → 约 1 秒严重卡顿**

初步直觉怀疑方向：
- ❌ 碰撞检测太复杂
- ❌ AI 决策开销大
- ❌ 物理模拟太重

> ❌ **踩坑记录 #4：凭经验猜瓶颈**
>
> 一开始凭经验怀疑是碰撞或物理的问题，花了不少时间看相关代码，
> 其实方向完全错了。**永远先上工具看数据，不要先猜。**

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

> ❌ **踩坑记录 #5：Insights 抓了多次没抓到卡顿**
>
> 用 Unreal Insights 抓了几次 .utrace 文件，但因为卡顿是偶发的，
> 每次抓到的都是正常帧，没有捕获到卡顿帧。
>
> **为什么 `stat dumphitches` 更好：** 它会在卡顿发生时自动记录，
> 不用在时间轴上翻找，适合排查偶发卡顿。
>
> **教训：偶发卡顿用 `stat dumphitches`，持续性能问题用 Insights。**

### 3.4 第三步：定位根因

卡顿的直接原因是 **`LoadObject`**（1507ms）触发了同步资源加载。

| 触发链路 | 说明 |
|:---|:---|
| 敌人攻击命中 → GAS ApplyGameplayEffect | 触发伤害处理 |
| PostGameplayEffectExecute → Loading Library | 伤害处理中引用了未加载的资源 |
| LoadObject → ALS_AnimBP | 首次同步加载并实时编译动画蓝图 |
| **结果：主线程阻塞 1.77 秒** | **卡顿** |

加载的具体对象：`/Game/AdvancedLocomotionV4/CharacterAssets/MannequinSkeleton/ALS_AnimBP`

> ❌ **踩坑记录 #6：Insights 定位到 LoadPackageInternal 后差点又走偏**
>
> 看到 `LoadPackageInternal` 耗时 951ms，第一反应是"资源太大"，
> 差点去优化 ALS_AnimBP 的大小和贴图分辨率。
> 但实际上问题不是资源大小，而是**加载时机**——在受击瞬间做同步加载。
>
> **教训：分清"什么东西慢"和"为什么现在慢"。**
> 资源大小合理，错在它不该在战斗关键路径上同步加载。

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

以下按时间顺序列出本次排查中犯过的错误：

| #  | 错误 | 后果 | 正确做法 |
|:--:|:---|:---|:---|
| 1 | 在 `Programs` 目录下找 UnrealInsights | 找不到程序，浪费时间 | 路径是 `Engine\Binaries\Win64\` |
| 2 | 只用默认通道抓取 | 数据颗粒度不够，看不到函数名 | 加参数 `-trace=default,cpu,gpu,stats -StatNamedEvents` |
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

| 命令 | 用途 |
|:---|:---|
| `stat unit` | 实时查看 Game/Render/GPU 耗时 |
| `stat unitgraph` | 图形化显示各线程耗时曲线 |
| `stat dumphitches` | 捕获卡顿帧的详细调用栈统计 |
| `stat streaming` | 监控资源流式加载状态 |
| `trace.start` | 开始 Unreal Insights 追踪 |
| `trace.stop` | 停止追踪并保存 |
| `trace.start -trace=cpu,gpu,stats` | 带详细通道的追踪 |

### 6.3 Unreal Insights 术语对照

| 中文 | 英文 | 说明 |
|:---|:---|:---|
| 时序洞察 | Timing Insights | 主界面，显示各线程时间线 |
| 帧面板 | Frames Panel | 显示每帧耗时，找长帧用 |
| 计时器面板 | Timers Panel | 调用树统计，按耗时排序 |
| 资源加载洞察 | Asset Loading Insights | 针对资源加载卡顿 |
| 网络洞察 | Networking Insights | 针对网络同步问题 |

---

> **文档编制日期：** 2026-05-24
> **相关项目：** SoulLikeDemo (UE 4.26.2) / GAS + ALSv4 + UnLua
> **原始记录来源：** DeepSeek Web 端对话记录
