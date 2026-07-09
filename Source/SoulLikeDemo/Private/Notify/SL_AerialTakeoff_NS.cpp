// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_AerialTakeoff_NS.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemGlobals.h"
#include <SL_AbilitySystemComponent.h>
#include "AbilityTask_Aerial.h"


void USL_AerialTakeoff_NS::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	// 获取ASC
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(MeshComp->GetOwner());
	if (!ASCInterface) return;

	USL_AbilitySystemComponent* ASC = Cast<USL_AbilitySystemComponent>(ASCInterface->GetAbilitySystemComponent());
	if (!ASC) return;

	TArray<UGameplayTask*> ActiveTasks = ASC->GetCurrentlyActiveTasks();
	for (UGameplayTask* Task : ActiveTasks)
	{
		UAbilityTask_Aerial* AerialTask = Cast<UAbilityTask_Aerial>(Task);
		if (AerialTask && AerialTask->IsActive())
		{
			AerialTask->RequestDive();
			return;
		}
	}
}

