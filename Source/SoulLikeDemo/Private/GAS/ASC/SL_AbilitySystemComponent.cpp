#include <SL_AbilitySystemComponent.h>
#include <GameplayAbilitySpec.h>

USL_AbilitySystemComponent::USL_AbilitySystemComponent()
	: Super()
{

}

FGameplayAbilitySpecHandle USL_AbilitySystemComponent::GiveAbilityForBP(TSubclassOf<UGameplayAbility> AbilityClass, int32 InLevel, int32 InInputID, UObject* SourceObject)
{
	return GiveAbility(FGameplayAbilitySpec(AbilityClass, InLevel, InInputID, SourceObject));
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
