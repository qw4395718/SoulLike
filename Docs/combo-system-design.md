# SoulLike 连招系统设计文档

> 版本：2026-05-26 | 基于现有代码重构

---

## 一、架构总览

### 1.1 核心组件

```
PlayerController                   ComboManagerComponent              AbilityTask_ComboMontage
                                   (流程控制中心)                     (动画执行层)
  ProcessComboInput()                                                     
  -----------------> HandleInputPressed()                                
                          |                                               
                          ├─ 查 ComboInfoTable                            
                          ├─ 按 ExecuteType 分流                          
                          │    ├─ Instant → 激活 GAS Ability             
                          │    ├─ Charge  → StartCharge()                
                          │    └─ Channel → 激活 GAS Ability             
                          │                                               
                          ├─ 连招窗口内：OnInputReceived() -------> 蒙太奇衔接
                          └─ 初始连段：TryActivateAbility()
```

### 1.2 数据驱动

所有连招行为由 `ComboInfo` 数据表驱动，不需要修改 C++ 代码来添加新招式：

```
ComboInfo 数据表（DataTable）
  ├─ 键：ActiveRequireWindowTag + InputActionType
  └─ 值：FComboInfo（ExecuteType、消耗、倍率、GA 类）
          ↑
  一行数据 = 一个招式
```

---

## 二、核心数据结构

### 2.1 EComboInputActionType（输入类型）

```cpp
UENUM(BlueprintType)
enum class EComboInputActionType : uint8
{
    None,
    Light,      // 轻攻击（X / 鼠标左键）
    Height,     // 重攻击（Y / 鼠标右键）
    Special,    // 特殊攻击（B 键）
    Max
};
```

### 2.2 EComboExecuteType（执行类型）

```cpp
UENUM(BlueprintType)
enum class EComboExecuteType : uint8
{
    Instant,    // 瞬发型：按下立即执行（原逻辑）
    Charge,     // 蓄力型：按住蓄力，释放触发
    Channel,    // 持续型：按住持续生效（如持续施法）
    Max
};
```

### 2.3 FComboInfo（招式数据行）

```cpp
USTRUCT(BlueprintType)
struct FComboInfo : public FTableRowBase
{
    // ===== 查询条件 =====
    FGameplayTag ActiveRequireWindowTag;     // 当前窗口标签
    EComboInputActionType InputActionType;   // 输入按键

    // ===== 行为 =====
    EComboExecuteType ExecuteType;           // 执行类型
    TSubclassOf<UGameplayAbility> NextAbilityClass;  // 触发的 GA

    // ===== 消耗 =====
    float StaminaCost = 20.0f;               // 体力消耗

    // ===== 伤害 =====
    float DamageMultiplier = 1.0f;           // 伤害倍率

    // ===== 蓄力参数（仅 ExecuteType == Charge 时有效）=====
    float MinChargeTime = 0.5f;              // 最短蓄力时间
    float MaxChargeTime = 2.0f;              // 满蓄力时间
    float MaxChargeDamageMultiplier = 2.5f;  // 满蓄伤害倍率
    TSubclassOf<UGameplayAbility> ChargeHoldAbilityClass;  // 蓄力占位 GA
};
```

### 2.4 FComboLookupKey（查询键）

```cpp
USTRUCT(BlueprintType)
struct FComboLookupKey
{
    FGameplayTag WindowTag;          // 当前窗口标签
    EComboInputActionType InputType; // 输入类型
};
```

ComboInfoTable 内部使用 `TMap<FComboLookupKey, FComboInfo>` 做 O(1) 查找。

---

## 三、执行流程

### 3.1 HandleInputPressed（核心入口）

```
Player presses a key
  │
  ├─ Controller::OnXxxPressed()
  │   └─ ProcessComboInput(InputType)
  │       └─ ComboManager::HandleInputPressed(InputType)
  │
  ├─ Step 1: 获取 ASC 当前 Tags
  │   ├─ 有 State.Window.* Tag → 连招窗口内
  │   └─ 无                  → 初始连段（补 State.Window.None）
  │
  ├─ Step 2: 查 ComboInfoTable
  │   └─ FindNextComboInfo(WindowTags, InputType)
  │
  ├─ Step 3: 按 ExecuteType 分流
  │
  ├─ Instant:
  │   ├─ 体力检查
  │   ├─ 窗口内 → 通知 ActiveComboTask 做蒙太奇衔接
  │   └─ 初始   → TryActivateAbility() + 消耗体力
  │
  ├─ Charge:
  │   ├─ 窗口内 → 注册蒙太奇过渡回调
  │   └─ 初始   → StartCharge()
  │
  └─ Channel:
      └─ 直接 TryActivateAbility()（Ability 内部管理持续逻辑）
```

