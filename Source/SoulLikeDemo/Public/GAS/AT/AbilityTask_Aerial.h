#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include <SoulLikeGameGlobal.h>
#include "AbilityTask_Aerial.generated.h"

class ACharacter;
class UAnimMontage;
class UAbilitySystemComponent;
class UCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAerialPhaseCompleted);

UCLASS()
class SOULLIKEDEMO_API UAbilityTask_Aerial : public UAbilityTask
{
	GENERATED_BODY()

public:
	static UAbilityTask_Aerial* CreateAerialTask(
		UGameplayAbility* OwningAbility,
		UAnimMontage* InLaunchMontage,
		UAnimMontage* InDiveMontage,
		UAnimMontage* InLandingMontage,
		float InMaxAirTime,
		float InLaunchVelocityZ = 300.0f,
		float InPlayRate = 1.0f);

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

	void RequestDive();
	void OnLanded();

protected:
	void LaunchPhase();
	void AirLoopPhase();
	void DivePhase();
	void FinishAerial();

	void OnAirTimeOut();

	UFUNCTION()
	void OnLaunchMontageEnded(UAnimMontage* InMontage, bool bInterrupted);

	UFUNCTION()
	void OnDiveMontageEnded(UAnimMontage* InMontage, bool bInterrupted);

	UFUNCTION()
	void OnLandingMontageEnded(UAnimMontage* InMontage, bool bInterrupted);

	UAnimInstance* GetAnimInstance() const;
	UCharacterMovementComponent* GetMovementComp() const;

public:
	UPROPERTY(BlueprintAssignable)
	FOnAerialPhaseCompleted OnLaunchComplete;

	UPROPERTY(BlueprintAssignable)
	FOnAerialPhaseCompleted OnAirLoopStart;

	UPROPERTY(BlueprintAssignable)
	FOnAerialPhaseCompleted OnDiveStart;

	UPROPERTY(BlueprintAssignable)
	FOnAerialPhaseCompleted OnAerialEnd;

protected:
	UPROPERTY()
	UAnimMontage* LaunchMontage;

	UPROPERTY()
	UAnimMontage* DiveMontage;

	UPROPERTY()
	UAnimMontage* LandingMontage;

	float MaxAirTime;
	float LaunchVelocityZ;
	float PlayRate;
	ELaunchType CurrentPhase;
	FTimerHandle AirTimerHandle;

	TWeakObjectPtr<ACharacter> CachedCharacter;
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
};
