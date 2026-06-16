// Effects/SL_CameraShake_Hit.h
// 打击感：相机震动 —— Perlin 噪声 CameraShake

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "SL_CameraShake_Hit.generated.h"

UCLASS()
class SOULLIKEDEMO_API USL_CameraShake_Hit : public UCameraShakeBase
{
	GENERATED_BODY()

public:
	USL_CameraShake_Hit(const FObjectInitializer& ObjectInitializer);
};
