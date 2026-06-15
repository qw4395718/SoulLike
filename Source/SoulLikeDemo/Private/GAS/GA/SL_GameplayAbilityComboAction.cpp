#include "SL_GameplayAbilityComboAction.h"
#include "AbilitySystemComponent.h"
#include <SL_ComboManagerComponent.h>
#include <AT/AbilityTask_ComboMontage.h>

#include "Stats/Stats.h"

DECLARE_CYCLE_STAT(TEXT("ActivateAbility"), STAT_ComboAction_ActivateAbility, STATGROUP_Game);

void USL_GameplayAbilityComboAction::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    SCOPE_CYCLE_COUNTER(STAT_ComboAction_ActivateAbility);
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    USL_ComboManagerComponent* ComboMgr =
        Avatar->FindComponentByClass<USL_ComboManagerComponent>();
    if (!ComboMgr)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 通过 WeaponAnimSet 解析蒙太奇
    UAnimMontage* Montage = ComboMgr->ResolveCurrentMontage();
    if (!Montage)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 获取连招接续入口时间
    float ComboEntryTime = ComboMgr->GetCurrentComboInfo().ComboEntryTime;

    // 创建 ComboMontage Task（传入入口时间，跳过Idle段）
    ComboTask = UAbilityTask_ComboMontage::CreateComboMontageTask(
        this, Montage, 0.2f, 1.0f, ComboEntryTime);
    if (!ComboTask)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 注册到 ComboManager
    ComboMgr->RegisterActiveComboTask(ComboTask);
    ComboMgr->ClearTargetWindowTag();

    // 绑定回调
    ComboTask->OnCompleted.AddDynamic(this, &USL_GameplayAbilityComboAction::OnComboMontageCompleted);
    ComboTask->OnInterrupted.AddDynamic(this, &USL_GameplayAbilityComboAction::OnComboMontageInterrupted);

    // 启动 Task
    ComboTask->ReadyForActivation();
}

void USL_GameplayAbilityComboAction::CancelAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateCancelAbility)
{
    if (ComboTask && ComboTask->IsActive())
    {
        ComboTask->EndTask();
    }
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void USL_GameplayAbilityComboAction::OnComboMontageCompleted()
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

    bool bReplicateEnd = false;
    bool bWasCancelled = false;
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEnd, bWasCancelled);
}

void USL_GameplayAbilityComboAction::OnComboMontageInterrupted()
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

    bool bReplicateEnd = false;
    bool bWasCancelled = true;
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEnd, bWasCancelled);
}