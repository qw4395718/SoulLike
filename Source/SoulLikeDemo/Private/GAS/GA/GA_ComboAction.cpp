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

    // 创建 ComboMontage Task
    ComboTask = UAbilityTask_ComboMontage::CreateComboMontageTask(
        this, Montage, 0.2f, 1.0f);
    if (!ComboTask)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 注册到 ComboManager
    ComboMgr->RegisterActiveComboTask(ComboTask);

    // 绑定回调
    ComboTask->OnCompleted.AddDynamic(this, &UGA_ComboAction::OnComboMontageCompleted);
    ComboTask->OnInterrupted.AddDynamic(this, &UGA_ComboAction::OnComboMontageInterrupted);

    // 启动 Task
    ComboTask->ReadyForActivation();
}

void UGA_ComboAction::CancelAbility(
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

    bool bReplicateEnd = false;
    bool bWasCancelled = false;
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEnd, bWasCancelled);
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

    bool bReplicateEnd = false;
    bool bWasCancelled = true;
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEnd, bWasCancelled);
}
