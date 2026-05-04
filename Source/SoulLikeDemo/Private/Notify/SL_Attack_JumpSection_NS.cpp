// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_Attack_JumpSection_NS.h"

void USL_Attack_JumpSection_NS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		// 假设角色持有武器，并通过接口控制碰撞

	}
}

void USL_Attack_JumpSection_NS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{

}
