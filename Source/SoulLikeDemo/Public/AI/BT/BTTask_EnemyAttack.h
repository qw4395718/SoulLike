// Public/AI/BTTask/BTTask_EnemyAttack.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_EnemyAttack.generated.h"

class USL_GameplayAbilityBase;
class UBehaviorTreeComponent;

UCLASS()
class SOULLIKEDEMO_API UBTTask_EnemyAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_EnemyAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
    virtual FString GetStaticDescription() const override;

protected:
    /** 激活敌人攻击能力所需的GameplayTag（对应SL_GameplayAbilityBase::ActivationTag） */
    UPROPERTY(EditAnywhere, Category = "GAS")
        FGameplayTag AttackAbilityTag;

    /** 目标黑板键 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
        struct FBlackboardKeySelector TargetActorKey;

    /** 是否在攻击前先移动到攻击范围 */
    UPROPERTY(EditAnywhere, Category = "Config")
        bool bMoveToAttackRange = true;

    /** 蒙太奇完成回调 */
    void OnMontageFinished();

    /** 清理绑定 */
    void ClearAbilityDelegate();

private:
    // 缓存的行为树组件（用于异步回调）
    UBehaviorTreeComponent* CachedOwnerComp;

    // 缓存的能力实例（用于绑定委托）
    TWeakObjectPtr<USL_GameplayAbilityBase> CachedAbilityInstance;

    // 委托句柄
    FDelegateHandle MontageDelegateHandle;
};