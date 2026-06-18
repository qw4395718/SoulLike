// Component/HitFeedback/SL_HitFeedbackManagerComponent.cpp

#include "SL_HitFeedbackManagerComponent.h"
#include "Effects/SL_CameraShake_Hit.h"
#include "Manager/GlobalDelegatesManager.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include <GameFramework/Character.h>

USL_HitFeedbackManagerComponent::USL_HitFeedbackManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USL_HitFeedbackManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 订阅 GlobalDelegatesManager 的命中反馈事件
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		OnHitFeedbackHandle = DelegateMgr->OnHitFeedback.AddUFunction(this, TEXT("OnHitFeedback"));
	}
}

void USL_HitFeedbackManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 恢复所有正在 HitStop 的 Actor（防止组件销毁后 Actor 卡死）
	for (auto& Pair : HitStopTimerHandles)
	{
		if (Pair.Key)
		{
			Pair.Key->CustomTimeDilation = 1.0f;
			GetWorld()->GetTimerManager().ClearTimer(Pair.Value);
		}
	}
	HitStopTimerHandles.Empty();

	// 取消委托绑定
	if (OnHitFeedbackHandle.IsValid())
	{
		if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
		{
			DelegateMgr->OnHitFeedback.Remove(OnHitFeedbackHandle);
			OnHitFeedbackHandle.Reset();
		}
	}

	Super::EndPlay(EndPlayReason);
}

void USL_HitFeedbackManagerComponent::OnHitFeedback(const FHitFeedbackData& InData)
{
	// === 1. Hit Stop（所有客户端都要执行，暂停攻击者和受击者） ===
	ApplyHitStop(InData);

	// === 2. Camera Shake + 屏幕效果（仅本机角色视角） ===
	ACharacter* LocalChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!LocalChar) return;

	AActor* LocalActor = Cast<AActor>(LocalChar);
	bool bIsLocalInstigator = (LocalActor == InData.InstigatorActor);
	bool bIsLocalTarget    = (LocalActor == InData.TargetActor);
	if (!bIsLocalInstigator && !bIsLocalTarget) return;

	float Scale = GetShakeScale(InData.Severity);
	if (bIsLocalInstigator)
	{
		PlayHitCameraShake(InData.Severity, Scale * 0.5f);
		//PlayHitScreenEffect(InData.Severity);
	}
	else if (bIsLocalTarget)
	{
		PlayHitCameraShake(InData.Severity, Scale);
		//PlayHitScreenEffect(InData.Severity);
	}
}

/************************************************************************/
/*                              Hit Stop                                */
/************************************************************************/

void USL_HitFeedbackManagerComponent::ApplyHitStop(const FHitFeedbackData& InData)
{
	float Duration = 0.1f;
	float Dilation = 0.5f;
	switch (InData.Severity)
	{
	case EHitSeverity::Light:   Duration = HitStop_Light;   Dilation = HitStopDilation_Light;   break;
	case EHitSeverity::Heavy:   Duration = HitStop_Heavy;   Dilation = HitStopDilation_Heavy;   break;
	case EHitSeverity::Execute: Duration = HitStop_Execute; Dilation = HitStopDilation_Execute; break;
	case EHitSeverity::Parry:   Duration = HitStop_Parry;   Dilation = HitStopDilation_Parry;   break;
	case EHitSeverity::Block:   Duration = HitStop_Block;   Dilation = HitStopDilation_Block;   break;
	}

	auto PauseAndSchedule = [this, Duration, Dilation](AActor* InActor)
	{
		if (!InActor) return;
		InActor->CustomTimeDilation = Dilation;
		FTimerHandle& Handle = HitStopTimerHandles.FindOrAdd(InActor);
		GetWorld()->GetTimerManager().ClearTimer(Handle);
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateUFunction(this,
			TEXT("RestoreActorTimeDilation"), InActor), Duration, false);
	};

	PauseAndSchedule(InData.InstigatorActor);
	PauseAndSchedule(InData.TargetActor);
}

void USL_HitFeedbackManagerComponent::RestoreActorTimeDilation(AActor* InActor)
{
	if (InActor)
	{
		InActor->CustomTimeDilation = 1.0f;
		HitStopTimerHandles.Remove(InActor);
	}
}

/************************************************************************/
/*                            Camera Shake                              */
/************************************************************************/

void USL_HitFeedbackManagerComponent::PlayHitCameraShake(EHitSeverity InSeverity, float InShakeScale) const
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC) return;

	APlayerCameraManager* CamMgr = PC->PlayerCameraManager;
	if (!CamMgr) return;

	TSubclassOf<UCameraShakeBase> ShakeClass = USL_CameraShake_Hit::StaticClass();
	if (!ShakeClass) return;

	CamMgr->StartCameraShake(ShakeClass, InShakeScale);
}

/************************************************************************/
/*                            屏幕特效                                  */
/************************************************************************/

void USL_HitFeedbackManagerComponent::PlayHitScreenEffect(EHitSeverity InSeverity) const
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC) return;

	APlayerCameraManager* CamMgr = PC->PlayerCameraManager;
	if (!CamMgr) return;

	float FadeInDuration = 0.02f;
	float HoldDuration;
	float FadeOutDuration;
	FLinearColor FlashColor;

	switch (InSeverity)
	{
	case EHitSeverity::Light:
		FlashColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.15f);
		HoldDuration = 0.04f;
		FadeOutDuration = 0.06f;
		break;
	case EHitSeverity::Heavy:
		FlashColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.4f);
		HoldDuration = 0.06f;
		FadeOutDuration = 0.10f;
		break;
	case EHitSeverity::Execute:
		FlashColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.8f);
		HoldDuration = 0.15f;
		FadeOutDuration = 0.20f;
		break;
	case EHitSeverity::Parry:
		FlashColor = FLinearColor(1.0f, 1.0f, 0.3f, 0.6f);
		HoldDuration = 0.10f;
		FadeOutDuration = 0.15f;
		break;
	case EHitSeverity::Block:
		FlashColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.1f);
		HoldDuration = 0.02f;
		FadeOutDuration = 0.04f;
		break;
	default:
		return;
	}

	CamMgr->StartCameraFade(0.0f, 1.0f, FadeInDuration, FlashColor, false, true);

	FTimerHandle FadeOutHandle;
	float TotalHold = FadeInDuration + HoldDuration;
	const_cast<USL_HitFeedbackManagerComponent*>(this)->GetWorld()->GetTimerManager().SetTimer(
		FadeOutHandle, FTimerDelegate::CreateWeakLambda(const_cast<USL_HitFeedbackManagerComponent*>(this),
		[CamMgr, FadeOutDuration]()
		{
			CamMgr->StartCameraFade(1.0f, 0.0f, FadeOutDuration, FLinearColor::White, false, false);
		}), TotalHold, false);
}

/************************************************************************/
/*                             工具函数                                 */
/************************************************************************/

float USL_HitFeedbackManagerComponent::GetShakeScale(EHitSeverity InSeverity) const
{
	switch (InSeverity)
	{
	case EHitSeverity::Light:   return Shake_Light;
	case EHitSeverity::Heavy:   return Shake_Heavy;
	case EHitSeverity::Execute: return Shake_Execute;
	case EHitSeverity::Parry:   return Shake_Parry;
	case EHitSeverity::Block:   return Shake_Block;
	default:                    return 1.0f;
	}
}
