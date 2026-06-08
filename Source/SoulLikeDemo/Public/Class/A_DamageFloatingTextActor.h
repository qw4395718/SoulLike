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

	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	float FloatSpeed = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	float Lifetime = 3.0f;

	float ElapsedTime;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	float MaxVisibleDistance = 3000.0f;

	TFunction<void(A_DamageFloatingTextActor*)> ReleaseCallback;
};
