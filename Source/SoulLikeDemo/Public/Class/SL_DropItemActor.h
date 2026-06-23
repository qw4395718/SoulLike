// Public/Class/SL_DropItemActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction_IF.h"
#include "SoulLikeGameGlobal.h"
#include "SL_DropItemActor.generated.h"

class UWidgetComponent;

UCLASS()
class SOULLIKEDEMO_API ASL_DropItemActor : public AActor, public IInteraction_IF
{
	GENERATED_BODY()

public:
	ASL_DropItemActor();

	/** 初始化掉落物 */
	void InitializeDrop(FName InItemID, int32 InCount);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ===== IInteraction_IF =====
	virtual bool CanInteraction() override;
	virtual FString GetInteractionString() override;
	virtual bool ExeInteract() override;
	virtual int GetInteractionUIWidgetIndex() override;

	UFUNCTION()
	void OnRep_ItemID();

	/** 超时自动销毁 */
	void OnLifeTimeExpired();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UWidgetComponent* NameplateWidget;

	/** 物品ID */
	UPROPERTY(ReplicatedUsing = OnRep_ItemID, BlueprintReadOnly, Category = "Drop")
	FName ItemID;

	/** 物品数量 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Drop")
	int32 ItemCount;

	/** 自动消失时间 */
	float LifeTime = 60.0f;
};