### 3.2 蓄力生命周期（Charge）

```
StartCharge()
  │
  ├─ 设置 m_bIsCharging = true
  ├─ 记录 m_chargeStartTime
  ├─ 启用 TickComponent
  │
  ├─ Tick → UpdateCharge()
  │   ├─ 计算 m_chargeLevel（0.0 ~ 1.0）
  │   ├─ 到达 MinChargeTime → 激活 ChargeHoldAbilityClass（蓄力占位动画）
  │   ├─ 广播 OnChargeLevelChanged（UI 更新蓄力条）
  │   └─ 到达 MaxChargeTime → 自动 ReleaseCharge()
  │
  └─ ReleaseCharge()  <- 由 OnHeavyAttackReleased() 或 Tick 触发
      │
      ├─ 清 m_bIsCharging，停 Tick
      ├─ 根据蓄力等级选择释放标签：
      │   ├─ m_chargeLevel >= 1.0 → State.Charge.Full
      │   └─ m_chargeLevel < 1.0  → State.Charge.Partial
      ├─ 查表找到对应的释放 GA
      ├─ 应用蓄力伤害倍率
      └─ TryActivateAbility()
```

---

## 四、时序图

### 4.1 普通瞬发连招（Instant）

```
Player      Controller      ComboManager      ComboInfoTable    ASC         AbilityTask
  │             │                │                  │            │             │
  │---Press---->│                │                  │            │             │
  │             │-ProcessCombo-->│                  │            │             │
  │             │                │---FindNext------>│            │             │
  │             │                │<--FComboInfo-----│            │             │
  │             │                │                  │            │             │
  │             │                │  [ExecuteType=Instant]         │             │
  │             │                │                  │            │             │
  │             │                ├─ 窗口内？                    │             │
  │             │                │  是→OnInputReceived()------->│             │
  │             │                │  否→TryActivateAbility()---->│             │
  │             │                │                  │            │             │
```

### 4.2 蓄力重击（Charge）

```
Player      Controller      ComboManager      ComboInfoTable    ASC         AbilityTask
  │             │                │                  │            │             │
  │---Press---->│                │                  │            │             │
  │             │-ProcessCombo-->│                  │            │             │
  │             │                │---FindNext------>│            │             │
  │             │                │<--FComboInfo-----│            │             │
  │             │                │                  │            │             │
  │             │                │  [ExecuteType=Charge]         │             │
  │             │                │                  │            │             │
  │             │                │-StartCharge()    │            │             │
  │             │                │  [Tick starts]   │            │             │
  │             │                │                  │            │             │
  │    [Hold]   │                │                  │            │             │
  │~~~~~Hold~~~~│                │                  │            │             │
  │             │                │-UpdateCharge()   │            │             │
  │             │                │  · 计算蓄力等级   │            │             │
  │             │                │  · 广播UI进度     │            │             │
  │             │                │  · 到Min→激活    │            │             │
  │             │                │    ChargeHold GA---->-------→│             │
  │             │                │                  │            │             │
  │---Release-->│                │                  │            │             │
  │             │-ReleaseCharge->│                  │            │             │
  │             │                │  [Tick stops]    │            │             │
  │             │                │                  │            │             │
  │             │                │ 按等级查释放表    │            │             │
  │             │                │---FindNext------>│            │             │
  │             │                │<--ReleaseInfo----│            │             │
  │             │                │                  │            │             │
  │             │                │ 应用蓄力倍率     │            │             │
  │             │                │-TryActivate()--->│            │             │
```

---

## 五、职责边界

| 组件 | 职责 | 不应做的事 |
|------|------|-----------|
| PlayerController | 映射按键到 ComboInputActionType；释放时通知 ReleaseCharge | 不参与连招逻辑判断 |
| ComboManagerComponent | 连招流程控制：查表、分流、蓄力状态、体力管理 | 不处理动画蒙太奇 |
| AbilityTask_ComboMontage | 蒙太奇播放、衔接、窗口标签管理 | 不决定打哪一招 |
| ComboInfoTable | 数据存储与查询 | 不含业务逻辑 |
| GAS Ability | 具体招式效果（伤害、动画、特效） | 不参与连招路由 |
| UI（蓄力进度条） | 监听 OnChargeLevelChanged | 不管理蓄力状态 |

