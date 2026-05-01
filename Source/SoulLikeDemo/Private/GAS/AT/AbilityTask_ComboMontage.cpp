#include "AbilityTask_ComboMontage.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemGlobals.h"
#include <Animation/AnimInstance.h>
#include <AbilitySystemInterface.h>

UAbilityTask_ComboMontage* UAbilityTask_ComboMontage::CreateComboMontageTask(
	UGameplayAbility* OwningAbility,
	UAnimMontage* MontageToPlay,
	float InBlendOutTime,
	float InPlayRate)
{
	UAbilityTask_ComboMontage* Task = NewAbilityTask<UAbilityTask_ComboMontage>(OwningAbility);
	Task->Montage = MontageToPlay;
	Task->BlendOutTime = InBlendOutTime;
	Task->PlayRate = InPlayRate;
	return Task;
}

void UAbilityTask_ComboMontage::OnAllowBlendReached(FGameplayTag CurrentWindowTag)
{
	if (bHasFinished) return;

	bReadyToBlend = true;
	ComboWindowTag = CurrentWindowTag;

	// 如果有暂存的输入，立即执行打断
	if (bBlendOutRequested) {
		UE_LOG(LogTemp, Log, TEXT("[ComboTask] AllowBlend reached, consuming cached input"));
		ExecuteBlendOut();
	}
}

EComboInputHandledResult UAbilityTask_ComboMontage::OnInputReceived(EComboInputActionType InputAction)
{
	if (bHasFinished || 
		InputAction <= EComboInputActionType::EComboInputAction_None ||
		InputAction >= EComboInputActionType::EComboInputAction_Max) 
		return EComboInputHandledResult::Rejected;

	if (bReadyToBlend)
	{
		// 情形A：已经可以混合，直接打断
		UE_LOG(LogTemp, Log, TEXT("[ComboTask] Immediate blend out on input"));
		ExecuteBlendOut();
		return EComboInputHandledResult::AcceptedAndBlended;
	}
	else
	{
		// 情形B：还未到AllowBlend，缓存输入
		bHasPendingInput = true;
		bBlendOutRequested = true;
		UE_LOG(LogTemp, Log, TEXT("[ComboTask] Input cached, waiting for AllowBlend"));
		return EComboInputHandledResult::Accepted;
	}
}

void UAbilityTask_ComboMontage::Activate()
{
	Super::Activate();

	if (!Ability || !Montage)
	{
		EndTask();
		return;
	}

	// 获取Avatar Actor
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character)
	{
		EndTask();
		return;
	}

	// 播放蒙太奇
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		EndTask();
		return;
	}

	AnimInstance->Montage_Play(Montage, PlayRate);

	// 绑定蒙太奇结束委托
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UAbilityTask_ComboMontage::OnMontageCompleted);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);

	// 绑定蒙太奇混合出委托
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	BlendingOutDelegate.BindUObject(this, &UAbilityTask_ComboMontage::OnMontageBlendOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Montage);

	// 如果Ability提前结束
	if (Ability)
	{
		Ability->OnGameplayAbilityEnded.AddUObject(this, &UAbilityTask_ComboMontage::OnMontageInterrupted);
	}
}

void UAbilityTask_ComboMontage::OnDestroy(bool bInOwnerFinished)
{
	if (Ability)
	{
		Ability->OnGameplayAbilityEnded.RemoveAll(this);
	}
	// 清理蒙太奇绑定
	RemoveAllDelegates();
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (Character && Montage)
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Stop(0.f, Montage);
		}
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_ComboMontage::OnMontageCompleted(UAnimMontage* InMontage, bool bInterrupted)
{
	if (bHasFinished) return;
	bHasFinished = true;
	OnCompleted.Broadcast();
	EndTask();
}

void UAbilityTask_ComboMontage::OnMontageBlendOut(UAnimMontage* InMontage, bool bInterrupted)
{
	// 正常混合出（动画播完），直接结束
	if (!bHasFinished)
	{
		bHasFinished = true;
		OnCompleted.Broadcast();
		EndTask();
	}
}

void UAbilityTask_ComboMontage::OnMontageInterrupted(UGameplayAbility* ActiveAbility)
{
	if (!bHasFinished)
	{
		bHasFinished = true;
		EndTask();
	}
}



void UAbilityTask_ComboMontage::ExecuteBlendOut()
{
	if (bHasFinished) return;
	bHasFinished = true;

	// 停止蒙太奇（带混合时间）
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (Character && Montage)
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Stop(BlendOutTime, Montage);
		}
	}	

	// 通知GA：动画被打断了
	OnInterrupted.Broadcast();
}

void UAbilityTask_ComboMontage::RemoveAllDelegates()
{
	OnCompleted.Clear();
	OnInterrupted.Clear();
}

