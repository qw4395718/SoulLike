#include "SL_GameplayAbilityNPCBase.h"
#include "SL_CharacterBase.h"
#include "SL_AbilitySystemComponent.h"
#include <Abilities/Tasks/AbilityTask_PlayMontageAndWait.h>

USL_GameplayAbilityNPCBase::USL_GameplayAbilityNPCBase()
{
	// 默认支持网络
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USL_GameplayAbilityNPCBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo,ActivationInfo, TriggerEventData);

	CurrentHandle = Handle;
	CurrentActorInfo = ActorInfo;
	MontagePlayState = EMontagePlayState::None;

    PlayMontageForAbility(AttackMontage, ActivationInfo,1.0f);

}

void USL_GameplayAbilityNPCBase::EndAbilityForBP(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	EndAbility(Handle, &ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void USL_GameplayAbilityNPCBase::PlayMontageForAbility(UAnimMontage* Montage, const FGameplayAbilityActivationInfo ActivationInfo, float PlayRate)
{
 if (!CurrentActorInfo || !Montage)
        return;

    UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
    if (!ASC) return;

    // ===== 修改：使用 AbilityTask_PlayMontageAndWait =====
    // UE4.26: 这是推荐的方式，能正确处理蒙太奇的完成/中断
    UAbilityTask_PlayMontageAndWait* MontageTask = 
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,           // TaskInstanceName
            Montage,
            PlayRate,
            NAME_None,           // SectionToPlay
            false                // bStopWhenAbilityEnds
        );

    if (MontageTask)
    {
        MontagePlayState = EMontagePlayState::Playing;

        // 绑定蒙太奇完成事件
        MontageTask->OnCompleted.AddDynamic(this, &USL_GameplayAbilityNPCBase::OnMontageCompleted);
        MontageTask->OnInterrupted.AddDynamic(this, &USL_GameplayAbilityNPCBase::OnMontageInterrupted);

        // 激活任务
        MontageTask->ReadyForActivation();
    }
}

void USL_GameplayAbilityNPCBase::ApplyEffectToTarget(TSubclassOf<UGameplayEffect> InEffectClass, AActor* Target, float Level)
{
	if (!CurrentActorInfo || !Target || !EffectClass)
		return;

	UAbilitySystemComponent* ASCSource = CurrentActorInfo->AbilitySystemComponent.Get();
	if (IAbilitySystemInterface* TargetASC_IF = Cast<IAbilitySystemInterface>(Target))
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

void USL_GameplayAbilityNPCBase::OnMontageCompleted()
{
	// 蒙太奇播放完成，结束技能
    MontagePlayState = EMontagePlayState::Completed;
    
    UE_LOG(LogTemp, Verbose, TEXT("Ability %s: Montage completed"), *GetName());

    // 广播完成事件（BTTask会监听这个）
    BroadcastMontageCompleted();

    // 结束能力
    EndAbility(CurrentHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USL_GameplayAbilityNPCBase::OnMontageInterrupted()
{
    MontagePlayState = EMontagePlayState::Interrupted;
    
    UE_LOG(LogTemp, Warning, TEXT("Ability %s: Montage interrupted"), *GetName());

    // 广播完成事件（让BTTask知道任务结束）
    BroadcastMontageCompleted();

    // 取消能力
    EndAbility(CurrentHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void USL_GameplayAbilityNPCBase::BroadcastMontageCompleted()
{
    if (OnMontageCompletedDelegate.IsBound())
    {
        OnMontageCompletedDelegate.Broadcast();
    }
}