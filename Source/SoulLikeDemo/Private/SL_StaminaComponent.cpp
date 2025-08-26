#include "SL_StaminaComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_StaminaComponent::USL_StaminaComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

bool USL_StaminaComponent::GetIsStaminaZero()
{
	return false;
}

float USL_StaminaComponent::GetCurrentStaminaValue()
{
	return 0.0f;
}

float USL_StaminaComponent::GetMaxStaminaValue()
{
	return 0.0f;
}

void USL_StaminaComponent::ReduceStaminaValue(float ReduceValue)
{

}

void USL_StaminaComponent::OnStaminaValueZero()
{

}

void USL_StaminaComponent::ReviveStaminaValue(float ReviveValue)
{

}
