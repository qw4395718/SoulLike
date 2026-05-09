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
    static const FName bHasTarget;                  // Bool - 是否有目标
    static const FName TargetActor;                 // Object - 当前攻击目标
    static const FName TargetLocation;              // Vector - 目标位置
    static const FName DistanceToTarget;            // Float - 目标距离
    static const FName bInAttackRange;              // Bool - 是否在攻击范围内
    static const FName bHasSuspectedLocation;        // Bool - 是否有可疑位置
    static const FName SuspectedLocation;            // Vector - 可疑位置
    static const FName bInChaseRange;                // Bool - 是否在追击范围内
    
    // 状态相关
    static const FName bIsDead;                      // Bool - 是否死亡

};

