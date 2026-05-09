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

bool USL_AbilitySystemComponent::TryActivateAbilityByTag(const FGameplayTag& AbilityTag)
{
    if (!AbilityTag.IsValid()) return false;

    // UE4.26: 通过Tag查找可激活的能力
    FGameplayAbilitySpec* FoundSpec = FindAbilitySpecFromTag(AbilityTag);
    if (FoundSpec)
    {
        return TryActivateAbility(FoundSpec->Handle);
    }

    UE_LOG(LogTemp, Warning, TEXT("USL_AbilitySystemComponent::TryActivateAbilityByTag - No ability found with tag: %s"), 
        *AbilityTag.ToString());
    return false;
}

FGameplayAbilitySpecHandle USL_AbilitySystemComponent::FindAbilitySpecHandleByTag(const FGameplayTag& AbilityTag) const
{
    const FGameplayAbilitySpec* FoundSpec = FindAbilitySpecFromTag(AbilityTag);
    if (FoundSpec)
    {
        return FoundSpec->Handle;
    }
    return FGameplayAbilitySpecHandle();
}

UGameplayAbility* USL_AbilitySystemComponent::GetActiveAbilityInstanceByTag(const FGameplayTag& AbilityTag) const
{
    const FGameplayAbilitySpec* FoundSpec = FindAbilitySpecFromTag(AbilityTag);
    if (FoundSpec && FoundSpec->IsActive())
    {
        // UE4.26: 获取激活的能力实例
        TArray<UGameplayAbility*> Instances = FoundSpec->GetAbilityInstances();
        for (UGameplayAbility* Instance : Instances)
        {
            if (Instance && Instance->IsActive())
            {
                return Cast<UGameplayAbility>(Instance);
            }
        }
    }
    return nullptr;
}

FGameplayAbilitySpec* USL_AbilitySystemComponent::FindAbilitySpecFromTag(const FGameplayTag& AbilityTag) const
{
	// 检查Tag是否有效
	if (!AbilityTag.IsValid())
	{
		return nullptr;
	}

	// UE4.26: GetActivatableAbilities() 返回 const TArray<FGameplayAbilitySpec>&
	// 但因为我们在const函数中，需要使用const_cast来获取可修改的指针
	// 这是安全的，因为我们只是读取数据并返回指针，不修改数据
	const TArray<FGameplayAbilitySpec>& ActivatableAbilities_temp = GetActivatableAbilities();

	for (int32 Index = 0; Index < ActivatableAbilities_temp.Num(); Index++)
	{
		const FGameplayAbilitySpec& Spec = ActivatableAbilities_temp[Index];

		// 检查能力本身的Tag
		if (Spec.Ability && Spec.Ability->AbilityTags.HasTag(AbilityTag))
		{
			// 通过索引获取可修改的引用
			return &const_cast<TArray<FGameplayAbilitySpec>&>(ActivatableAbilities_temp)[Index];
		}

		// 检查动态赋予的Tag
		if (Spec.DynamicAbilityTags.HasTag(AbilityTag))
		{
			return &const_cast<TArray<FGameplayAbilitySpec>&>(ActivatableAbilities_temp)[Index];
		}
	}

	// 没有找到匹配的能力
	UE_LOG(LogTemp, Verbose, TEXT("USL_AbilitySystemComponent::FindAbilitySpecFromTag - No ability found with tag: %s"),
		*AbilityTag.ToString());

	return nullptr;
}

void USL_AbilitySystemComponent::SetAliveTag()
{
	RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Dead"));
	AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Alive")); 
}
