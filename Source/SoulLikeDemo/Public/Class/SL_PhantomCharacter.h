#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Struct/OnlineSummonStructs.h"
#include "SL_PhantomCharacter.generated.h"

/**
 * 灵体角色
 * 在召唤者世界中生成，代表被召唤玩家的分身
 * 半透明外观，受限制的交互能力
 */
UCLASS()
class SOULLIKEDEMO_API ASL_PhantomCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASL_PhantomCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/************************************************************************/
	/*                               外部调用                               */
	/************************************************************************/
	// 用 PhantomData 初始化灵体外观
	UFUNCTION(BlueprintCallable, Category = "Phantom")
	void ApplyPhantomData(const FPhantomData& InData);

	// 获取灵体数据
	UFUNCTION(BlueprintPure, Category = "Phantom")
	const FPhantomData& GetPhantomData() const { return PhantomData; }

	// 设置能否与世界交互（拾取道具、开关门等）
	UFUNCTION(BlueprintCallable, Category = "Phantom")
	void SetInteractionEnabled(bool bEnabled);

	// 遣返灵体（返回自己的世界）
	UFUNCTION(BlueprintCallable, Category = "Phantom")
	void Repatriate(EReturnReason InReason);

protected:
	/************************************************************************/
	/*                               内部调用                               */
	/************************************************************************/
	// 重建灵体外观（网格体、材质、动画）
	void RebuildAppearance();

	// 应用半透明视觉效果
	void ApplyTranslucentEffect();

	// 限制灵体的物理/交互能力
	void ApplyInteractionRestrictions();

	// 属性复制回调
	UFUNCTION()
	void OnRep_PhantomData();

	/************************************************************************/
	/*                               内部访问                               */
	/************************************************************************/
	// 灵体数据
	UPROPERTY(ReplicatedUsing = OnRep_PhantomData)
	FPhantomData PhantomData;

	// 能否与世界交互
	UPROPERTY(Replicated)
	bool bCanInteractWithWorld;

	// 能否受到伤害
	UPROPERTY(Replicated)
	bool bCanBeDamagedByWorld;

	// 半透明材质的动态实例（用于调整透明度）
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> PhantomMaterials;

	// 半透明材质路径（可在蓝图中替换）
	UPROPERTY(EditDefaultsOnly, Category = "Phantom|Visual")
	TSoftObjectPtr<UMaterialInterface> PhantomMaterialOverride;
};