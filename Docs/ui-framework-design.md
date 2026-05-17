# SoulLikeDemo UI 框架设计方案

> 项目：SoulLikeDemo（UE 4.26 C++）
> 阶段：架构设计讨论稿
> 当前基础设施：UUIManagerSubsystem（GameInstanceSubsystem）、UGlobalDelegatesManager

---

## 一、当前 UI 架构现状

### 1.1 UIManagerSubsystem — 生命周期管理中枢

`UUIManagerSubsystem` 作为 `UGameInstanceSubsystem`，统一管理所有 UI 的注册、创建、显示、隐藏、销毁。

**核心能力：**

- **注册/注销**：`RegisterWidget(EWidgetType, TSubclassOf<UUserWidget>)` — 将蓝图类与枚举类型绑定
- **打开/关闭**：`OpenWidget(FUICreateParams)` / `CloseWidget(EWidgetType)` — 支持屏幕空间和世界空间两种模式
- **页面栈**：`PushWidget` / `PopWidget` — 管理页面导航和返回逻辑
- **焦点管理**：`SetFocusToWidget` / `GetFocusedWidgetName`
- **创建参数**：`FUICreateParams` 支持屏幕 UI 和世界空间 UI（跟随 Actor），含距离裁剪

**注册的界面类型（EWidgetType）：**

| 枚举值 | 说明 | 空间类型 |
|---|---|---|
| `EWIDGET_PlayerStatus` | 玩家状态栏（血量/体力/魔法） | 屏幕 |
| `EWIDGET_PawnStatusInScreen` | 角色头顶状态栏 | 世界空间 |
| `EWIDGET_BossStatus` | Boss 状态栏 | 屏幕 |
| `EWIDGET_MainMenu` | 主菜单 | 屏幕 |
| `EWIDGET_InterActPanel` | 交互选项面板 | 屏幕 |
| `EWIDGET_NPCDialog` | NPC 对话界面 | 屏幕 |
| `EWIDGET_Inventory` | 背包/仓库界面 | 屏幕 |

### 1.2 GlobalDelegatesManager — 全局事件总线

`UGlobalDelegatesManager` 提供全局多播委托，解耦 Component 与 UI 之间的数据推送。

**已有委托：**

- `OnAttributeHealthChanged` — 血量变动
- `OnAttributeStaminaChanged` — 耐力变动
- `OnCharacterDied` / `OnCharacterRevived` — 死亡/复活
- `OnItemUsed` / `OnItemEffectTriggered` / `OnItemCountChanged` — 道具相关
- `OnClickInterActBtnToDialog` — 交互按钮到对话
- `OnClickInterMainMenuButton` — 主菜单按钮点击

### 1.3 已有接口体系

| 接口 | 职责 | 面向对象 |
|---|---|---|
| `ICombat_IF` | 战斗能力（受伤、处决、背刺、攻击、防御） | 所有可战斗的 Actor |
| `IHealth_IF` | 生命值查询与修改 | 所有有生命值的 Actor |
| `ICharacterComponent_IF` | 组件定位器（获取各 Component 引用） | 角色 |
| `IWeaponAccessory_IF` | 武器访问 | 持有武器的角色 |
| `IInteraction_IF` | 交互能力（对话、拾取等） | 所有可交互的 Actor |
| `IBehavioralResponse_IF` | 输入行为响应 | 玩家角色 |
| `ILockRotation_IF` | 锁定视角 | 可被锁定的 Actor |
| `IScreenWidget_IF` | 世界空间 Widget 接收所属 Pawn | 世界空间 UI |

---

## 二、问题一：按钮业务逻辑应以何种形式与 UI 蓝图关联

### 2.1 核心矛盾

蓝图擅长布局和视觉表现，但不适合承载复杂的、可复用的业务逻辑。需要一条清晰的**责任分界线**。

### 2.2 推荐架构：三层分离

```
┌──────────────────────────────────────────┐
│  蓝图层 (布局 + 数据绑定 + 事件连线)        │  ← 纯视觉
│  按钮.OnClicked → 调用 C++ 父类方法         │
├──────────────────────────────────────────┤
│  C++ Widget 父类 (SL_UserWidgetBase)      │  ← 轻量逻辑
│  - 持有 UIManagerSubsystem 引用            │
│  - 提供 OpenOtherWidget / CloseSelf       │
│  - 定义 BlueprintNativeEvent 供蓝图重写    │
├──────────────────────────────────────────┤
│  外部 Manager / Component (业务核心)       │  ← 重逻辑
│  - 装备计算、背包操作、合成系统等            │
│  - Widget 通过 Subsystem 或直接获取来调用   │
└──────────────────────────────────────────┘
```

### 2.3 三种场景的分层策略

