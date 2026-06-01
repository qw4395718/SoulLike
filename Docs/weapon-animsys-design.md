# WeaponAnimSet 连招动画系统实施方案

> 设计目标：为 SoulLike 项目实现类似《怪物猎人世界》风格的派生连招系统，
> 通过 `USL_WeaponAnimSet` DataAsset 将武器动画数据与连招逻辑解耦。

---

## 目录

1. [现状与问题](#1-现状与问题)
2. [整体架构](#2-整体架构)
3. [详细步骤](#3-详细步骤)
   - [Step 1: 创建 WeaponAnimSet DataAsset](#step-1-创建-weaponanimset-dataasset)
   - [Step 2: 武器数据表关联 AnimSet](#step-2-武器数据表关联-animset)
   - [Step 3: FComboInfo 增加输出蒙太奇标签](#step-3-fcomboinfo-增加输出蒙太奇标签)
   - [Step 4: ComboManager 增加蒙太奇解析方法](#step-4-combomanager-增加蒙太奇解析方法)
   - [Step 5: 通用连招 GA 接入 AnimSet](#step-5-通用连招-ga-接入-animset)
   - [Step 6: 角色主 AnimBP 接入 IdleAdditivePose](#step-6-角色主-animbp-接入-idleadditivepose)
4. [内容端工作](#4-内容端工作)
5. [运行时全流程](#5-运行时全流程)
6. [改动清单](#6-改动清单)
7. [与其他系统的关系](#7-与其他系统的关系)
8. [未涵盖范围](#8-未涵盖范围)

---

## 1 现状与问题

### 现有架构

```
Input → ComboManager::HandleInputPressed()
  → 查 UComboInfoTable → 找到 FComboInfo
    → CurrentComboInfo = FComboInfo（已暂存）
      → ASC->TryActivateAbilityByClass(NextAbilityClass)
        → GA 激活 → 内部硬编码蒙太奇 → UAbilityTask_ComboMontage
```

### 已有基础设施

| 组件 | 职责 |
|------|------|
| `USL_ComboManagerComponent` | 连招状态管理、输入检测、窗口标签管理、蓄力系统 |
| `FComboInfo` (DataTable) | 单段连招配置：窗口Tag、输入类型、执行类型、GA、消耗、伤害倍率、蓄力参数 |
| `FComboLookupKey` | `WindowTag + InputType` 复合 Key |
| `UAbilityTask_ComboMontage` | 蒙太奇播放、AllowBlend 输入缓存、混合打断 |
| `UComboInfoTable` | 连招表查询 |

### 核心断点

**每种武器的连招用不同的 GA，GA 内部硬编码了蒙太奇路径**。导致：
- 新增武器必须新建全套 GA
- 武器间不能复用通用连招逻辑
- 连招数据分散在多个 GA 中，不便于集中管理

### 设计原则

- 连招逻辑在 C++/GAS 中驱动，AnimBP 只负责播放动画
- 动画资源按武器类型聚合到 DataAsset，数据驱动而非代码驱动
- 保持单 AnimBP 架构（不引入 Linked Anim BP）

---

## 2 整体架构

```
┌─────────────────────────────────────────────────────────┐
│                  数据层（DataTable）                      │
│  DT_ComboInfo:                                           │
│    ActiveRequireWindowTag | InputType | OutputMontageTag │
│    | NextAbilityClass | DamageMult | StaminaCost | ...   │
└──────────────┬──────────────────────────────────────────┘
               │ 查询
┌──────────────▼──────────────────────────────────────────┐
│                  逻辑层（C++）                            │
│  ComboManager::HandleInputPressed()                     │
│    → 查 DT_ComboInfo → FComboInfo                       │
│    → ASC->TryActivateAbilityByClass(GA_ComboAction)     │
│      → GA: ResolveCurrentMontage()                      │
│        → Weapon->GetWeaponAnimSet()                     │
│          → ComboMontages[OutputMontageTag]               │
│      → UAbilityTask_ComboMontage::Create(蒙太奇)         │
└──────────────┬──────────────────────────────────────────┘
               │ 播放
┌──────────────▼──────────────────────────────────────────┐
│                  动画层（AnimBP + Montage）               │
│  角色主 AnimBP:                                          │
│    ├─ Locomotion 状态机                                  │
│    ├─ Additive 层: WeaponAnimSet->IdleAdditivePose      │
│    └─ Slot 'ComboSlot': 播放攻击蒙太奇                   │
└─────────────────────────────────────────────────────────┘
```

---

## 3 详细步骤

### Step 1: 创建 WeaponAnimSet DataAsset

**新增文件**：`Source/SoulLikeDemo/Public/Data/SL_WeaponAnimSet.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SoulLikeGameGlobal.h"
#include "SL_WeaponAnimSet.generated.h"

UCLASS(BlueprintType)
class SOULLIKEDEMO_API USL_WeaponAnimSet : public UDataAsset
{
    GENERATED_BODY()

public:
    // ===== 标识 =====
    /** 对应的武器类型 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EWeaponType WeaponType;

    // ===== 连招蒙太奇映射 =====
    /** Key = FComboInfo.OutputMontageTag, Value = 对应的蒙太奇 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    TMap<FGameplayTag, TSoftObjectPtr<UAnimMontage>> ComboMontages;

    // ===== 待机姿态 =====
    /** 拔刀状态时叠加到角色下身的 Additive 动画 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pose")
    TSoftObjectPtr<UAnimSequence> IdleAdditivePose;

    // ===== 持刀移动参数 =====
    /** 持刀时的移速倍率 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float DrawMoveSpeedMultiplier = 1.0f;

    /** 持刀时动画播放速率 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float AnimDrawPlayRate = 1.0f;

    // ===== 受击蒙太奇 =====
    /** 受击反应动画（可选，未设置则使用角色默认受击） */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reaction")
    TSoftObjectPtr<UAnimMontage> HitReactMontage;

public:
    /** 根据标签从 ComboMontages 中查询蒙太奇 */
    UFUNCTION(BlueprintPure, Category = "WeaponAnimSet")
    UAnimMontage* GetComboMontageByTag(const FGameplayTag& InTag) const;
};
```

**新增文件**：`Source/SoulLikeDemo/Private/Data/SL_WeaponAnimSet.cpp`

```cpp
#include "SL_WeaponAnimSet.h"

UAnimMontage* USL_WeaponAnimSet::GetComboMontageByTag(const FGameplayTag& InTag) const
{
    if (!InTag.IsValid()) return nullptr;

    const TSoftObjectPtr<UAnimMontage>* Found = ComboMontages.Find(InTag);
    if (Found && !Found->IsNull())
    {
        return Found->LoadSynchronous();
    }
    return nullptr;
}
```

需要将 `Private/Data/` 目录加入 Build.cs 的模块包含路径，或在 `SoulLikeDemo.Build.cs` 中添加对 `Data` 目录的扫描。

---

### Step 2: 武器数据表关联 AnimSet

**修改文件**：`Source/SoulLikeDemo/Public/SoulLikeGameGlobal.h`

在 `FWeaponDataInfo` 末尾（第551行 `};` 前）增加：

```cpp
/** 该武器的动画数据集（连招蒙太奇、待机姿态等） */
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
TSoftObjectPtr<USL_WeaponAnimSet> WeaponAnimSet;
```

**修改文件**：`Source/SoulLikeDemo/Public/Class/SL_WeaponBase.h`

在 `public` 区域增加：

```cpp
/** 获取当前武器的 AnimSet（运行时懒加载） */
UFUNCTION(BlueprintPure, Category = "Weapon|Animation")
USL_WeaponAnimSet* GetWeaponAnimSet() const;
```

**修改文件**：`Source/SoulLikeDemo/Private/Class/SL_WeaponBase.cpp`

增加实现：

```cpp
#include <SL_WeaponAnimSet.h>

USL_WeaponAnimSet* ASL_WeaponBase::GetWeaponAnimSet() const
{
    if (WeaponData.WeaponAnimSet.IsNull()) return nullptr;
    return WeaponData.WeaponAnimSet.LoadSynchronous();
}
```

---

### Step 3: FComboInfo 增加输出蒙太奇标签

**修改文件**：`Source/SoulLikeDemo/Public/SoulLikeGameGlobal.h`

在 `FComboInfo` 中（现有 `NextAbilityClass` 字段附近）增加：

```cpp
/** 从 WeaponAnimSet 中查询蒙太奇用的标签 */
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
FGameplayTag OutputMontageTag;
```

同时在 `DefaultGameplayTags.ini`（或 Project Settings > GameplayTags）中注册一套蒙太奇标签：

```
Montage.Combo.Light_1       - 轻击第一段
Montage.Combo.Light_2       - 轻击第二段
Montage.Combo.Light_3       - 轻击第三段
Montage.Combo.Heavy_1       - 重击第一段
Montage.Combo.Heavy_2       - 重击第二段
Montage.Combo.Special       - 特殊技
Montage.Combo.Charge_Release - 蓄力释放
Montage.Combo.Roll_Attack   - 翻滚攻击
```

---

### Step 4: ComboManager 增加武器连招表查询

**核心改动**：将全局连招表查询改为优先查询武器专属的连招表，降级到全局默认表。

新增辅助方法 `GetCurrentWeaponComboTable`：

**修改文件**：`Source/SoulLikeDemo/Public/Component/Character/SL_ComboManagerComponent.h`

在 `public` 区域新增：

```cpp
/** 从当前持有武器的 WeaponAnimSet 中获取武器专属连招表 */
class UComboInfoTable* GetCurrentWeaponComboTable() const;
```

**修改文件**：`Source/SoulLikeDemo/Private/Component/Character/SL_ComboManagerComponent.cpp`

include 增加：

```cpp
#include <DataTableManager.h>
#include <ComboInfoTable.h>
```

实现：

```cpp
UComboInfoTable* USL_ComboManagerComponent::GetCurrentWeaponComboTable() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    IWeaponAccessory_IF* WeaponAccessory = Cast<IWeaponAccessory_IF>(Owner);
    if (!WeaponAccessory) return nullptr;

    ASL_WeaponBase* Weapon = WeaponAccessory->GetRightHandWeapon();
    if (!Weapon) return nullptr;

    USL_WeaponAnimSet* AnimSet = Weapon->GetWeaponAnimSet();
    if (!AnimSet) return nullptr;

    if (AnimSet->ComboInfoTable.IsNull()) return nullptr;
    return Cast<UComboInfoTable>(AnimSet->ComboInfoTable.LoadSynchronous());
}
```

修改 `HandleInputPressed` 中的查表部分（原第45-47行 `GetDataTable` → 改为优先查武器表）：

```cpp
// 替换原有的：
// UComboInfoTable* comboInfoTable = Cast<UComboInfoTable>(
//     tableManager->GetDataTable(EDataTableType::DT_ComboInfo));

// 改为：
UComboInfoTable* ComboTable = GetCurrentWeaponComboTable();
if (!ComboTable)
{
    // 降级：从全局 DataTableManager 获取
    if (UDataTableManager* TM = UDataTableManager::Get(this))
    {
        ComboTable = Cast<UComboInfoTable>(
            TM->GetDataTable(EDataTableType::DT_ComboInfo));
    }
}
if (!ComboTable) return;

// 后续所有 comboInfoTable 引用替换为 ComboTable
```

### Step 4b: ComboManager 增加蒙太奇解析方法

**修改文件**：`Source/SoulLikeDemo/Public/Component/Character/SL_ComboManagerComponent.h`

在 `public` 区域新增：

```cpp
/** 根据当前 CurrentComboInfo 的 OutputMontageTag，
 *  从当前持有武器的 WeaponAnimSet 中解析出蒙太奇 */
UFUNCTION(BlueprintCallable, Category = "Combo")
UAnimMontage* ResolveCurrentMontage() const;
```

**修改文件**：`Source/SoulLikeDemo/Private/Component/Character/SL_ComboManagerComponent.cpp`

include 增加：

```cpp
#include <WeaponAccessory_IF.h>
#include <SL_WeaponBase.h>
#include <SL_WeaponAnimSet.h>
```

实现：

```cpp
UAnimMontage* USL_ComboManagerComponent::ResolveCurrentMontage() const
{
    if (!CurrentComboInfo.OutputMontageTag.IsValid())
        return nullptr;

    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    // 通过武器访问接口获取当前右/左手武器
    IWeaponAccessory_IF* WeaponAccessory = Cast<IWeaponAccessory_IF>(Owner);
    if (!WeaponAccessory) return nullptr;

    ASL_WeaponBase* Weapon = WeaponAccessory->GetRightHandWeapon();
    if (!Weapon) return nullptr;

    USL_WeaponAnimSet* AnimSet = Weapon->GetWeaponAnimSet();
    if (!AnimSet) return nullptr;

    return AnimSet->GetComboMontageByTag(CurrentComboInfo.OutputMontageTag);
}
```

---

### Step 5: 通用连招 GA 接入 AnimSet

**核心改动**：创建一个 `GA_ComboAction`，取代每种武器各一个 GA 的模式。

**新增文件**：`Source/SoulLikeDemo/Public/GAS/GA/GA_ComboAction.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_ComboAction.generated.h"

UCLASS()
class SOULLIKEDEMO_API UGA_ComboAction : public UGameplayAbility
{
    GENERATED_BODY()

public:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void CancelAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateCancelAbility) override;

protected:
    UFUNCTION()
    void OnComboMontageCompleted();

    UFUNCTION()
    void OnComboMontageInterrupted();

    UPROPERTY()
    class UAbilityTask_ComboMontage* ComboTask;
};
```

**新增文件**：`Source/SoulLikeDemo/Private/GAS/GA/GA_ComboAction.cpp`

```cpp
#include "GA_ComboAction.h"
#include "AbilitySystemComponent.h"
#include <SL_ComboManagerComponent.h>
#include <AT/AbilityTask_ComboMontage.h>

void UGA_ComboAction::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // 1. 获取 ComboManager
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar) { EndAbility(Handle, ActorInfo, ActivationInfo, true, false); return; }

    USL_ComboManagerComponent* ComboMgr =
        Avatar->FindComponentByClass<USL_ComboManagerComponent>();
    if (!ComboMgr) { EndAbility(Handle, ActorInfo, ActivationInfo, true, false); return; }

    // 2. 注册此 Task 到 ComboManager（在创建 Task 后调用 RegisterActiveComboTask）

    // 3. 通过 WeaponAnimSet 解析蒙太奇
    UAnimMontage* Montage = ComboMgr->ResolveCurrentMontage();
    if (!Montage) { EndAbility(Handle, ActorInfo, ActivationInfo, true, false); return; }

    // 4. 创建 ComboMontage Task
    ComboTask = UAbilityTask_ComboMontage::CreateComboMontageTask(
        this, Montage, 0.2f, 1.0f);

    if (!ComboTask) { EndAbility(Handle, ActorInfo, ActivationInfo, true, false); return; }

    // 5. 注册到 ComboManager
    ComboMgr->RegisterActiveComboTask(ComboTask);

    // 6. 绑定回调
    ComboTask->OnCompleted.AddDynamic(this, &UGA_ComboAction::OnComboMontageCompleted);
    ComboTask->OnInterrupted.AddDynamic(this, &UGA_ComboAction::OnComboMontageInterrupted);

    // 7. 启动 Task
    ComboTask->ReadyForActivation();
}

void UGA_ComboAction::CancelAbility(...)
{
    if (ComboTask && ComboTask->IsActive())
    {
        ComboTask->EndTask();
    }
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UGA_ComboAction::OnComboMontageCompleted()
{
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (Avatar)
    {
        if (USL_ComboManagerComponent* ComboMgr =
            Avatar->FindComponentByClass<USL_ComboManagerComponent>())
        {
            ComboMgr->UnregisterActiveComboTask();
            ComboMgr->OnMontageFinished();
        }
    }
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_ComboAction::OnComboMontageInterrupted()
{
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (Avatar)
    {
        if (USL_ComboManagerComponent* ComboMgr =
            Avatar->FindComponentByClass<USL_ComboManagerComponent>())
        {
            ComboMgr->UnregisterActiveComboTask();
        }
    }
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}
```

**修改连招 DataTable**：所有 `FComboInfo.NextAbilityClass` 统一指向 `GA_ComboAction`。

---

### Step 6: 角色主 AnimBP 接入 IdleAdditivePose

#### 6.1 AnimBP 变量

在角色主 AnimBP 中增加变量：

```
变量名: WeaponAnimSet
类型:   USL_WeaponAnimSet* (对象引用)
默认值: nullptr
```

#### 6.2 AnimGraph 调整

```
[Locomotion 状态机] ─── 基础 Pose
       │
       ▼
[LayeredBonePerBone] ─── 按骨骼分层混合
  ├─ 基础层: Locomotion 输出（全部骨骼）
  └─ 叠加层: WeaponAnimSet->IdleAdditivePose（Additive）
             权重条件: bWeaponDrawn == true
             骨骼过滤: spine_01 及以上（仅上身）
       │
       ▼
[Slot 'ComboSlot'] ─── 播放攻击蒙太奇（覆盖全身或仅上身）
       │
       ▼
[Output Pose]
```

#### 6.3 角色 C++ 同步 AnimSet

角色拔出/切换武器时，将当前武器的 AnimSet 推送到 AnimBP：

```cpp
// SL_CharacterBase 或 EquipmentComponent 中
void ASL_CharacterBase::OnWeaponChanged()
{
    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!AnimInst) return;

    ASL_WeaponBase* Weapon = GetRightHandWeapon();
    USL_WeaponAnimSet* NewAnimSet = Weapon ? Weapon->GetWeaponAnimSet() : nullptr;

    // 设置 AnimBP 中的 WeaponAnimSet 变量
    // 方式A: 通过属性访问器（如果 AnimBP 暴露了 Set 方法）
    if (IWeaponAnimSetInterface* WSI = Cast<IWeaponAnimSetInterface>(AnimInst))
    {
        WSI->SetWeaponAnimSet(NewAnimSet);
    }
    // 方式B: 直接通过 SetParameters 设置
    // AnimInst->SetParameters(...)
}
```

> **注**：具体推送方式取决于 AnimBP 中的变量暴露策略。
> 简单做法是在 AnimBP 中创建一个 `Event SetWeaponAnimSet` 公开事件，
> 或使用 `UAnimInstance` 的蓝图可调用函数接口。

---

## 4 内容端工作

### 4.1 创建 WeaponAnimSet DataAsset 实例

在 Content Browser 中创建目录：

```
Content/SoulLikeDemo/Animation/WeaponAnimSets/
```

为每种武器类型创建一个 DataAsset 实例：

| Asset | 武器类型 | 关键内容 |
|-------|---------|---------|
| `WAS_Sword` | Sword | 剑的连招蒙太奇、待机 Additive |
| `WAS_GreatSword` | GreatSword | 大剑的连招蒙太奇、待机 Additive |
| `WAS_Dagger` | Dagger | 匕首的连招蒙太奇、待机 Additive |
| `WAS_Shield` | Shield | 盾牌的连招蒙太奇 |
| ... | ... | ... |

### 4.2 武器 DataTable 补充

在武器数据表（`DT_WeaponDataInfo`）中，为每行增加 `WeaponAnimSet` 列，指向对应的 Asset。

### 4.3 连招表按武器独立组织

**不再使用全局一张连招表**。每种武器创建独立的 `UComboInfoTable` DataTable 实例，各自包含该武器独有的窗口 Tag 和连招结构。

在 Content Browser 中创建目录：

```
Content/SoulLikeDemo/Data/ComboTables/
```

为每种武器类型创建一个 ComboInfoTable：

| 表 | 适用武器 | 说明 |
|----|---------|------|
| `CT_Sword` | Sword | 轻/重击三段连招 |
| `CT_GreatSword` | GreatSword | 蓄力斩三段 + 释放派生 |
| `CT_Katana` | Katana | 轻击连段 + 气刃斩开刃 |
| `CT_Dagger` | Dagger | 快速五段连击 |
| ... | ... | ... |

每张表的窗口 Tag 使用武器私有的命名空间，互不冲突：

**CT_GreatSword 示例行：**

| ActiveRequireWindowTag | InputActionType | OutputMontageTag | NextAbilityClass | ExecuteType | DamageMult |
|---|---|---|---|---|---|
| State.Window.None | Light | Gst.Charge_Start | GA_ComboAction | Charge | 1.0 |
| Gst.Charge_Ready | Light | Gst.Charge_L2 | GA_ComboAction | Charge | 1.0 |
| Gst.Charge_L2_Ready | Light | Gst.Charge_L3 | GA_ComboAction | Charge | 1.0 |
| Gst.Charge_Any_Release | Release | Gst.Slash_Strong | GA_ComboAction | Instant | 2.0 |
| Gst.Slash_Strong_End | Light | Gst.Charge_Start | GA_ComboAction | Charge | 1.0 |

**CT_Katana 示例行：**

| ActiveRequireWindowTag | InputActionType | OutputMontageTag | NextAbilityClass | ExecuteType | DamageMult |
|---|---|---|---|---|---|
| State.Window.None | Light | Ktn.Light_1 | GA_ComboAction | Instant | 1.0 |
| Ktn.Light_1_Release | Light | Ktn.Light_2 | GA_ComboAction | Instant | 1.2 |
| Ktn.Light_2_Release | Light | Ktn.Light_3 | GA_ComboAction | Instant | 1.5 |
| Ktn.Light_3_Release | Special | Ktn.Spirit_1 | GA_ComboAction | Instant | 2.0 |

每张表通过 `WeaponAnimSet.ComboInfoTable` 字段与武器关联。

### 4.4 窗口 Tag 的命名空间

窗口 Tag 不再需要全局注册一套统一标签。每种武器的 Tag 使用武器缩写作为前缀：

```
大剑: Gst.Charge_Ready, Gst.Charge_L2_Ready, Gst.Charge_Any_Release, ...
太刀: Ktn.Light_1_Release, Ktn.Light_3_Release, Ktn.Spirit_Start, ...
匕首: Dgr.Light_1_Release, Dgr.Light_2_Release, ...
```

这些 Tag 的添加/移除由各武器蒙太奇中的 AnimNotify 控制，互不干扰。

### 4.5 动画描述规范

在内容团队讨论动画资源时，使用「三维快照法」描述招式动作，便于AI理解姿态变化和计算连招衔接。

#### 4.5.1 三维快照法

描述一个招式动作只需要两张快照：

```
[发力快照] + [收力快照]
```

每张快照包含三个维度：

| 维度 | 取值 | 说明 |
|------|------|------|
| **躯干朝向** | 左转/右转 X 度 / 正向前倾 / 正面 | 身体旋转方向和角度 |
| **手位（前后）** | 左手身前/身后 + 右手身前/身后 | 每只手的正/背身位置 |
| **手位（高度）** | 腰高 / 肩高 / 头高 | 以人体为参考系的高度分级 |

#### 4.5.2 快照填写格式

```
发力快照：
  躯干：身体右转90度左右
  手位：左手身前,右手身后
  高度：左手肩高,右手头高

收力快照：
  躯干：身体左转90度左右
  手位：左手身前,右手身前
  高度：左手肩高,右手肩高
```

#### 4.5.3 连招匹配判断

两个招式能否流畅衔接，对比 **A的收力快照** 与 **B的发力快照**：

```
匹配度 = 躯干差 + 手位差 + 高度差
```

- **三项一致** → 完美衔接
- **一项变化** → 自然过渡（如同侧手位高度微调）
- **两项变化** → 需要 Blend 消化（如手位前后翻转+高度变化）
- **三项全变** → 衔接不自然，不建议直接配对

#### 4.5.4 示例

| 招式 | 发力快照 | 收力快照 |
|------|---------|---------|
| Attack13 左上起双斜斩 | 躯干左转90°,左手身后右手身前,左手头高右手肩高 | 躯干右转90°,双手身前,双手肩高 |
| Attack14 右上起双斜斩 | 躯干右转90°,左手身前右手身后,左手肩高右手头高 | 躯干左转90°,双手身前,双手肩高 |
| Attack15 跨两步右上起 | 躯干右转90°,双手身前 | 躯干左转90°,左手身后右手身前 |
| Attack29 右手上挑腾空 | 躯干右转130°,左手身前右手身后 | 躯干正向腾空状态 |

从快照可直观判断：
- **13→14**: 收力右转90°→发力右转90°(同向✓)，但双手身前→右手身后(手位翻转✗)，肩高→头高(抬升✗)
- **13→15**: 收力右转90°→发力右转90°(同向✓)，双手身前→双手身前(一致✓)

#### 4.5.5 从快照到动画文件名

蒙太奇文件名格式：

```
AM_{武器缩写}_{动作名}
```

示例：
- `Attack13` → `AM_Dbl_Light1`
- `Attack29` → `AM_Dbl_Uppercut`

蒙太奇的正式英文名由策划/程序根据游戏内招式名称决定，快照描述只用于AI理解动画和计算连招衔接。
> **未来扩展**：后续可改为直接输入发力截图和收力截图，由 AI 自动提取骨骼角度和手部坐标，
> 实现逐帧级的衔接匹配分析。当前文本版的三维快照法为此预留了同构的数据结构。


---

## 5 运行时全流程

```
拔刀 → 武器加载 → WeaponAnimSet 可通过 GetWeaponAnimSet() 访问
  ↓
角色 AnimBP 同步 WeaponAnimSet（用于 Additive Pose）
  ↓
Input (Light)
  → ComboManager::HandleInputPressed()
    → GetCurrentWeaponComboTable() → CT_Sword（武器专属连招表）
      → State.Window.None + Light
        → FComboInfo {
            OutputMontageTag: "Swd.Light_1",
            NextAbilityClass: GA_ComboAction,
            DamageMultiplier: 1.0,
            ...
          }
    → CurrentComboInfo = FComboInfo
    → ASC->TryActivateAbilityByClass(GA_ComboAction)
      → GA_ComboAction::ActivateAbility()
        → ComboMgr->ResolveCurrentMontage()
          → IWeaponAccessory_IF->GetRightHandWeapon()
            → Weapon->GetWeaponAnimSet()
              → ComboMontages["Swd.Light_1"]
                → AM_Sword_Light1 (UAnimMontage*)
        → UAbilityTask_ComboMontage::Create(AM_Sword_Light1)
          → 播放蒙太奇
            → 蒙太奇 Notify: 开启碰撞
            → 蒙太奇 Notify: 添加 Swd.Light_1_Release Tag（武器私有命名空间）
            → 蒙太奇 Notify: 到达 AllowBlend 位置
              → 等待输入...
```

---

## 6 改动清单

| # | 文件 | 改动类型 |
|---|------|---------|
| 1 | 新建 `Public/Data/SL_WeaponAnimSet.h` | 新增 |
| 2 | 新建 `Private/Data/SL_WeaponAnimSet.cpp` | 新增 |
| 3 | `Public/SoulLikeGameGlobal.h` — `FWeaponDataInfo` | 加 `WeaponAnimSet` 字段 |
| 4 | `Public/SoulLikeGameGlobal.h` — `FComboInfo` | 加 `OutputMontageTag` 字段 |
| 5 | `Public/Class/SL_WeaponBase.h` | 加 `GetWeaponAnimSet()` 声明 |
| 6 | `Private/Class/SL_WeaponBase.cpp` | 实现 `GetWeaponAnimSet()` + include |
| 7 | `Public/Component/Character/SL_ComboManagerComponent.h` | 加 `GetCurrentWeaponComboTable()` + `ResolveCurrentMontage()` 声明 |
| 8 | `Private/Component/Character/SL_ComboManagerComponent.cpp` | 实现两个方法 + 修改 `HandleInputPressed` 查表逻辑 + include |
| 9 | 新建 `Public/GAS/GA/GA_ComboAction.h` | 新增 |
| 10 | 新建 `Private/GAS/GA/GA_ComboAction.cpp` | 新增 |
| 11 | 角色主 AnimBP | 接入 `IdleAdditivePose` + Slot + AnimSet 变量 |
| 12 | 角色 C++（SL_CharacterBase / EquipmentComponent） | 拔出/切换武器时刷新 AnimBP 的 AnimSet |
| 13 | 内容端：`Content/.../WeaponAnimSets/WAS_*.uasset` | 每种武器创建一个 DataAsset 实例 |
| 14 | 内容端：`Content/.../Data/ComboTables/CT_*.uasset` | 每种武器创建独立的连招表 |
| 15 | 内容端：武器 DataTable | 补充 `WeaponAnimSet` 列 |
| 16 | 内容端：连招表 | 补充 `OutputMontageTag` 列，统一 `NextAbilityClass` |

---

## 7 与其他系统的关系

| 系统 | 是否受影响 | 说明 |
|------|-----------|------|
| 队伍系统（TeamID） | 无影响 | 完全正交 |
| 武器碰撞/伤害 | 无影响 | 伤害计算不变，蒙太奇 Notify 继续驱动碰撞检测 |
| 蓄力系统 | 兼容 | `FComboInfo` 的蓄力参数 + `OutputMontageTag` 指定蓄力释放时的蒙太奇 |
| 弹反系统 | 无影响 | 弹反窗口由 `ASL_WeaponBase::EnableParryWindow` 控制 |
| 受击/布娃娃 | 无影响 | `HitReactMontage` 在 AnimSet 中可选，不影响现有死亡/布娃娃逻辑 |
| 现有 `FWeaponDataInfo.AnimClass` | 无影响 | 该字段用于武器自身的挂载骨骼动画（如弓的 AnimBP），与连招蒙太奇无关 |

---

## 8 未涵盖范围

以下内容不在本次实施范围内，后续可单独讨论：

- **远程武器**（弓、法杖）：投射物生成、弹药管理
- **双持武器**：左右手独立连招
- **武器变形**（如 Charge Blade 剑/斧切换）：AnimSet 运行时切换
- **处决/背刺动画**：属于特殊动作系统，不通过常规连招表驱动
- **翻滚/垫步取消攻击**：连招窗口输入处理逻辑的增强
- **联机同步**：连招状态的网络复制

---

> 文档版本：v1.3
> 最后更新：2026-06-01
> 更新说明：v1.3 — 4.5 动画描述规范改为「三维快照法」，用发力/收力快照的躯干+手位+高度
> 三维数据进行连招衔接分析。
