#include "SL_GameplayAbilityBase.h"
#include "SL_CharacterBase.h"
#include "SL_AbilitySystemComponent.h"

USL_GameplayAbilityBase::USL_GameplayAbilityBase()
{
	// 默认支持网络
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USL_GameplayAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{// 检查消耗等行为是否受限
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CurrentHandle = Handle;
	CurrentActorInfo = ActorInfo;

	OnAbilityActivatedForLua(Handle, *ActorInfo, ActivationInfo);
}

void USL_GameplayAbilityBase::EndAbilityForBP(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	EndAbility(Handle, &ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void USL_GameplayAbilityBase::PlayMontageForAbility(UAnimMontage* Montage, const FGameplayAbilityActivationInfo ActivationInfo, float PlayRate)
{
	if (!CurrentActorInfo || !Montage)
		return;

	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		// 绑定蒙太奇完成事件
		//FOnMontageCompleteDelegate CompleteDelegate;
		//CompleteDelegate.BindUObject(this, &USL_GameplayAbilityBase::OnMontageCompleted);
		ASC->PlayMontage(this, ActivationInfo, Montage, PlayRate, NAME_None, PlayRate);
	}
}

void USL_GameplayAbilityBase::ApplyEffectToTarget(TSubclassOf<UGameplayEffect> EffectClass, AActor* Target, float Level)
{
	if (!CurrentActorInfo || !Target)
		return;

	UAbilitySystemComponent* ASCSource = CurrentActorInfo->AbilitySystemComponent.Get();
	if (IAbilitySystemInterface* TargetASC_IF = Cast<IAbilitySystemInterface>(Target))
	{
		// 从接口获取目标的 AbilitySystemComponent
		UAbilitySystemComponent* TargetASC = TargetASC_IF->GetAbilitySystemComponent();
		if (!TargetASC)
			return;

		FGameplayEffectSpecHandle SpecHandle = ASCSource->MakeOutgoingSpec(EffectClass, Level, ASCSource->MakeEffectContext());
		if (SpecHandle.IsValid())
		{
			ASCSource->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
		}
	}
}

void USL_GameplayAbilityBase::OnMontageCompleted()
{
	// 蒙太奇播放完成，结束技能
	EndAbility(CurrentHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USL_GameplayAbilityBase::OnMontageInterrupted()
{
	// 蒙太奇被打断，取消技能
	EndAbility(CurrentHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}