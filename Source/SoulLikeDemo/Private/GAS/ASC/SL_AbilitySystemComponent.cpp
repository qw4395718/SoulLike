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
