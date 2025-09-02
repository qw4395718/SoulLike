#include "SL_CharacterStateComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_CharacterStateComponent::USL_CharacterStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Ä¬ÈÏ³õÊ¼»¯
	bIsReadyForExecution = false;
	bCanBackStab = false;
}

bool USL_CharacterStateComponent::CanExecute()
{
	return bIsReadyForExecution;
}

bool USL_CharacterStateComponent::CanBackStabs()
{
	return bCanBackStab;
}

void USL_CharacterStateComponent::InitCharacterStateInfo()
{

}

void USL_CharacterStateComponent::SetIsReadyForExecution(bool bReady)
{
	bIsReadyForExecution = bReady;
}
