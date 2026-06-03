#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "SL_ComboWindow_ANS.generated.h"

UENUM(BlueprintType)
enum class EComboWindowEvent : uint8
{
	BeginWindow     UMETA(DisplayName = "ComboWindowStart"),
	AllowBlend      UMETA(DisplayName = "ComboWindowBlendStart"),
	EndWindow       UMETA(DisplayName = "ComboWindowEnd")
};

UCLASS(meta = (DisplayName = "连招窗口通知"))
class SOULLIKEDEMO_API USL_ComboWindow_ANS : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
		FGameplayTag ComboWindowTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
		EComboWindowEvent WindowEvent;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	virtual FString GetNotifyName_Implementation() const override;
};