#include "SL_WeaponComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_WeaponComponent::USL_WeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
