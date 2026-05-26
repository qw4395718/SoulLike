# SoulLike UI Widget 架构设计参考

> 版本：2026-05-26 | 基于项目实际重构经验

---

## 一、设计原则

### 原则 1：不预判，抽共性

基类的提取时机由重复次数决定，而非预判：

| 重复次数 | 行动 |
|----------|------|
| 1 次 | 各自实现，不动 |
| 2 次 | 停下来观察共性 |
| >=3 次 | 提取基类 |

### 原则 2：职责分离，两条主线

Widget 可分为两个基本类型，互不交叉：

- **被动展示型**：读数据、显示内容，用户不直接点击交互。示例：物品图标、状态条、血量条
- **主动交互型**：用户点击 -> 触发行为。示例：菜单按钮、关卡选择、交互选项

混用两条线的基类（如让一个"可点击按钮"继承"物品槽"）会在后续扩展时暴露出职责不匹配的问题。

### 原则 3：基类带宽控制

向基类添加新功能前，确认它是否符合"大部分场景使用"的门槛：

- Button：100% -> 进基类（必须）
- Text：100% -> 进基类（必须）
- Image：约70% -> 可选进基类（OptionalWidget）
- 动画、输入提示、额外按钮等：<30% -> 不进基类，留给派生类

低于门槛的功能放在派生类中，避免基类膨胀成"万能控件"。

### 原则 4：性能不是隔离因素

- BindWidget 没有绑定时仅消耗 8 字节的空指针
- OptionalWidget = true 的组件无运行时开销
- 真正影响性能的是：Widget 总实例数、Overdraw、Tick 频率

---

## 二、当前架构（重构后）

```
UUserWidget
|
+-- 分支 A：物品/状态槽 (BaseSlot 分支)
|   职责：图标 + 堆叠数量展示，被动展示
|
|   UUI_BaseSlot
|   +-- m_showImage (Image)        -- 图标
|   +-- m_stackNum (TextBlock)     -- 堆叠数量
|   |
|   +-- UUI_DefaultSlot            -> 道具图标场景
|   |   +-- m_newTipImage          + 新获取提示
|   |
|   +-- UUI_IconSlot               -> 状态效果图标
|   |   +-- SetData(FStatusEffectInfo) + 状态数据绑定
|   |
|   +-- UUI_InventorySlot          -> 背包格子
|       +-- 拖拽/选中方法          + 背包专有交互
|
+-- 分支 B：可点击列表项 (ListItemBase 分支)
|   职责：按钮 + 文本 + 图标 + 点击委托，主动交互
|
|   UUI_ListItemBase
|   +-- m_mainButton (Button)      -- 主按钮
|   +-- m_titleText (TextBlock)    -- 标题文本
|   +-- m_iconImage (Image, 可选)  -- 图标
|   +-- int32 m_itemData           -- 关联数据
|   +-- FOnListItemClickedSignature -- 点击委托
|   +-- SetDisplayText()
|   +-- SetIcon()
|   +-- SetItemData() / GetItemData()
|   +-- SetItemEnabled()           -- 禁用时自动置灰
|   +-- SetSelected()
|   |
|   +-- UUI_MenuItem               -> 主菜单功能项
|       +-- m_newTipImage          + 新获取提示标记
|
+-- 独立类
    |
    +-- UUI_InterActButton         -> 交互选项（独特交互模式）
        +-- 选中/取消动画
        +-- 输入设备提示
        +-- NormalColor / SelectedColor
```

---

## 三、基类使用指南

### 3.1 什么时候用 ListItemBase

适合的场景：
- 一个可点击的"卡片"或"按钮条目"
- 需要显示标题，可选显示图标
- 点击后触发一个行为（打开界面、选择关卡、执行功能）

使用方式：CreateWidget<UUI_ListItemBase>，调用 SetDisplayText / SetIcon / SetItemData / SetItemEnabled，绑定 OnItemClicked 委托。

不需要派生 C++ 子类——蓝图继承即可。

### 3.2 什么时候用 BaseSlot

适合的场景：
- 显示物品/道具的图标和数量
- 用户不直接点击该 widget 触发行为（点击由父级通过坐标判断）
- 背包、装备栏、快捷栏、商店物品

### 3.3 什么时候新建派生类

条件：基类无法覆盖业务，且该功能的使用率低于 50%。

