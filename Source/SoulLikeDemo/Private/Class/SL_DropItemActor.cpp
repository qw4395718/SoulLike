// Private/Class/SL_DropItemActor.cpp
#include "SL_DropItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

ASL_DropItemActor::ASL_DropItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetGenerateOverlapEvents(false);

	NameplateWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameplateWidget"));
	NameplateWidget->SetupAttachment(RootComponent);
	NameplateWidget->SetWidgetSpace(EWidgetSpace::Screen);
	NameplateWidget->SetDrawSize(FVector2D(120, 30));
	NameplateWidget->SetRelativeLocation(FVector(0, 0, 50));
	NameplateWidget->SetVisibility(false);
}

void ASL_DropItemActor::InitializeDrop(FName InItemID, int32 InCount)
{
	ItemID = InItemID;
	ItemCount = InCount;

	// 60秒后自动销毁
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(
		DelayHandle,
		this,
		&ASL_DropItemActor::OnLifeTimeExpired,
		LifeTime,
		false
	);
}

void ASL_DropItemActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASL_DropItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASL_DropItemActor, ItemID);
	DOREPLIFETIME(ASL_DropItemActor, ItemCount);
}

// ===== IInteraction_IF =====

bool ASL_DropItemActor::CanInteraction()
{
	return !ItemID.IsNone();
}

FString ASL_DropItemActor::GetInteractionString()
{
	return FString::Printf(TEXT("按 E 拾取 %s x%d"), *ItemID.ToString(), ItemCount);
}

bool ASL_DropItemActor::ExeInteract()
{
	// 通过 PlayerController 获取 InventoryComponent 并添加物品
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return false;

	// 通过 PC 的 Server RPC 请求服务器执行拾取
	// 这里暂时只处理服务器端直接删除掉落物 Actor
	if (HasAuthority())
	{
		Destroy();
		return true;
	}

	return false;
}

int ASL_DropItemActor::GetInteractionUIWidgetIndex()
{
	return 0;
}

// ===== 内部回调 =====

void ASL_DropItemActor::OnRep_ItemID()
{
	// 可在此处更新可视化表现（根据 ItemID 显示不同模型）
}

void ASL_DropItemActor::OnLifeTimeExpired()
{
	Destroy();
}
