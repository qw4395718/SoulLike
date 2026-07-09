// A_DamageFloatingTextActor.cpp
#include "A_DamageFloatingTextActor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"
//#include "Engine/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

A_DamageFloatingTextActor::A_DamageFloatingTextActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	m_widgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidget"));
	RootComponent = m_widgetComponent;

	m_widgetComponent->SetWidgetSpace(EWidgetSpace::World);
	m_widgetComponent->SetDrawSize(FVector2D(200.0f, 60.0f));
	m_widgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	m_widgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_widgetComponent->SetVisibility(false);

	bIsActive = false;
	ElapsedTime = 0.0f;
	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);
}

void A_DamageFloatingTextActor::BeginPlay()
{
	Super::BeginPlay();
}

void A_DamageFloatingTextActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsActive) return;

	BillboardToCamera();
	UpdateFloatAndLifetime(DeltaTime);
}

void A_DamageFloatingTextActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseCallback = nullptr;
	Super::EndPlay(EndPlayReason);
}

/************************************************************************/
/* 外部调用                                                                     */
/************************************************************************/

void A_DamageFloatingTextActor::ShowText(const FDamageFloatingTextData& InData)
{
	CurrentData = InData;
	ElapsedTime = 0.0f;

	FVector RandomOffset;
	FVector FloatDirection = FVector::UpVector;
	if (!InData.AttackSourceLocation.IsZero())
	{
		// 基准方向 = 攻击者 →→ 受击点
		const FVector BaseDirection = (InData.HitWorldLocation - InData.AttackSourceLocation).GetSafeNormal();

		// 飘散方向 = 基准方向 + 随机锥体偏差
		FloatDirection = BaseDirection;
		if (ScatterConeHalfAngle > 0.0f)
		{
			FloatDirection = FMath::VRandCone(FloatDirection, FMath::DegreesToRadians(ScatterConeHalfAngle));
		}

		// 初始位置：从受击点沿 -基准方向 回退（向攻击者方向），确保在受击者前方
		const float BackwardDist = FMath::FRandRange(0.0f, RandomOffsetRadius);
		RandomOffset = -BaseDirection * BackwardDist;
	}
	else
	{
		// 无攻击者时回退到水平面随机偏移
		const float two_pi = 6.3;
		const float Angle = FMath::FRandRange(0.0f, two_pi);
		const float Radius = FMath::FRandRange(0.0f, RandomOffsetRadius);
		RandomOffset = FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
	}

	StartLocation = InData.HitWorldLocation + RandomOffset;
	GoalLocation = StartLocation + FloatDirection * FloatHeight;
	SetActorLocation(StartLocation);

	UUserWidget* Widget = GetDamageWidget();
	if (Widget)
	{
		UTextBlock* DamageText = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("m_damageText")));
		if (DamageText)
		{
			const FString DamageStr = FString::Printf(TEXT("%d"), FMath::RoundToInt(InData.DamageValue));
			DamageText->SetText(FText::FromString(DamageStr));

			if (InData.bIsCriticalHit)
			{
				DamageText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.84f, 0.0f)));
				DamageText->SetRenderTransform(FWidgetTransform(
					FVector2D::ZeroVector,
					FVector2D(1.5f, 1.5f),
					FVector2D(0.5f, 0.5f),
					0.0f));
			}
			else
			{
				DamageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
				DamageText->SetRenderTransform(FWidgetTransform(
					FVector2D::ZeroVector,
					FVector2D(1.0f, 1.0f),
					FVector2D(0.5f, 0.5f),
					0.0f));
			}
		}

		UWidgetAnimation* FloatAnim = Cast<UWidgetAnimation>(Widget->GetWidgetFromName(TEXT("FloatFade")));
		if (FloatAnim)
		{
			Widget->PlayAnimation(FloatAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
		}
		if (InData.bIsCriticalHit)
		{
			UWidgetAnimation* CritAnim = Cast<UWidgetAnimation>(Widget->GetWidgetFromName(TEXT("CritScale")));
			if (CritAnim)
			{
				Widget->PlayAnimation(CritAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
			}
		}
	}

	bIsActive = true;
	m_widgetComponent->SetVisibility(true);
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
}

void A_DamageFloatingTextActor::HideAndRelease()
{
	if (!bIsActive) return;

	bIsActive = false;
	m_widgetComponent->SetVisibility(false);
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);

	UUserWidget* Widget = GetDamageWidget();
	if (Widget)
	{
		Widget->StopAllAnimations();
	}

	if (ReleaseCallback)
	{
		ReleaseCallback(this);
	}
}

/************************************************************************/
/* 内部调用                                                                     */
/************************************************************************/

void A_DamageFloatingTextActor::BillboardToCamera()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerCameraManager* CamManager = UGameplayStatics::GetPlayerCameraManager(World, 0);
	if (!CamManager) return;

	const FVector CameraLocation = CamManager->GetCameraLocation();
	const FVector MyLocation = GetActorLocation();

	FRotator LookAtRotation = (CameraLocation - MyLocation).Rotation();
	LookAtRotation.Pitch = 0.0f;
	LookAtRotation.Roll = 0.0f;
	SetActorRotation(LookAtRotation);
}

void A_DamageFloatingTextActor::UpdateFloatAndLifetime(float DeltaTime)
{
	ElapsedTime += DeltaTime;
	const float Alpha = FMath::Clamp(ElapsedTime / Lifetime, 0.0f, 1.0f);

	// 目标导向插值：用缓动曲线控制速度，ease-out = 开始快、终点慢
	const float EasedAlpha = 1.0f - FMath::Pow(1.0f - Alpha, FloatEaseExponent);
	const FVector NewLocation = FMath::Lerp(StartLocation, GoalLocation, EasedAlpha);
	SetActorLocation(NewLocation);

	if (ElapsedTime >= Lifetime)
	{
		HideAndRelease();
	}
}

UUserWidget* A_DamageFloatingTextActor::GetDamageWidget() const
{
	if (m_widgetComponent)
	{
		return m_widgetComponent->GetUserWidgetObject();
	}
	return nullptr;
}
