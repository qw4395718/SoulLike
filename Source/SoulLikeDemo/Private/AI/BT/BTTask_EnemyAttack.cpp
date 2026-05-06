// Private/AI/BTTask/BTTask_EnemyAttack.cpp
#include "BTTask_EnemyAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "SL_EnemyBase.h"
#include "SL_CharacterBase.h"

UBTTask_EnemyAttack::UBTTask_EnemyAttack()
{
    NodeName = TEXT("Enemy Attack (EQS)");
    bNotifyTick = false;
    bCreateNodeInstance = true;
    CachedBTComp = nullptr;
}

EBTNodeResult::Type UBTTask_EnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(
        OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!Target) return EBTNodeResult::Failed;

    // 如果有EQS验证查询，先运行EQS再攻击
    if (EQSValidateAttackQuery)
    {
        CachedBTComp = &OwnerComp;

        FEnvQueryRequest QueryRequest(EQSValidateAttackQuery, AIController->GetPawn());
        QueryRequest.Execute(EEnvQueryRunMode::SingleResult,
            FQueryFinishedSignature::CreateUObject(this, &UBTTask_EnemyAttack::OnEQSQueryFinished));

        // 保持运行中，等待EQS回调
        return EBTNodeResult::InProgress;
    }

    // 没有EQS验证，直接攻击
    PerformAttack(AIController, Target);
    return EBTNodeResult::Succeeded;
}

void UBTTask_EnemyAttack::OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    if (!CachedBTComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_EnemyAttack: BTComp is null in callback"));
        return;
    }

    AAIController* AIController = CachedBTComp->GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(*CachedBTComp, EBTNodeResult::Failed);
        return;
    }

    AActor* Target = Cast<AActor>(
        CachedBTComp->GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!Target)
    {
        FinishLatentTask(*CachedBTComp, EBTNodeResult::Failed);
        return;
    }

    // 如果EQS验证通过，执行攻击
    PerformAttack(AIController, Target);
    FinishLatentTask(*CachedBTComp, EBTNodeResult::Succeeded);
}

void UBTTask_EnemyAttack::PerformAttack(AAIController* AIController, AActor* Target)
{
    ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(AIController->GetPawn());
    if (!Enemy) return;

    // 面向目标
    FVector Dir = Target->GetActorLocation() - Enemy->GetActorLocation();
    Dir.Z = 0;
    if (Dir.SizeSquared() > 0.01f)
    {
        FRotator TargetRot = Dir.Rotation();
        Enemy->SetActorRotation(TargetRot);
    }

    // 触发攻击动画（需要通过GAS Ability或Montage播放）
    UAnimInstance* AnimInstance = Enemy->GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        // TODO: 从EnemyConfig中获取攻击Montage播放
        // AnimInstance->Montage_Play(AttackMontage);
        // AnimInstance->Montage_JumpToSection(AttackSectionName);
    }

    UE_LOG(LogTemp, Verbose, TEXT("Enemy %s attacked target %s"), *Enemy->GetName(), *Target->GetName());
}

EBTNodeResult::Type UBTTask_EnemyAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    CachedBTComp = nullptr;
    return EBTNodeResult::Aborted;
}

FString UBTTask_EnemyAttack::GetStaticDescription() const
{
    if (EQSValidateAttackQuery)
    {
        return FString::Printf(TEXT("Attack with EQS: %s, Section: %s"),
            *EQSValidateAttackQuery->GetName(), *AttackSectionName.ToString());
    }
    return FString::Printf(TEXT("Attack (No EQS), Section: %s"), *AttackSectionName.ToString());
}