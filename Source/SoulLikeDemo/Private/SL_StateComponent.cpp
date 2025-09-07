#include "SL_StateComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_StateComponent::USL_StateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Ä¬ÈÏ³õÊ¼»¯
	DamageIncreasePercentage = 0.0f;
	DamageReducePercentage = 0.0f;
}

float USL_StateComponent::DamageDealtCalculate(float Damage)
{
	float CalculateValue = 0.0f;
	CalculateValue = Damage*(1+ DamageIncreasePercentage);
	return CalculateValue;
}

float USL_StateComponent::DamageReceivedCalculate(float Damage)
{
	float CalculateValue = 0.0f;
	CalculateValue = Damage*(DamageReducePercentage > REDUCE_DAMAGE_PERCENTAGE:1 - fDamageReducePercentage?1 - REDUCE_DAMAGE_PERCENTAGE);
	return CalculateValue;
}

float USL_StateComponent::StaminaCostCalculate(float StaminaCost)
{
	float CalculateValue = 0.0f;
	CalculateValue = StaminaCost * (StaminaCostReducePercentage > REDUCE_STAMINACOST_PERCENTAGE:StaminaCostReducePercentage ? REDUCE_STAMINACOST_PERCENTAGE);
	return CalculateValue;
}
