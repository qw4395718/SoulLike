#include "AbilityTask_ComboMontage.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemGlobals.h"
#include <Animation/AnimInstance.h>

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

void UAbilityTask_ComboMontage::OnAllowBlendReached()
{
	if (bHasFinished) return;

	bReadyToBlend = true;

	// 如果有暂存的输入，立即执行打断
	if (bHasPendingInput && PendingInputAction != EComboInputActionType::EComboInputAction_None)
	{
		UE_LOG(LogTemp, Log, TEXT("[ComboTask] AllowBlend reached, consuming cached input"));
		ExecuteBlendOut();
	}
}

bool UAbilityTask_ComboMontage::OnInputReceived(EComboInputActionType InputAction)
{
	RETURN_VALUE_IF_TRUE(bHasFinished,false);
	if (InputAction >= EComboInputActionType::EComboInputAction_Max ||
		InputAction <= EComboInputActionType::EComboInputAction_None)
	{
		return false;
	}

	if (bReadyToBlend)
	{
		// 情形A：已经可以混合，直接打断
		PendingInputAction = InputAction;
		UE_LOG(LogTemp, Log, TEXT("[ComboTask] Immediate blend out on input"));
		ExecuteBlendOut();
		return true;
	}
	else
	{
		// 情形B：还未到AllowBlend，缓存输入
		bHasPendingInput = true;
		PendingInputAction = InputAction;
		UE_LOG(LogTemp, Log, TEXT("[ComboTask] Input cached, waiting for AllowBlend"));
		return true;
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

	// 触发BlendOut委托，通知GA可以结束了
	OnBlendOut.Broadcast();

	// 如果GA有EndAbility调用，这里不做EndTask，
	// 让GA在收到OnBlendOut后自行EndAbility
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

void UAbilityTask_ComboMontage::OnDestroy(bool bInOwnerFinished)
{
	// 清理蒙太奇绑定
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