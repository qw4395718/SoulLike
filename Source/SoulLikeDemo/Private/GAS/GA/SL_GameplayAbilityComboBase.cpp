#include "SL_GameplayAbilityComboBase.h"
#include "SL_CharacterBase.h"
#include "SL_AbilitySystemComponent.h"
#include <AT/AbilityTask_ComboMontage.h>
#include <Abilities/Tasks/AbilityTask_WaitGameplayEvent.h>
#include <SL_ComboManagerComponent.h>

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
	Super::ActivateAbility(Handle, ActorInfo,ActivationInfo, TriggerEventData);
	if (ActorInfo == nullptr)
	{// 安全性检查
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{// 检查消耗等行为是否受限
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 创建蒙太奇任务
	if (AttackMontage)
	{
		ComboMontageTask = UAbilityTask_ComboMontage::CreateComboMontageTask(
			this,
			AttackMontage,
			BlendOutTime,
			1.0f
		);

		ComboMontageTask->OnCompleted.AddDynamic(this, &USL_GameplayAbilityComboBase::OnMontageCompleted);
		ComboMontageTask->OnInterrupted.AddDynamic(this, &USL_GameplayAbilityComboBase::OnMontageInterrupted);
		RegisterActiveComboTask(ComboMontageTask);
		ComboMontageTask->ReadyForActivation();

		UE_LOG(LogTemp, Warning, TEXT("WindowTag_ActivateAbility"));
	}
	else
	{
		// 没有动画就直接结束
		UnRegisterActiveComboTask();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
}

void USL_GameplayAbilityComboBase::EndAbilityForBP(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UnRegisterActiveComboTask();
	EndAbility(Handle, &ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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
			ASCSource->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC, ASCSource->GetPredictionKeyForNewAction());
		}
	}
}

void USL_GameplayAbilityComboBase::OnMontageCompleted()
{
	RETURN_IF_TRUE(CurrentActorInfo == nullptr);
	// 蒙太奇播放完成，结束技能
	if (USL_ComboManagerComponent* ComboMgr = Cast<USL_ComboManagerComponent>(GetComboManager()))
	{
		// 通知 ComboManager：当前蒙太奇自然完成，不要等待 BlendOut 了
		ComboMgr->OnMontageFinished();
	}


	UnRegisterActiveComboTask();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USL_GameplayAbilityComboBase::OnMontageInterrupted()
{
	RETURN_IF_TRUE(CurrentActorInfo == nullptr);
	// 蒙太奇播放中断，结束技能
	UnRegisterActiveComboTask();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

UActorComponent* USL_GameplayAbilityComboBase::GetComboManager() const
{
	// 使用缓存
	if (CachedComboManager.IsValid())
	{
		return CachedComboManager.Get();
	}

	// 从当前Avatar拿到信息
	AActor* actor = GetAvatarActorFromActorInfo();
	if (actor)
	{
		CachedComboManager = actor->FindComponentByClass<USL_ComboManagerComponent>();
	}

	return CachedComboManager.Get();
}

void USL_GameplayAbilityComboBase::RegisterActiveComboTask(UAbilityTask_ComboMontage* InTask)
{
	// 注册到ComboManager中
	if (USL_ComboManagerComponent* ComboMgr =Cast<USL_ComboManagerComponent>(GetComboManager()))
	{
		ComboMgr->RegisterActiveComboTask(InTask);
		ComboMgr->ClearTargetWindowTag();
	}
	UE_LOG(LogTemp, Warning, TEXT("WindowTag_RegisterActiveComboTask"));

}

void USL_GameplayAbilityComboBase::UnRegisterActiveComboTask()
{
	if (USL_ComboManagerComponent* ComboMgr = Cast<USL_ComboManagerComponent>(GetComboManager()))
	{
		ComboMgr->UnregisterActiveComboTask();
	}
}

