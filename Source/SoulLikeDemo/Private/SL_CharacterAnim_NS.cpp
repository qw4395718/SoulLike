// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CharacterAnim_NS.h"
#include "GameFramework/Character.h"
#include "AnimNotify_IF.h"

void USL_CharacterAnim_NS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner()))
		{
			if (IAnimNotify_IF* AnimNotifyTarget = Cast<IAnimNotify_IF>(Character))
			{
				AnimNotifyTarget->AnimNotifyResponse(int(AnimStartNotify));
			}
		}
	}
}

void USL_CharacterAnim_NS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner()))
		{
			if (IAnimNotify_IF* AnimNotifyTarget = Cast<IAnimNotify_IF>(Character))
			{
				AnimNotifyTarget->AnimNotifyResponse(int(AnimEndNotify));
			}
		}
	}
}
