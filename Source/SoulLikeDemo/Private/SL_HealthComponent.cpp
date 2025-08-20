#include "SL_HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_HealthComponent::USL_HealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
