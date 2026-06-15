# 部位破坏与掉落物系统设计

> 目标项目：SoulLike (UE 4.26)
> 参考原型：《怪物猎人》系列部位破坏机制
> 适用场景：大型敌人/Boss 战的部位破坏、素材掉落
> 创建日期：2026-06-15

---

## 目录

1. [系统概述](#1-系统概述)
2. [核心数据结构](#2-核心数据结构)
3. [部位破坏流程](#3-部位破坏流程)
4. [掉落物系统](#4-掉落物系统)
5. [可视化反馈](#5-可视化反馈)
6. [与现有系统的集成](#6-与现有系统的集成)
7. [分阶段实施建议](#7-分阶段实施建议)

---

## 1 系统概述

### 1.1 核心玩法

```
玩家攻击敌人特定部位
  -> 积累对该部位的"破坏值"
    -> 达到阈值 -> 部位破坏触发
      -> 视觉变化（模型切换/裂纹材质/断裂）
      -> 行为变化（攻击模式改变/移动变慢）
      -> 掉落物生成（素材掉落）
```

### 1.2 与类魂系统的融合

| 怪物猎人特性 | 类魂适配方案 |
|-------------|-------------|
| 全身独立部位血量 | 仅关键部位（头、尾、手、弱点）可破坏 |
| 断尾改变攻击范围 | 断尾后尾部攻击消失，硬直窗口变长 |
| 破头降低伤害 | 破头后部分招式削弱 |
| 掉落物拾取 | 掉落物类似魂系"道具光点"，走过去拾取 |
| 剥取素材 | 敌人死亡后靠近按交互键拾取尸体 |

### 1.3 可破坏部位类型

| 部位 | 破坏效果 | 视觉变化 | 行为影响 |
|------|---------|---------|---------|
| 头部 | 破角/破面 | 角折断/疤痕 | 部分招式削弱，硬直增加 |
| 尾部 | 断尾 | 尾巴消失 | 尾部攻击移除，攻击范围缩小 |
| 左/右手 | 破手 | 手上装甲碎裂 | 对应手部攻击力下降 |
| 腿部 | 破腿 | 腿部裂纹 | 移动速度下降，踉跄概率增加 |
| 背部/甲壳 | 破甲 | 甲壳碎裂 | 该区域受击伤害增加 |
| 弱点 | 击破 | 特效显示 | 大硬直，高伤害 |

---

## 2 核心数据结构

### 2.1 部位配置

```cpp
// 敌人配置中的部位数据
USTRUCT(BlueprintType)
struct FPartBreakConfig
{
    GENERATED_BODY()

    // 部位标识
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PartID;                           // 如 "Head", "Tail", "L_Arm"

    // 破坏条件
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BreakThreshold;                   // 需要累积的破坏值

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanOnlyBreakOnce = true;          // 是否只能破坏一次

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BreakLevelCount = 1;              // 破坏阶段数（Level 1 = 裂纹, Level 2 = 断裂）

    // 破坏触发时的伤害
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BreakBonusDamage = 50.0f;         // 破坏瞬间额外伤害

    // 受击伤害倍率（正常 = 1.0，弱点 = 1.5，护甲 = 0.5）
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageMultiplier = 1.0f;

    // 破坏后伤害倍率变化
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PostBreakDamageMultiplier = 1.5f; // 破甲后该部位受伤更重

    // 掉落物
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDropItemInfo> DropItems;        // 破坏瞬间掉落的道具

    // 视觉
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> BrokenMesh;      // 破坏后的模型

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMaterialInterface> BrokenMaterial; // 破裂材质

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystem* BreakVFX;                     // 破坏特效

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* BreakSound;                        // 破坏音效
};
```

### 2.2 运行时部位状态

```cpp
// 运行时追踪每个部位的破坏状态
USTRUCT()
struct FPartBreakState
{
    GENERATED_BODY()

    UPROPERTY()
    FName PartID;

    // 已累积的破坏值（伤害 + 特殊破防攻击）
    float AccumulatedDamage = 0.0f;

    // 当前破坏阶段（0=未破坏, 1=一级破坏, 2=二级破坏）
    int32 CurrentBreakLevel = 0;

    // 是否已完全破坏
    bool bIsFullyBroken = false;

    // 是否启用破坏后伤害倍率
    bool bUsePostBreakMultiplier = false;
};
```

### 2.3 掉落物定义

```cpp
// 掉落物条目
USTRUCT(BlueprintType)
struct FDropItemInfo
{
    GENERATED_BODY()

    // 掉落道具ID（对应道具数据表中的 ItemID）
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;

    // 掉落数量
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 1;

    // 掉落概率（0.0~1.0）
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DropProbability = 1.0f;

    // 掉落类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDropType DropType = EDropType::OnBreak;  // 破坏时掉落 / 死亡时拾取
};

UENUM(BlueprintType)
enum class EDropType : uint8
{
    OnBreak,        // 部位破坏瞬间掉落
    OnCarve,        // 敌人体力归零后可剥取
    OnDeath         // 死亡时自动掉落
};
```

### 2.4 敌人整体配置

```cpp
// 放在 FEnemyConfigInfo 中扩展
struct FEnemyConfigInfo
{
    // ... 已有字段

    // 部位破坏配置列表
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FPartBreakConfig> PartBreakConfigs;

    // 死亡剥取列表
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDropItemInfo> CarveItems;
};
```

---

## 3 部位破坏流程

### 3.1 伤害路由

现有武器碰撞系统 `ApplyDamageToOverlappingActors` 中已经计算伤害。需要在该流程中注入部位破坏系统：

```
ApplyDamageToOverlappingActors
  -> CalculateFinalDamage (已有)
  -> 确定命中部位（通过骨骼名称/物理材质）
  -> AccumulatePartDamage(PartID, FinalDamage)
    -> 检查是否达到阈值
      -> 触发 PartBreak(PartID, CurrentLevel)
```

### 3.2 实现方案

**方案 A：基于物理材质（推荐，与现有系统兼容）**

在武器碰撞的 HitResult 中获取命中骨骼，通过骨骼名称映射到配置表中的 PartID：

```cpp
void ASL_WeaponBase::ApplyDamageToOverlappingActors()
{
    // ... 现有循环

    for (AActor* Actor : ActorsCopy)
    {
        // ... 现有伤害计算
        
        // 新增：部位破坏值积累
        if (ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(Actor))
        {
            // 从碰撞结果获取命中骨骼
            FName HitBoneName = GetHitBoneName(SweepResult);
            Enemy->AccumulatePartDamage(HitBoneName, FinalDamage);
        }

        // ... 应用 GE
    }
}
```

**方案 B：基于物理材质（简便，不需要骨骼匹配）**

在敌人蒙太奇中通过动画通知标记当前攻击窗口对应的部位。攻击窗口开启时积累的伤害计入该部位。

```cpp
// 在 EnemyBase 端追踪当前暴露部位
void ASL_EnemyBase::SetExposedPart(FName InPartID)
{
    CurrentExposedPart = InPartID;  // 由动画通知调用
}

void ASL_EnemyBase::AccumulatePartDamage(float InDamage)
{
    if (!CurrentExposedPart.IsNone())
    {
        PartStates[CurrentExposedPart].AccumulatedDamage += InDamage;
        CheckPartBreak(CurrentExposedPart);
    }
}
```

推荐方案 A，因为它不需要动画通知的额外编排，直接根据武器实际命中点计算。

### 3.3 破坏判定

```cpp
void ASL_EnemyBase::CheckPartBreak(FName InPartID)
{
    FPartBreakState& State = PartStates[InPartID];
    FPartBreakConfig* Config = FindPartConfig(InPartID);
    if (!Config) return;

    if (State.bIsFullyBroken && Config->bCanOnlyBreakOnce) return;

    // 检查是否达到下一破坏阶段
    float RequiredDamage = Config->BreakThreshold * (State.CurrentBreakLevel + 1);
    if (State.AccumulatedDamage >= RequiredDamage)
    {
        // 触发破坏
        ApplyPartBreak(InPartID, State.CurrentBreakLevel + 1);
    }
}

void ASL_EnemyBase::ApplyPartBreak(FName InPartID, int32 InNewLevel)
{
    FPartBreakState& State = PartStates[InPartID];
    FPartBreakConfig* Config = FindPartConfig(InPartID);

    State.CurrentBreakLevel = InNewLevel;
    if (InNewLevel >= Config->BreakLevelCount)
    {
        State.bIsFullyBroken = true;
        State.bUsePostBreakMultiplier = true;
    }

    // 1. 播放破坏特效和音效（Multicast RPC）
    Multicast_PlayBreakEffect(InPartID, Config->BreakVFX, Config->BreakSound);

    // 2. 切换模型/材质
    ApplyBreakVisual(InPartID, Config);

    // 3. 造成额外伤害
    ApplyBreakBonusDamage(Config->BreakBonusDamage);

    // 4. 生成掉落物
    SpawnBreakDrops(Config->DropItems);

    // 5. 广播破坏事件（供行为树/AI 响应）
    OnPartBroken.Broadcast(InPartID, InNewLevel);
}
```

---

## 4 掉落物系统

### 4.1 掉落物 Actor

```cpp
UCLASS()
class ASL_DropItemActor : public AActor
{
    GENERATED_BODY()

public:
    // 初始化
    void InitializeDrop(FName InItemID, int32 InCount);

    // 交互拾取
    UFUNCTION()
    void OnInteract(APlayerController* InPC);

protected:
    UPROPERTY(ReplicatedUsing = OnRep_Visual)
    FDropItemVisual VisualData;

    UPROPERTY()
    UStaticMeshComponent* MeshComp;

    UPROPERTY()
    UWidgetComponent* NameplateWidget;  // 显示道具名称 + "按E拾取"

    // 拾取光效
    UPROPERTY()
    UPointLightComponent* GlowLight;

    // 拾取半径
    float PickupRadius = 150.0f;

    // 自动消失时间
    float LifeTime = 60.0f;
};
```

### 4.2 掉落物生成

```cpp
void ASL_EnemyBase::SpawnBreakDrops(const TArray<FDropItemInfo>& InDropItems)
{
    if (!HasAuthority()) return;

    for (const FDropItemInfo& DropInfo : InDropItems)
    {
        if (FMath::FRand() > DropInfo.DropProbability) continue;

        // 生成掉落物 Actor
        FTransform SpawnTransform = GetDropSpawnTransform();  // 在破坏位置附近
        ASL_DropItemActor* Drop = GetWorld()->SpawnActor<ASL_DropItemActor>(
            DropItemActorClass, SpawnTransform);
        
        if (Drop)
        {
            Drop->InitializeDrop(DropInfo.ItemID, DropInfo.Count);
            Drop->SetLifeSpan(60.0f);  // 60秒后消失
        }
    }
}
```

### 4.3 死亡剥取

```cpp
void ASL_EnemyBase::OnEnemyDied()
{
    // 敌人死亡后，在尸体旁生成可剥取的光点
    // 玩家靠近按交互键拾取
    // 可配置多次剥取（最多 3 次）
    // 每次随机从配置表中取数条
}
```

### 4.4 掉落物视觉

| 类型 | 表现 |
|------|------|
| 普通素材 | 白色光点 + 道具 3D 模型 |
| 稀有素材 | 金色光点 + 缓慢旋转 + 光柱 |
| 部位破坏 | 破坏瞬间从该部位飞出多个光点 |
| 死亡剥取 | 尸体旁出现可交互光点 |

---

## 5 可视化反馈

### 5.1 破坏过程视觉

```
破坏前                       一级破坏                     二级破坏
[完整模型]                  [裂纹材质]                  [断裂模型]
   |                            |                            |
   |--- 累计伤害达到 50% ---->  |--- 累计伤害达到 100% ---->  |
                              裂纹纹理叠加                 替换为断裂Mesh
                              边缘泛红光                   掉落物飞溅
```

### 5.2 破坏瞬间反馈

- 屏幕中央出现 "XX 部位破坏！" 提示文字
- 短暂慢动作（0.3s Time Dilation）
- 掉落物从破坏处飞散（物理模拟）
- 破坏音效 + 粒子特效

### 5.3 掉落物拾取 UI

- 靠近掉落物时显示 "按 E 拾取 XX" 的交互提示
- 拾取后显示获取动画（道具图标飞入背包）
- 拾取列表：一次拾取显示本次获得的所有道具

---

## 6 与现有系统的集成

### 6.1 现有武器碰撞系统

当前的 `ASL_WeaponBase::ApplyDamageToOverlappingActors` 中，伤害计算流程需要扩展：

```cpp
// 当前：
FinalDamage -> GAS GE -> TargetASC

// 扩展后：
FinalDamage
  -> AccumulatePartDamage(HitBone, FinalDamage)  // 新增
  -> GAS GE -> TargetASC                           // 现有
```

需要从 SweepResult 中获取命中骨骼信息。如果 SweepResult 不可用，可以使用物理材质（PhysicalMaterial）来匹配部位。

### 6.2 现有伤害 GE 系统

部位破坏的额外伤害可以直接使用现有的 `DamageEffectClass`（GE_Damage）进行应用，复用已有的 GAS 管道：

```cpp
void ApplyBreakBonusDamage(float InBonusDamage)
{
    // 复用现有的 DamageEffectClass
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);
    Spec.Data->SetSetByCallerMagnitude(DamageTag, InBonusDamage);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}
```

### 6.3 现有 AI 行为树

部位破坏事件应该通知行为树，以便 AI 做出响应：

```
OnPartBroken 委托
  -> BehaviorTree 的 Service/Task 监听
    -> 切换到"愤怒"阶段（攻击频率提升）
    -> 移除被破坏部位对应的攻击招式
    -> 增加新的硬直行为
```

### 6.4 现有掉落物/道具系统

- 掉落物拾取后调用 `InventoryComponent->AddItemByID()`
- 道具数据表已存在，掉落物直接引用现有的 ItemID

---

## 7 分阶段实施建议

### Phase 1 -- 基础框架

- 创建 `FPartBreakConfig` / `FPartBreakState` 数据结构
- 在 `ASL_EnemyBase` 中添加部位破坏管理器
- 实现骨骼命中检测（从武器 SweepResult 获取骨骼名）
- 破坏触发后切换材质/模型

### Phase 2 -- 掉落物系统

- 创建 `ASL_DropItemActor` 掉落物 Actor
- 实现掉落物生成、拾取交互、消失
- 破坏瞬间掉落物飞散
- 死亡剥取交互

### Phase 3 -- 完善与打磨

- 破坏瞬间慢动作 + 屏幕提示
- 掉落物稀有度（颜色/光效分级）
- AI 行为响应（愤怒/狂暴阶段）
- 部位独立受击音效

### Phase 4 -- 大型 Boss 专用

- 多阶段破坏（外甲 -> 核心 -> 击破）
- 部位再生（某些 Boss 可修复部位）
- 部位独立锁定（玩家可锁定特定部位）
- 采集动画（剥取时播放固定动画）

---

> 文档版本：v1.0
> 最后更新：2026-06-15
> 状态：待实施
