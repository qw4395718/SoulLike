// Component/HitFeedback/SL_HitFeedbackManagerComponent.h
// 打击感反馈集中管理器 —— 统一管理 Hit Stop / Camera Shake / 屏幕特效

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <SoulLikeGameGlobal.h>
#include "SL_HitFeedbackManagerComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULLIKEDEMO_API USL_HitFeedbackManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USL_HitFeedbackManagerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ===== 命中反馈统一入口（绑定 GlobalDelegatesManager） =====
	UFUNCTION()
	void OnHitFeedback(const FHitFeedbackData& InData);

	// ===== Hit Stop =====
	void ApplyHitStop(const FHitFeedbackData& InData);
	UFUNCTION()
	void RestoreActorTimeDilation(AActor* InActor);

	// ===== Camera Shake =====
	void PlayHitCameraShake(EHitSeverity InSeverity, float InShakeScale) const;

	// ===== 屏幕特效 =====
	void PlayHitScreenEffect(EHitSeverity InSeverity) const;

	// 根据严重程度获取 ShakeScale
	float GetShakeScale(EHitSeverity InSeverity) const;

public:
	/************************************************************************/
	/*                   集中配置参数（后续需要改为根据招式的配置属性进行差值变化）                          */
	/************************************************************************/

	// ===== Hit Stop 时长（秒） =====
	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|HitStop")
	float HitStop_Light = 0.10f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|HitStop")
	float HitStop_Heavy = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|HitStop")
	float HitStop_Execute = 0.30f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|HitStop")
	float HitStop_Parry = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|HitStop")
	float HitStop_Block = 0.08f;

	// ===== Hit Stop 时间膨胀系数（越小越接近冻结） =====
	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|HitStop", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float HitStopDilation_Light = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|HitStop", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float HitStopDilation_Heavy = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|HitStop", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float HitStopDilation_Execute = 0.02f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|HitStop", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float HitStopDilation_Parry = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|HitStop", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float HitStopDilation_Block = 0.6f;

	// ===== Camera Shake 缩放 =====
	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|CameraShake")
	float Shake_Light = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|CameraShake")
	float Shake_Heavy = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|CameraShake")
	float Shake_Execute = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|CameraShake")
	float Shake_Parry = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "HitFeedback|CameraShake")
	float Shake_Block = 0.3f;

protected:
	// Hit Stop 定时器池（随组件生命周期，不随武器销毁丢失）
	UPROPERTY()
	TMap<AActor*, FTimerHandle> HitStopTimerHandles;

	// 委托句柄
	FDelegateHandle OnHitFeedbackHandle;
};
