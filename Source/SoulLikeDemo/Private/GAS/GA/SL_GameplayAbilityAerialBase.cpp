#include "SL_GameplayAbilityAerialBase.h"
#include "SL_CharacterBase.h"
#include "SL_ComboManagerComponent.h"
#include "AT/AbilityTask_Aerial.h"

USL_GameplayAbilityAerialBase::USL_GameplayAbilityAerialBase()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USL_GameplayAbilityAerialBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	StartAerialTaskFromComboInfo();
	if (!AerialTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ASL_CharacterBase* Character = Cast<ASL_CharacterBase>(GetAvatarActorFromActorInfo()))
	{
		Character->SetAerialTask(AerialTask);
	}
}

void USL_GameplayAbilityAerialBase::StartAerialTaskFromComboInfo()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	USL_ComboManagerComponent* ComboMgr = Avatar->FindComponentByClass<USL_ComboManagerComponent>();
	if (!ComboMgr) return;

	const FComboInfo& Info = ComboMgr->GetCurrentComboInfo();

	UAnimMontage* LaunchMontage = ComboMgr->ResolveMontageByTag(Info.OutputMontageTag);
	UAnimMontage* DiveMontage = ComboMgr->ResolveMontageByTag(Info.DiveMontageTag);
	UAnimMontage* LandingMontage = ComboMgr->ResolveMontageByTag(Info.GroundFollowUpMontageTag);
	float AirTime = Info.MaxAirTime;

	if (!LaunchMontage && !DiveMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Aerial] No montage found for aerial combo, skipping"));
		return;
	}

	AerialTask = UAbilityTask_Aerial::CreateAerialTask(
		this,
		LaunchMontage,
		DiveMontage,
		LandingMontage,
		AirTime);

	if (!AerialTask) return;

	AerialTask->OnLaunchComplete.AddDynamic(this, &USL_GameplayAbilityAerialBase::OnLaunchMontageComplete);
	AerialTask->OnAirLoopStart.AddDynamic(this, &USL_GameplayAbilityAerialBase::OnEnterAirLoop);
	AerialTask->OnDiveStart.AddDynamic(this, &USL_GameplayAbilityAerialBase::OnEnterDive);
	AerialTask->OnAerialEnd.AddDynamic(this, &USL_GameplayAbilityAerialBase::OnAerialFinished);

	AerialTask->ReadyForActivation();
}

void USL_GameplayAbilityAerialBase::RequestDive()
{
	if (AerialTask && AerialTask->IsActive())
	{
		AerialTask->RequestDive();
	}
}

void USL_GameplayAbilityAerialBase::OnLaunchMontageComplete()
{
}

void USL_GameplayAbilityAerialBase::OnEnterAirLoop()
{
	OnAirLoopStarted();
}

void USL_GameplayAbilityAerialBase::OnEnterDive()
{
	OnDiveStarted();
}

void USL_GameplayAbilityAerialBase::OnAerialFinished()
{
	OnAerialEnded();

	bool bReplicateEnd = true;
	bool bWasCancelled = false;
	if (ASL_CharacterBase* Character = Cast<ASL_CharacterBase>(GetAvatarActorFromActorInfo()))
	{
		Character->ClearAerialTask();
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEnd, bWasCancelled);
}
