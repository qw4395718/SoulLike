#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SL_GameplayAbilityComboAction.generated.h"

UCLASS()
class SOULLIKEDEMO_API USL_GameplayAbilityComboAction : public UGameplayAbility
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