| 场景 | 逻辑归属 | 理由 |
|---|---|---|
| 按钮点击 → 打开另一个界面 | C++ Widget 基类方法，或直接在蓝图调 UIManagerSubsystem | UI 层自主行为，不涉及游戏数据 |
| 按钮点击 → 消耗物品 / 装备武器 | C++ Widget 方法调用 InventoryComponent / EquipmentComponent | Widget 是发起者，实际逻辑在 Component |
| 按钮点击 → 复杂计算（合成配方验证等） | 委托给专门 Manager（如 CraftingManager），Widget 只传参、收结果 | 纯业务逻辑，与 UI 无关，可独立测试 |

### 2.4 按钮点击调用链

```
蓝图按钮 OnClicked
  → 调用 C++ 父类 OnSomeButtonClicked()  (BlueprintNativeEvent，蓝图可重写)
    → 简单导航：直接调 UIManagerSubsystem->OpenWidget(...)
    → 中等业务：通过 C++ 方法调用 Component
    → 复杂业务：通过 GameInstance->GetSubsystem<SomeManager>() 调用
```

### 2.5 关键原则

**不需要为每个 UI 蓝图创建一对一的"业务类"。** 与 UI 展示强相关的逻辑放在 C++ Widget 父类；纯数据/游戏逻辑放在已有的 Component 或 Manager 中。Widget 保持"薄"，Manager 承载"厚"逻辑。

---

## 三、问题二：UI 蓝图之间的跳转如何关联

### 3.1 核心原则：Widget 之间不直接互相引用

```
❌ 错误做法：
  背包 Widget 里写：打开商店 Widget → 硬引用商店 Widget 类

✅ 正确做法：
  背包 Widget 调用：UIManagerSubsystem->OpenWidget(EWidgetType::Shop)
  UIManagerSubsystem 负责：查找注册的蓝图类 → 创建 → 添加到视口 → 入栈
```

### 3.2 导航三层模型

```
┌────────────────────────────────────┐
│  Widget A (背包)                    │
│  按钮点击 → "我要打开 EWIDGET_Shop"  │
│          → 或 "返回上一页" (Pop)     │
└──────────────┬─────────────────────┘
               │  只依赖 EWidgetType 枚举
               ▼
┌────────────────────────────────────┐
│  UIManagerSubsystem (导航中枢)       │
│  - OpenWidget(Type) → 创建或恢复界面 │
│  - PushWidget / PopWidget → 页面栈  │
│  - 管理同一时刻哪些页面可见           │
│  - 处理层级冲突                     │
└──────────────┬─────────────────────┘
               │
               ▼
┌────────────────────────────────────┐
│  Widget B (商店)                    │
│  被创建、获得焦点、接收上下文数据      │
└────────────────────────────────────┘
```

### 3.3 需要关注的特殊场景

**（1）带参数的跳转**

从背包打开"物品详情"需要传物品 ID。可扩展 `FUICreateParams` 增加通用数据载荷，创建 Widget 后调用 `OnWidgetOpened(Payload)`。

**（2）弹窗叠加 vs 页面替换**

- 主菜单 → 背包：压栈，返回时回到主菜单
- 背包 → 物品详情：可在背包上层叠加，也可替换（视设计而定）
- 死亡画面：清栈 + 打开新页面

建议在 Widget 基类中提供语义化导航方法（`NavigateTo` / `NavigateBack` / `OpenPopup`），而非让每个蓝图直接操作栈。

**（3）页面返回时的数据回传**

商店购买完成关闭后，背包需要刷新。两种方式：

- **被动刷新**：背包的 `OnWidgetResume()` 中重新从 Component 拉数据（推荐，解耦）
- **主动通知**：商店关闭时通过委托广播，背包监听（适合实时性要求高的场景）

---

## 四、UI 与 Component 的交互形式

### 4.1 三种交互模式

项目中已并存三种模式，各有适用场景：

| 模式 | 方向 | 耦合度 | 适用场景 |
|---|---|---|---|
| **委托/事件** | Component → UI（推送） | 弱 | 数据变化时 UI 被动刷新 |
| **接口** | UI ↔ Actor（按能力查询） | 中 | 对 Actor 的通用能力操作 |
| **Subsystem 直接访问** | UI → 业务层（主动拉取/写入） | 强 | UI 发起的主动操作 |

### 4.2 模式分配策略

```
UI Widget
   │
   ├─ 获取数据（读）  →  接口或 Subsystem（按需选择）
   ├─ 执行操作（写）  →  Subsystem 或 Component 方法
   └─ 响应变化（监听）→  委托/事件（GlobalDelegatesManager）
```

### 4.3 委托/事件模式 — 已成熟，继续沿用

```
HealthComponent 血量变化
  → Broadcast OnAttributeHealthChanged(Actor, Old, New, Max, Min)
    → HUD_PlayerStatusBar 监听 → UpdateProgressInfo(...)
    → HUD_PawnStatusBarInScreen 监听 → UpdateProgressInfo(...)
```