---

## 六、数据表配表示例

### 6.1 轻攻击三连（Instant）

| ActiveRequireWindowTag | InputType | ExecuteType | NextAbilityClass | StaminaCost |
|---|---|---|---|---|
| State.Window.None | Light | Instant | GA_LightAttack_1 | 10 |
| State.Window.Light_1 | Light | Instant | GA_LightAttack_2 | 10 |
| State.Window.Light_2 | Light | Instant | GA_LightAttack_3 | 12 |

### 6.2 蓄力重击（Charge）

| ActiveRequireWindowTag | InputType | ExecuteType | MinCharge | MaxCharge | MaxDamageMult | Next/ChargeHold |
|---|---|---|---|---|---|---|
| State.Window.None | Height | Charge | 0.5 | 2.0 | 2.5 | ChargeHold=GA_HeavyCharge_Hold |
| State.Charge.Partial | Height | Instant | - | - | - | GA_HeavyCharge_Partial |
| State.Charge.Full | Height | Instant | - | - | - | GA_HeavyCharge_Full |

### 6.3 持续施法（Channel）

| ActiveRequireWindowTag | InputType | ExecuteType | NextAbilityClass |
|---|---|---|---|
| State.Window.None | Special | Channel | GA_ChannelSpell_1 |

Channel 类型的 GA 内部自行管理"按住持续 vs 松开停止"的逻辑（通过 AbilityTask 监听 InputReleased）。

---

## 七、扩展指南

### 7.1 添加新的输入类型

```cpp
// SoulLikeGameGlobal.h 的 EComboInputActionType 中添加
Defence,        // 防御
Interact,       // 交互
```

然后在 Controller 中绑定新按键：
```cpp
InputComponent->BindAction("Defence", IE_Pressed, this, &ASL_PlayerControllerBase::OnDefencePressed);
```

### 7.2 添加新的执行类型

1. `SoulLikeGameGlobal.h` 的 `EComboExecuteType` 中加值
2. `ComboManagerComponent::HandleInputPressed` 的 switch 中加 case
3. 如需跨帧状态管理，添加对应的 StartXxx / ReleaseXxx 方法

### 7.3 添加新的蓄力等级

蓄力等级通过 Tag 区分。当前两级（Partial / Full）：

| 等级 | 标签 | 触发条件 |
|------|------|---------|
| 未满蓄 | State.Charge.Partial | MinChargeTime ≤ 蓄力时间 < MaxChargeTime |
| 满蓄 | State.Charge.Full | 蓄力时间 ≥ MaxChargeTime |

如需更多级别（三级蓄力），修改 `ReleaseCharge()` 中的分级逻辑：

```cpp
FGameplayTag ReleaseTag;
if (m_chargeLevel >= 1.0f)
    ReleaseTag = ...Full;
else if (m_chargeLevel >= 0.6f)
    ReleaseTag = ...Medium;
else
    ReleaseTag = ...Partial;
```

### 7.4 添加蓄力 UI

任何 Widget 可以监听 ComboManager 的委托：

```cpp
// 在 Widget 的 NativeConstruct 中
if (USL_ComboManagerComponent* ComboMgr = ...)
{
    ComboMgr->OnChargeLevelChanged.AddDynamic(this, &UMyWidget::OnChargeLevelUpdated);
}

// 回调
void UMyWidget::OnChargeLevelUpdated(float Level)
{
    m_chargeBar->SetPercent(Level);
    m_chargeText->SetText(FText::AsNumber(FMath::RoundToInt(Level * 100)));
}
```

---

## 八、相关文件索引

| 文件 | 路径 |
|------|------|
| 连招管理器头文件 | Source/.../Component/Character/SL_ComboManagerComponent.h |
| 连招管理器实现 | Source/.../Component/Character/SL_ComboManagerComponent.cpp |
| 连招数据表 | Source/.../Table/ComboInfoTable.h / .cpp |
| 连招蒙太奇 Task | Source/.../GAS/AT/AbilityTask_ComboMontage.h / .cpp |
| 全局枚举与结构体 | Source/.../SoulLikeGameGlobal.h |
| 控制器输入绑定 | Source/.../Class/SL_PlayerControllerBase.cpp |
| 本文档 | Docs/combo-system-design.md |
