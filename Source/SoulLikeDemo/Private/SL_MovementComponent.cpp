#include "SL_MovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include <UObject/UObjectGlobals.h>
#include <UObject/ConstructorHelpers.h>
#include <GameFramework/Character.h>

USL_MovementComponent::USL_MovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USL_MovementComponent::InitMovemenetInfo()
{
	// 由外部信息进行初始化
	LoadMovementMentage("");
}

void USL_MovementComponent::ExeRoll()
{
	// 获取拥有者的信息
	AActor* OwnActor = GetOwner();
	if (OwnActor == nullptr) { return; }
	ACharacter* OwnCharacter = Cast<ACharacter>(OwnActor);
	if (OwnCharacter && CanExeRoll() == true)
	{
		UAnimInstance* AnimInstance = OwnCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(DodgeMontage);
		}
	}
}

bool USL_MovementComponent::CanExeRoll()
{
	return CanRoll;
}

void USL_MovementComponent::LoadMovementMentage(const FString MentagePath)
{
	if (MentagePath == "") { return; }
	static ConstructorHelpers::FObjectFinder<UAnimMontage> DodgeMontageFinder(*MentagePath);
	if (DodgeMontageFinder.Succeeded())
	{
		DodgeMontage = DodgeMontageFinder.Object;
	}
}
