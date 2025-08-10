// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_BackStabbing_CA_NS.h"
#include "SoulLikeCharacter.h"

void USL_BackStabbing_CA_NS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		ASoulLikeCharacter* Character = Cast<ASoulLikeCharacter>(MeshComp->GetOwner());
		if (Character)
		{
			Character->SetBackStabbingState(true);
		}
	}
}

void USL_BackStabbing_CA_NS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		ASoulLikeCharacter* Character = Cast<ASoulLikeCharacter>(MeshComp->GetOwner());
		if (Character)
		{
			Character->SetBackStabbingState(false);
		}
	}
}
