#include "SL_GameplayAbilityComboBase.h"
#include "SL_CharacterBase.h"
#include "SL_AbilitySystemComponent.h"

USL_GameplayAbilityComboBase::USL_GameplayAbilityComboBase()
{
	// 默认支持网络
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USL_GameplayAbilityComboBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo == nullptr || TriggerEventData == nullptr)
	{// 安全性检查
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{// 检查消耗等行为是否受限
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CurrentHandle = Handle;
	CurrentActorInfo = ActorInfo;

	ApplyEffectToTarget(AcitvateEffectClass, TriggerEventData->Target, AcitvateEffectLevel);
	// 结束行为
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void USL_GameplayAbilityComboBase::EndAbilityForBP(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	EndAbility(Handle, &ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void USL_GameplayAbilityComboBase::PlayMontageForAbility(UAnimMontage* Montage, const FGameplayAbilityActivationInfo ActivationInfo, float PlayRate)
{
	if (!CurrentActorInfo || !Montage)
		return;

	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		// 绑定蒙太奇完成事件
		//FOnMontageCompleteDelegate CompleteDelegate;
		//CompleteDelegate.BindUObject(this, &USL_GameplayAbilityComboBase::OnMontageCompleted);
		ASC->PlayMontage(this, ActivationInfo, Montage, PlayRate, NAME_None, PlayRate);
	}
}

void USL_GameplayAbilityComboBase::ApplyEffectToTarget(TSubclassOf<UGameplayEffect> EffectClass, const AActor* const Target, float Level)
{
	if (!CurrentActorInfo || !Target)
		return;

	AActor* targetActor = const_cast<AActor*>(Target);
	RETURN_IF_TRUE(targetActor == nullptr);
	UAbilitySystemComponent* ASCSource = CurrentActorInfo->AbilitySystemComponent.Get();
	if (IAbilitySystemInterface* TargetASC_IF = Cast<IAbilitySystemInterface>(targetActor))
	{
		// 从接口获取目标的 AbilitySystemComponent
		UAbilitySystemComponent* TargetASC = TargetASC_IF->GetAbilitySystemComponent();
		if (!TargetASC)
			return;

		// 创建预测窗口，将后续操作与当前PredictionKey绑定
		FScopedPredictionWindow ScopedWindow(ASCSource, true);

		FGameplayEffectSpecHandle SpecHandle = ASCSource->MakeOutgoingSpec(EffectClass, Level, ASCSource->MakeEffectContext());
		if (SpecHandle.IsValid())
		{
			ASCSource->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC, FPredictionKey(ScopedWindow.ScopedPredictionKey));
		}
	}
}

void USL_GameplayAbilityComboBase::OnMontageCompleted()
{
	// 蒙太奇播放完成，结束技能
	EndAbility(CurrentHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USL_GameplayAbilityComboBase::OnMontageInterrupted()
{
	// 蒙太奇被打断，取消技能
	EndAbility(CurrentHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}