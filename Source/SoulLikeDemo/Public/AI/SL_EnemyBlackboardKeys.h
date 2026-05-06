// Public/AI/SL_EnemyBlackboardKeys.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BlackboardData.h"
#include "SL_EnemyBlackboardKeys.generated.h"

/**
 * 敌人AI黑板键定义
 * 在蓝图创建黑板空间时使用这些键
 */
UCLASS()
class SOULLIKEDEMO_API USL_EnemyBlackboardKeys : public UObject
{
    GENERATED_BODY()

public:
    // ===== 目标相关 =====
    static const FName TargetActor;              // Object - 当前攻击目标
    static const FName TargetLocation;           // Vector - 目标位置
    static const FName LastKnownLocation;        // Vector - 最后已知位置
    static const FName bHasTarget;               // Bool - 是否有目标

    // ===== 状态相关 =====
    static const FName EnemyState;               // Int/Enum - 敌人状态
    static const FName bInCombat;                // Bool - 是否在战斗
    static const FName bInAttackRange;           // Bool - 是否在攻击范围
    static const FName bCanSeeTarget;            // Bool - 是否能看见目标

    // ===== 移动相关 =====
    static const FName PatrolPathIndex;          // Int - 巡逻点索引
    static const FName bIsMoving;                // Bool - 是否正在移动
    static const FName MoveToLocation;           // Vector - 移动目标位置

    // ===== 攻击相关 =====
    static const FName AttackCooldown;           // Float - 攻击冷却
    static const FName bCanAttack;               // Bool - 是否可以攻击
    static const FName SelectedAttackType;        // Int - 攻击类型

    // ===== 感知相关 =====
    static const FName TimeSinceLostTarget;      // Float - 失去目标时间
};

