#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SoulLikeGameGlobal.h"
#include "SL_WeaponAnimSet.generated.h"

UCLASS(BlueprintType)
class SOULLIKEDEMO_API USL_WeaponAnimSet : public UDataAsset
{
    GENERATED_BODY()

public:
    // ===== 标识 =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EWeaponType WeaponType;

    // ===== 连招蒙太奇映射 =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    TMap<FGameplayTag, TSoftObjectPtr<UAnimMontage>> ComboMontages;

    // ===== 连招表 =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    TSoftObjectPtr<class UComboInfoTable> ComboInfoTable;

    // ===== 待机姿态 =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pose")
    TSoftObjectPtr<UAnimSequence> IdleAdditivePose;

    // ===== 持刀移动参数 =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float DrawMoveSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float AnimDrawPlayRate = 1.0f;

    // ===== 受击蒙太奇 =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reaction")
    TSoftObjectPtr<UAnimMontage> HitReactMontage;

public:
    UFUNCTION(BlueprintPure, Category = "WeaponAnimSet")
    UAnimMontage* GetComboMontageByTag(const FGameplayTag& InTag) const;
};
