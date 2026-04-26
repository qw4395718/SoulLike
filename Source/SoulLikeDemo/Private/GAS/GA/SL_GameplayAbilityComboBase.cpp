#include "SL_GameplayAbilityComboBase.h"
#include "SL_CharacterBase.h"
#include "SL_AbilitySystemComponent.h"
#include <AT/AbilityTask_ComboMontage.h>
#include <Abilities/Tasks/AbilityTask_WaitGameplayEvent.h>

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
		ComboMontageTask->ReadyForActivation();
	}
	else
	{
		// 没有动画就直接结束
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// ====== 2. 监听ComboManager发来的输入事件 ======
	UAbilityTask_WaitGameplayEvent* WaitComboInput = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FGameplayTag::RequestGameplayTag(TEXT("Event.Combo.InputReceived")),
		nullptr,
		false,
		true // OnlyTriggerOnce = true，只需要一次
	);
	WaitComboInput->EventReceived.AddDynamic(this, &USL_GameplayAbilityComboBase::OnComboInputReceived);
	WaitComboInput->ReadyForActivation();

	// ====== 3. 监听AnimNotify发来的AllowBlend事件 ======
	UAbilityTask_WaitGameplayEvent* WaitAllowBlend = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FGameplayTag::RequestGameplayTag(TEXT("Event.Combo.AllowBlend")),
		nullptr,
		false,
		true
	);
	WaitAllowBlend->EventReceived.AddDynamic(this, &USL_GameplayAbilityComboBase::OnAllowBlendReceived);
	WaitAllowBlend->ReadyForActivation();

}

void USL_GameplayAbilityComboBase::EndAbilityForBP(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
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
	// 激活缓存的下一招
	if (bHasPendingCombo && PendingNextGA)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->TryActivateAbilityByClass(PendingNextGA);
		}
	}

	EndAbility(CurrentHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void USL_GameplayAbilityComboBase::OnComboInputReceived(FGameplayEventData EventData)
{
	// 从事件中提取下一招GA的Class
	UClass* NextGAClass = const_cast<UClass*>(Cast<UClass>(EventData.OptionalObject));
	if (!NextGAClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_Combo] Received combo input but no NextGA class provided"));
		return;
	}

	PendingNextGA = NextGAClass;
	bComboInputReceived = true;

	UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Combo input received. NextGA=%s, bAllowBlend=%d"),
		*PendingNextGA->GetName(),
		bAllowBlendReached);

	// 检查是否可以立即执行
	if (bRespectBlendWindow)
	{
		if (bAllowBlendReached && ComboMontageTask && ComboMontageTask->IsReadyToBlend())
		{
			// 已到达AllowBlend，立即打断并缓存下一招
			bHasPendingCombo = true;
			ComboMontageTask->RequestBlendOut();
		}
		else
		{
			// 还没到AllowBlend，仅标记缓存
			bHasPendingCombo = true;
			UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Combo cached, waiting for AllowBlend"));
		}
	}
	else
	{
		// 不尊重窗口，直接打断
		bHasPendingCombo = true;
		ComboMontageTask->RequestBlendOut();
	}
}

void USL_GameplayAbilityComboBase::OnAllowBlendReceived(FGameplayEventData EventData)
{
	bAllowBlendReached = true;

	UE_LOG(LogTemp, Log, TEXT("[GA_Combo] AllowBlend received. bHasPendingCombo=%d, PendingGA=%s"),
		bHasPendingCombo,
		PendingNextGA ? *PendingNextGA->GetName() : TEXT("None"));

	// 通知Task
	if (ComboMontageTask)
	{
		ComboMontageTask->OnAllowBlendReached();
	}

	// 如果有缓存的连击输入，且允许混合，立即执行
	if (bHasPendingCombo && bAllowBlendReached)
	{
		ExecutePendingCombo();
	}
}

void USL_GameplayAbilityComboBase::ExecutePendingCombo()
{
	if (!PendingNextGA)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_Combo] ExecutePendingCombo called but no pending GA"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Executing pending combo → %s"), *PendingNextGA->GetName());

	// 请求Task打断动画
	if (ComboMontageTask && !ComboMontageTask->IsReadyToBlend())
	{
		// 如果已经可以混合就立即打断，否则标记等待
		ComboMontageTask->RequestBlendOut();
	}
	// 注意：下一招的激活在 OnMontageInterrupted 中处理
}
