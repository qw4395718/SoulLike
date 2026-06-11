#include "SL_SummonSign.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Manager/SL_SignManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include <UObject/ConstructorHelpers.h>
#include <GameFramework/Character.h>

ASL_SummonSign::ASL_SummonSign()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;	// 每秒更新一次，不需要高频Tick

	bReplicates = true;
	bIsLocked = false;

	// 场景根组件
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// 交互碰撞体
	InteractionCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollision"));
	InteractionCollision->SetupAttachment(RootComponent);
	InteractionCollision->SetBoxExtent(FVector(100, 100, 100));
	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionCollision->SetGenerateOverlapEvents(true);

	// 可视化网格体
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 名称显示组件
	NameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidget"));
	NameWidget->SetupAttachment(RootComponent);
	NameWidget->SetRelativeLocation(FVector(0, 0, 120));
	NameWidget->SetWidgetSpace(EWidgetSpace::Screen);
	NameWidget->SetDrawSize(FVector2D(200, 40));

	// SL_SummonSign.cpp 构造函数末尾
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GlowMat(TEXT("/Game/SoulLikeDemo/Materials/MI_SummonGlow_Inst.MI_SummonGlow_Inst"));
	if (GlowMat.Succeeded())
	{
		VisualMesh->SetMaterial(0, GlowMat.Object);
	}
}

void ASL_SummonSign::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASL_SummonSign, SignInfo);
	DOREPLIFETIME(ASL_SummonSign, bIsLocked);
}

void ASL_SummonSign::BeginPlay()
{
	Super::BeginPlay();

	// 绑定重叠委托
	if (InteractionCollision)
	{
		InteractionCollision->OnComponentBeginOverlap.AddDynamic(this, &ASL_SummonSign::OnInteractionOverlapBegin);
		InteractionCollision->OnComponentEndOverlap.AddDynamic(this, &ASL_SummonSign::OnInteractionOverlapEnd);
	}

	// 注册到 SignManager
	if (HasAuthority())
	{
		if (!bIsRemoteSign) RegisterToManager();
	}
}

void ASL_SummonSign::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 服务器端更新标记过期时间
	if (HasAuthority())
	{
		SignInfo.TimeRemaining -= DeltaTime;
		if (SignInfo.TimeRemaining <= 0.0f && !bIsLocked)
		{
			OnSignExpired();
		}
	}
}

void ASL_SummonSign::InitializeSign(const FSummonSignInfo& InSignInfo)
{
	SignInfo = InSignInfo;
	SignInfo.State = ESummonSignState::Active;
	bIsLocked = false;

	// 设置位置到标记所在位置
	SetActorTransform(SignInfo.SignTransform);

	UE_LOG(LogTemp, Log, TEXT("ASL_SummonSign::InitializeSign - Sign %s initialized"), *SignInfo.SignID.ToString());
}

void ASL_SummonSign::LockSign()
{
	bIsLocked = true;
	SignInfo.State = ESummonSignState::BeingSummoned;

	// 锁定后禁用碰撞，防止重复交互
	if (InteractionCollision)
	{
		InteractionCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	UE_LOG(LogTemp, Log, TEXT("ASL_SummonSign::LockSign - Sign %s locked"), *SignInfo.SignID.ToString());
}

void ASL_SummonSign::RemoveSign()
{
	SignInfo.State = ESummonSignState::Removed;
	UnregisterFromManager();

	// 延迟销毁，给客户端播放消失动画的时间
	SetLifeSpan(0.5f);
}

void ASL_SummonSign::OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	// Phase2: 追踪重叠角色（用于交互键确认目标）
	if (OtherActor->IsA<ACharacter>() && !OverlappingPlayers.Contains(OtherActor))
	{
		OverlappingPlayers.Add(OtherActor);
	}

	UE_LOG(LogTemp, Verbose, TEXT("ASL_SummonSign::OnInteractionOverlapBegin - %s entered sign %s range"),
		*OtherActor->GetName(), *SignInfo.SignID.ToString());
}

void ASL_SummonSign::OnInteractionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Phase2: 移除重叠角色追踪
	if (OtherActor)
	{
		OverlappingPlayers.Remove(OtherActor);
	}
}

bool ASL_SummonSign::IsOverlappedBy(const AActor* InActor) const
{
	for (const TWeakObjectPtr<AActor>& Ptr : OverlappingPlayers)
	{
		if (Ptr.Get() == InActor)
		{
			return true;
		}
	}
	return false;
}

void ASL_SummonSign::OnSignExpired()
{
	if (SignInfo.State != ESummonSignState::Active)
	{
		return;
	}

	SignInfo.State = ESummonSignState::Expired;
	UE_LOG(LogTemp, Verbose, TEXT("ASL_SummonSign::OnSignExpired - Sign %s expired"), *SignInfo.SignID.ToString());

	UnregisterFromManager();
	Destroy();
}

void ASL_SummonSign::RegisterToManager()
{
	if (!HasAuthority()) return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	USL_SignManager* SignManager = GameInstance->GetSubsystem<USL_SignManager>();
	if (SignManager)
	{
		SignManager->RegisterSign(SignInfo);
	}
}

void ASL_SummonSign::UnregisterFromManager()
{
	if (!HasAuthority()) return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	USL_SignManager* SignManager = GameInstance->GetSubsystem<USL_SignManager>();
	if (SignManager)
	{
		SignManager->UnregisterSign(SignInfo.SignID);
	}
}

void ASL_SummonSign::OnRep_SignInfo()
{
	// 客户端收到标记数据更新时的回调
	// 可用于更新 UI 显示或视觉状态
}