**优点**：Component 完全不知道 UI 的存在，UI 可随意增减监听者。

**注意**：高频数据（如每帧变化的体力值）需要节流或改用轮询。

### 4.4 接口粒度 — 按"能力域"划分，非按组件 1:1 映射

```
ICombat_IF  → 封装"战斗相关能力"（受伤、处决、背刺、攻击、防御）
IHealth_IF  → 封装"生命值相关能力"（查询、扣血、回血）
```

接口按**功能域**划分，不按组件一一对应。一个 Character 实现了 `ICombat_IF`，内部可能是 `CombatComponent` + `HealthComponent` 协作完成。

**判断一个能力是否需要接口：**

| 条件 | 建议 |
|---|---|
| 调用方只关心"能不能做"，不关心"谁来实现" | 用接口 ✅ |
| 该能力只有一个实现、永远只有一个实现 | 不需要接口 ❌ |
| 未来可能有不同实现（如不同角色类型） | 用接口 ✅ |

### 4.5 多组件协作场景 — Coordinator 模式

当一个 UI 操作跨越多个 Component 时，引入**协调层**：

**问题场景：玩家点击"使用道具"按钮**

涉及 Component：EquipmentComponent（获取选中道具）→ StateComponent（检查可用状态）→ StaminaComponent（检查体力）→ InventoryComponent（执行使用）→ GAS（播放动画/效果）

**错误做法 — UI 直接编排：**

```
❌ Widget 内部依次调用 5 个 Component
   → UI 变成"指挥官"，知道所有业务细节
   → 耦合过重，后续修改任何一步都需要改 UI 代码
```

**正确做法 — Coordinator 封装：**

```
✅ Widget 调用：ItemUseCoordinator->TryUseSelectedItem()
   → Coordinator 内部协调所有 Component
   → 结果通过委托回传
```

三种协调方案：

| 方案 | 适用场景 | 实现方式 |
|---|---|---|
| **委托广播** | 动作-响应型操作（使用道具、切换武器、拾取） | UI 广播意图 → Component 监听执行 → 结果委托回传 |
| **Manager/Coordinator** | 复杂业务流程（背包操作、合成系统、交易系统） | 独立的 Subsystem，封装多 Component 协调 |
| **Widget 基类封装** | 简单到中等复杂度、不需要跨 Widget 复用 | Widget C++ 基类方法内封装一次调用 |

---

## 五、推荐的分层架构总览

```
┌───────────────────────────────────────────────────────┐
│  UI 层 (蓝图 + C++ Widget 基类)                        │
│  - 数据展示、动画、输入响应                              │
│  - 通过三种渠道与外部交互：                               │
│    ① Subsystem 直接调用（同步操作）                      │
│    ② 接口查询（对 Actor 的通用能力查询）                  │
│    ③ 委托监听（异步数据推送）                             │
├───────────────────────────────────────────────────────┤
│  协调层 (Subsystem / Manager)                          │
│  - 封装跨 Component 的业务流程                          │
│  - 对 UI 提供统一的操作入口                              │
│  - 例子：ItemUseCoordinator、CraftingManager、          │
│    ShopManager                                         │
├───────────────────────────────────────────────────────┤
│  Component 层                                          │
│  - 单一职责的数据和能力                                  │
│  - 不直接与 UI 交互，通过委托向上推送                     │
│  - 实现相关接口 (ICombat_IF、IHealth_IF)                │
└───────────────────────────────────────────────────────┘
```

### 决策速查表

| UI 需要做什么 | 通过什么途径 | 例子 |
|---|---|---|
| 获取/修改单一数据 | 接口（`IHealth_IF`、`ICombat_IF`）或直接 Subsystem | 获取目标血量、判断可否处决 |
| 发起业务操作 | Subsystem / Coordinator | 使用道具、购买物品、装备武器 |
| 响应数据变化 | GlobalDelegatesManager 委托 | 血量变化刷新血条、死亡显示 UI |
| 导航/打开界面 | UIManagerSubsystem | 打开背包、打开商店 |

---

## 六、设计原则总结

1. **职责分离**：UI 管显示，Component 管数据，Manager 管协调。各层不越界。
2. **按能力域设计接口**：接口不是 Component 的复刻，而是调用方视角的能力集合。
3. **多用委托、少用直接引用**：对于 "数据变了，UI 要刷" 的场景，推送优于拉取。
4. **多组件协作用 Coordinator**：UI 不应成为多组件编排的指挥官。
5. **导航通过 UIManagerSubsystem 中转**：Widget 只依赖 EWidgetType 枚举，不直接引用其他 Widget。
6. **不为不存在的东西设计**：在有两个以上具体用例之前，不写通用组件；在有两个以上界面之前，不急于建立 C++ 基类体系。