示例：UUI_MenuItem 需要 m_newTipImage（新获取提示），但这个功能不是所有列表项都需要的，所以它作为派生类存在，而不是塞进 ListItemBase。

### 3.4 什么时候应该新建独立的类

条件：交互模式完全不同于已有分支。

示例：UUI_InterActButton 涉及选中动画、颜色切换、输入设备提示，这些功能与 ListItemBase 的"点击即触发"模型差异较大，作为独立类更清晰。

---

## 四、后续扩展指引

### 4.1 新加一个界面项时的决策树

```
新界面需要一个子控件？

+-- 它是"可点击的列表项"吗？
|   +-- 是 -> 直接用 UUI_ListItemBase（蓝图继承，不写 C++）
|   +-- 否 -> 它是"物品/状态槽"吗？
|       +-- 是 -> 用 UUI_BaseSlot 分支
|       +-- 否 -> 是否需要独特的交互模式？
|           +-- 需要新交互 -> 按场景新建独立类
|           +-- 只是展示 -> 直接用 UUserWidget
```

### 4.2 什么时候应该往基类加功能

当第 3 个不同的派生类都实现了同一种功能时：
1. 第一个类实现 -> 没问题
2. 第二个类也实现 -> 观察，暂不动
3. 第三个类又实现 -> 往基类加，作为 OptionalWidget

### 4.3 不推荐的做法

- 在一开始就设计"万能基类"涵盖所有可能
- 让可点击按钮继承物品槽（职责错位）
- 为了减少一行代码而在基类中添加使用率极低的组件
- 创建新 C++ 派生类只是为了换个 BindWidget 名称

---

## 五、本次重构记录

### 背景

原有 UI 子控件缺乏统一的职责划分：
- UUI_MenuItem 继承自 UUI_BaseSlot（物品槽基类），但它的实际职责是"可点击的菜单项"
- UUI_BaseCoin、UUI_CoinShowBar 定义了但从未被使用
- 关卡选择（LobbyScreen）用裸 UButton 构造，无法控制尺寸

### 改动清单

| 操作 | 文件 | 说明 |
|------|------|------|
| 新建 | UI_ListItemBase.h/.cpp | 通用列表项基类（按钮+文本+图标+委托） |
| 修改 | HUD_LobbyScreen.h/.cpp | 容器从 UHorizontalBox 改为 UScrollBox，子项从裸 UButton 改为 UUI_ListItemBase |
| 修改 | UI_MenuItem.h/.cpp | 基类从 UUI_BaseSlot 改为 UUI_ListItemBase，移除 m_actionButton/m_btnLabel |
| 修改 | Pop_MainMenu.cpp | 调用从 SetImageBrush/SetCenterTitle 改为 SetIcon/SetDisplayText |
| 删除 | UI_BaseCoin.h/.cpp | 未使用 |
| 删除 | UI_CoinShowBar.h/.cpp | 未使用 |

### 后续蓝图操作

| 蓝图 | 操作 |
|------|------|
| WBP_LevelSelectItem（新建） | 父类 UUI_ListItemBase，根部 SizeBox + Button + TextBlock，绑定到 m_mainButton/m_titleText |
| WBP_HUD_LobbyScreen | 替换为 ScrollBox（Orientation=Horizontal），Level Item Widget Class 设为 WBP_LevelSelectItem |
| WBP_MenuItem（已有） | 绑定从 m_actionButton/m_btnLabel 改为 m_mainButton/m_titleText；图标从 m_showImage 改为 m_iconImage（可选） |

---

## 六、相关文件索引

| 文件 | 位置 |
|------|------|
| UI 框架设计方案 | Docs/ui-framework-design.md |
| 本文档 | Docs/ui-widget-architecture.md |
| UUI_ListItemBase | Source/.../SharedCompoent/UI_ListItemBase.h |
| UUI_BaseSlot | Source/.../SharedCompoent/UI_BaseSlot.h |
| UUI_MenuItem | Source/.../SharedCompoent/UI_MenuItem.h |
| UUI_InterActButton | Source/.../SharedCompoent/UI_InterActButton.h |
| HUD_LobbyScreen | Source/.../HUD/HUD_LobbyScreen.h |
| Pop_MainMenu | Source/.../Pop/Pop_MainMenu.h |
