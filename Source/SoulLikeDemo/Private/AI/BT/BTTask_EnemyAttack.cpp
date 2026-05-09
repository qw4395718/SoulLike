// Private/AI/BTTask/BTTask_EnemyAttack.cpp
#include "BTTask_EnemyAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "SL_EnemyBase.h"
#include "SL_AbilitySystemComponent.h"
#include "SL_CharacterBase.h"
#include <SL_GameplayAbilityNPCBase.h>

UBTTask_EnemyAttack::UBTTask_EnemyAttack()
{
    NodeName = TEXT("Enemy Attack (GAS)");
    bNotifyTick = false;          // 无需Tick来轮询
    bCreateNodeInstance = true;  // 每个实例独立
    CachedOwnerComp = nullptr;
}

EBTNodeResult::Type UBTTask_EnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(AIController->GetPawn());
    if (!Enemy) return EBTNodeResult::Failed;

    // 检查攻击Tag是否有效
    if (!AttackAbilityTag.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UBTTask_EnemyAttack: AttackAbilityTag is not set!"));
        return EBTNodeResult::Failed;
    }

    // 获取目标的GAS组件
    IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Enemy);
    if (!ASI) return EBTNodeResult::Failed;

    USL_AbilitySystemComponent* ASC = Cast<USL_AbilitySystemComponent>(ASI->GetAbilitySystemComponent());
    if (!ASC) return EBTNodeResult::Failed;

    // ===== 面向目标 =====
    AActor* Target = Cast<AActor>(
        OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (Target)
    {
        FVector Dir = Target->GetActorLocation() - Enemy->GetActorLocation();
        Dir.Z = 0;
        if (Dir.SizeSquared() > 0.01f)
        {
            Enemy->SetActorRotation(Dir.Rotation());
        }
    }

    // ===== 通过Tag激活能力 =====
    bool bActivated = ASC->TryActivateAbilityByTag(AttackAbilityTag);
    
    if (!bActivated)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_EnemyAttack: Failed to activate ability with tag %s"), 
            *AttackAbilityTag.ToString());
        return EBTNodeResult::Failed;
    }

    UE_LOG(LogTemp, Verbose, TEXT("UBTTask_EnemyAttack: Activated ability %s"), *AttackAbilityTag.ToString());

    // ===== 缓存行为树组件（用于后续回调） =====
    CachedOwnerComp = &OwnerComp;

    // ===== 绑定蒙太奇完成委托 =====
    // 延迟一帧绑定，确保Ability已经激活
    FTimerHandle DummyHandle;
    Enemy->GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this, ASC]()
    {
        // 通过Tag获取刚激活的能力实例
        if (USL_GameplayAbilityNPCBase* AbilityInstance = Cast<USL_GameplayAbilityNPCBase>(ASC->GetActiveAbilityInstanceByTag(AttackAbilityTag)))
        {
            CachedAbilityInstance = AbilityInstance;
            
            // 绑定蒙太奇完成事件
            MontageDelegateHandle = AbilityInstance->OnMontageCompletedDelegate.AddUObject(
                this, &UBTTask_EnemyAttack::OnMontageFinished);
            
            UE_LOG(LogTemp, Verbose, TEXT("UBTTask_EnemyAttack: Bound to ability %s montage event"), 
                *AttackAbilityTag.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UBTTask_EnemyAttack: Ability instance not found immediately after activation"));
        }
    }));

    // 保持InProgress状态，等待蒙太奇完成或Tick检测
    return EBTNodeResult::InProgress;
}

void UBTTask_EnemyAttack::OnMontageFinished()
{
    UE_LOG(LogTemp, Verbose, TEXT("UBTTask_EnemyAttack: Montage finished callback received"));

    ClearAbilityDelegate();

    if (CachedOwnerComp)
    {
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
        CachedOwnerComp = nullptr;
    }
}

void UBTTask_EnemyAttack::ClearAbilityDelegate()
{
    if (MontageDelegateHandle.IsValid() && CachedAbilityInstance.IsValid())
    {
        CachedAbilityInstance->OnMontageCompletedDelegate.Remove(MontageDelegateHandle);
        MontageDelegateHandle.Reset();
    }
    CachedAbilityInstance = nullptr;
}

EBTNodeResult::Type UBTTask_EnemyAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // ===== 当行为树被中断时，清理所有绑定 =====
    ClearAbilityDelegate();

    // ===== 尝试取消正在运行的能力 =====
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(AIController->GetPawn());
        if (Enemy)
        {
            IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Enemy);
            if (ASI)
            {
                USL_AbilitySystemComponent* ASC = Cast<USL_AbilitySystemComponent>(ASI->GetAbilitySystemComponent());
                if (ASC)
                {
                    ASC->TryActivateAbilityByTag(AttackAbilityTag);
                }
            }
        }
    }

    CachedOwnerComp = nullptr;
    return EBTNodeResult::Aborted;
}

void UBTTask_EnemyAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    // 确保清理
    ClearAbilityDelegate();
}

FString UBTTask_EnemyAttack::GetStaticDescription() const
{
    if (AttackAbilityTag.IsValid())
    {
        return FString::Printf(TEXT("Attack via GAS: Tag=%s"), *AttackAbilityTag.ToString());
    }
    return TEXT("Attack via GAS: No Tag Set!");
}