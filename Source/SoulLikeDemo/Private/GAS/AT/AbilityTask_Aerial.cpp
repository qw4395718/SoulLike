#include "AbilityTask_Aerial.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"

UAbilityTask_Aerial* UAbilityTask_Aerial::CreateAerialTask(
	UGameplayAbility* OwningAbility,
	UAnimMontage* InLaunchMontage,
	UAnimMontage* InDiveMontage,
	UAnimMontage* InLandingMontage,
	float InMaxAirTime,
	float InLaunchVelocityZ,
	float InPlayRate)
{
	UAbilityTask_Aerial* Task = NewAbilityTask<UAbilityTask_Aerial>(OwningAbility);
	Task->LaunchMontage = InLaunchMontage;
	Task->DiveMontage = InDiveMontage;
	Task->LandingMontage = InLandingMontage;
	Task->MaxAirTime = InMaxAirTime;
	Task->LaunchVelocityZ = InLaunchVelocityZ;
	Task->PlayRate = InPlayRate;
	Task->CurrentPhase = ELaunchType::Launch;
	return Task;
}

void UAbilityTask_Aerial::Activate()
{
	Super::Activate();

	if (!Ability)
	{
		EndTask();
		return;
	}

	CachedCharacter = Cast<ACharacter>(GetAvatarActor());
	if (!CachedCharacter.IsValid())
	{
		EndTask();
		return;
	}

	CachedASC = Ability->GetAbilitySystemComponentFromActorInfo();

	LaunchPhase();
}

void UAbilityTask_Aerial::OnDestroy(bool bInOwnerFinished)
{
	if (AirTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AirTimerHandle);
	}

	UCharacterMovementComponent* MoveComp = GetMovementComp();
	if (MoveComp && MoveComp->GravityScale < 1.0f)
	{
		MoveComp->GravityScale = 1.0f;
	}

	Super::OnDestroy(bInOwnerFinished);
}

// ==================== Lifecycle ====================

void UAbilityTask_Aerial::LaunchPhase()
{
	CurrentPhase = ELaunchType::Launch;

	UCharacterMovementComponent* MoveComp = GetMovementComp();
	if (MoveComp)
	{
		MoveComp->GravityScale = 0.0f;
		MoveComp->SetMovementMode(MOVE_Falling);
	}

	// Apply launch upward velocity if set
	if (LaunchVelocityZ > 0.0f && MoveComp)
	{
		FVector Vel = MoveComp->Velocity;
		Vel.Z = LaunchVelocityZ;
		MoveComp->Velocity = Vel;
	}

	if (!LaunchMontage)
	{
		AirLoopPhase();
		return;
	}

	UAnimInstance* AnimInst = GetAnimInstance();
	if (!AnimInst || !CachedASC.IsValid())
	{
		AirLoopPhase();
		return;
	}

	CachedASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), LaunchMontage, PlayRate);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UAbilityTask_Aerial::OnLaunchMontageEnded);
	AnimInst->Montage_SetEndDelegate(EndDelegate, LaunchMontage);
}

void UAbilityTask_Aerial::AirLoopPhase()
{
	CurrentPhase = ELaunchType::AirLoop;

	UCharacterMovementComponent* MoveComp = GetMovementComp();
	if (MoveComp)
	{
		MoveComp->GravityScale = 1.0f;
	}

	OnAirLoopStart.Broadcast();

	if (GetWorld() && MaxAirTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(AirTimerHandle, this,
			&UAbilityTask_Aerial::OnAirTimeOut, MaxAirTime, false);
	}
}

void UAbilityTask_Aerial::DivePhase()
{
	CurrentPhase = ELaunchType::Dive;

	if (AirTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AirTimerHandle);
	}

	UCharacterMovementComponent* MoveComp = GetMovementComp();
	if (MoveComp)
	{
		if (MoveComp->GravityScale < 1.0f)
		{
			MoveComp->GravityScale = 1.0f;
		}
		MoveComp->SetMovementMode(MOVE_Falling);
	}

	OnDiveStart.Broadcast();

	if (DiveMontage)
	{
		UAnimInstance* AnimInst = GetAnimInstance();
		if (AnimInst && CachedASC.IsValid())
		{
			CachedASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), DiveMontage, PlayRate);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UAbilityTask_Aerial::OnDiveMontageEnded);
			AnimInst->Montage_SetEndDelegate(EndDelegate, DiveMontage);
		}
	}
}

void UAbilityTask_Aerial::FinishAerial()
{
	UAnimInstance* AnimInst = GetAnimInstance();
	if (AnimInst)
	{
		if (LaunchMontage && AnimInst->Montage_IsPlaying(LaunchMontage))
			AnimInst->Montage_Stop(0.15f, LaunchMontage);
		if (DiveMontage && AnimInst->Montage_IsPlaying(DiveMontage))
			AnimInst->Montage_Stop(0.15f, DiveMontage);
	}

	UCharacterMovementComponent* MoveComp = GetMovementComp();
	if (MoveComp)
	{
		MoveComp->GravityScale = 1.0f;
		if (MoveComp->MovementMode != MOVE_Walking)
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}
	}

	if (LandingMontage && AnimInst && CachedASC.IsValid())
	{
		CachedASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), LandingMontage, PlayRate);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UAbilityTask_Aerial::OnLandingMontageEnded);
		AnimInst->Montage_SetEndDelegate(EndDelegate, LandingMontage);
	}
	else
	{
		OnAerialEnd.Broadcast();
		EndTask();
	}
}

void UAbilityTask_Aerial::RequestDive()
{
	if (CurrentPhase == ELaunchType::AirLoop)
	{
		DivePhase();
	}
}

void UAbilityTask_Aerial::OnLanded()
{
	if (CurrentPhase != ELaunchType::Launch)
	{
		FinishAerial();
	}
}

void UAbilityTask_Aerial::OnAirTimeOut()
{
	if (CurrentPhase == ELaunchType::AirLoop)
	{
		UE_LOG(LogTemp, Log, TEXT("[AerialTask] Air time out, auto diving"));
		DivePhase();
	}
}

void UAbilityTask_Aerial::OnLaunchMontageEnded(UAnimMontage* InMontage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		OnLaunchComplete.Broadcast();
		AirLoopPhase();
	}
}

void UAbilityTask_Aerial::OnDiveMontageEnded(UAnimMontage* InMontage, bool bInterrupted)
{
}

void UAbilityTask_Aerial::OnLandingMontageEnded(UAnimMontage* InMontage, bool bInterrupted)
{
	OnAerialEnd.Broadcast();
	EndTask();
}

UAnimInstance* UAbilityTask_Aerial::GetAnimInstance() const
{
	if (CachedCharacter.IsValid())
	{
		return CachedCharacter->GetMesh()->GetAnimInstance();
	}
	return nullptr;
}

UCharacterMovementComponent* UAbilityTask_Aerial::GetMovementComp() const
{
	if (CachedCharacter.IsValid())
	{
		return CachedCharacter->GetCharacterMovement();
	}
	return nullptr;
}
