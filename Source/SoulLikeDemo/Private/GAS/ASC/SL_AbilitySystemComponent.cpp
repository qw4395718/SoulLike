#include <SL_AbilitySystemComponent.h>
#include <GameplayAbilitySpec.h>

USL_AbilitySystemComponent::USL_AbilitySystemComponent()
	: Super()
{
	SetIsReplicatedByDefault(true);
}

FGameplayAbilitySpecHandle USL_AbilitySystemComponent::GiveAbilityForBP(TSubclassOf<UGameplayAbility> AbilityClass, int32 InLevel, int32 InInputID, UObject* SourceObject)
{
	if (GetOwnerActor()->HasAuthority())
	{
		return GiveAbility(FGameplayAbilitySpec(AbilityClass, InLevel, InInputID, SourceObject));
	}
	return FGameplayAbilitySpecHandle();
}

FGameplayAbilitySpecHandle USL_AbilitySystemComponent::GiveAbilityAndActivateOnceForBP(TSubclassOf<UGameplayAbility> AbilityClass, int32 InLevel, int32 InInputID, UObject* SourceObject)
{
	FGameplayAbilitySpec Spec(AbilityClass, InLevel, InInputID, SourceObject);
	return GiveAbilityAndActivateOnce(Spec);
}

TArray<FGameplayAbilitySpec>& USL_AbilitySystemComponent::GetActivatableAbilitiesForBP()
{
	return ActivatableAbilities.Items;
}

bool USL_AbilitySystemComponent::TryActivateAbilityByHandle(FGameplayAbilitySpecHandle Handle)
{
	// 1. 检查Handle是否有效
	if (!Handle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TryActivateAbilityByHandle: Invalid Handle"));
		return false;
	}

	return Super::TryActivateAbility(Handle);

}

void USL_AbilitySystemComponent::ClearAbilityByHandle(FGameplayAbilitySpecHandle Handle)
{
	Super::ClearAbility(Handle);
}

TArray<UGameplayTask*> USL_AbilitySystemComponent::GetCurrentlyActiveTasks()
{
	TArray<UGameplayTask*> ActiveTasks;

	// 遍历所有激活的技能（Ability）
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.IsActive())
		{
			for (UGameplayAbility* AbilityInstance : Spec.GetAbilityInstances())
			{
				if (AbilityInstance)
				{
					// 获取该技能当前所有活跃的任务
					TArray<UGameplayTask*> AbilityTasks = GetAbilityActiveTasks(AbilityInstance);
					ActiveTasks.Append(AbilityTasks);
				}
			}
		}
	}

	return ActiveTasks;
}
