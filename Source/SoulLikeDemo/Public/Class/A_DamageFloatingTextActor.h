// A_DamageFloatingTextActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoulLikeGameGlobal.h"
#include "A_DamageFloatingTextActor.generated.h"

class UWidgetComponent;

UCLASS()
class SOULLIKEDEMO_API A_DamageFloatingTextActor : public AActor
{
	GENERATED_BODY()

public:
	A_DamageFloatingTextActor();

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "Damage Floating")
	void ShowText(const FDamageFloatingTextData& InData);

	UFUNCTION(BlueprintCallable, Category = "Damage Floating")
	void HideAndRelease();

	UFUNCTION(BlueprintPure, Category = "Damage Floating")
	bool IsTextActive() const { return bIsActive; }

	void SetReleaseCallback(TFunction<void(A_DamageFloatingTextActor*)> InCallback)
	{
		ReleaseCallback = InCallback;
	}

protected:
	/************************************************************************/
	/* 继承实现                                                                     */
	/************************************************************************/
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	void BillboardToCamera();
	void UpdateFloatAndLifetime(float DeltaTime);
	class UUserWidget* GetDamageWidget() const;

protected:
	/************************************************************************/
	/* 内部访问                                                                     */
	/************************************************************************/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* m_widgetComponent;

	FDamageFloatingTextData CurrentData;
	bool bIsActive;

	// 随机偏移半径（锚点为中心的采样半径，0 则无偏移）
	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	float RandomOffsetRadius = 80.0f;

	// 飘字目标高度（起点到终点的垂直距离）
	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	float FloatHeight = 50.0f;

	// 终点缓动曲线指数（1.0=线性，>1.0=ease-out，<1.0=ease-in）
	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	float FloatEaseExponent = 1.8f;

	// 飘散锥体半角（度），在攻击方向周围随机偏转，0 = 不偏转
	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	float ScatterConeHalfAngle = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	float Lifetime = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	float MaxVisibleDistance = 3000.0f;

	float ElapsedTime;
	FVector StartLocation;
	FVector GoalLocation;

	float FloatSpeed;		// 已废弃，保留以兼容旧配置读取

	TFunction<void(A_DamageFloatingTextActor*)> ReleaseCallback;
};