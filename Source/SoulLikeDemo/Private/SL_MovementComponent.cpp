#include "SL_MovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_MovementComponent::USL_MovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
