// Effects/SL_CameraShake_Hit.cpp

#include "SL_CameraShake_Hit.h"
#include <Camera/CameraShake.h>

USL_CameraShake_Hit::USL_CameraShake_Hit(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//// 创建 Perlin 噪声振荡模式（UE4.26 推荐方式）
	//UPerlinNoiseCameraShakePattern* Pattern =
	//	CreateDefaultSubobject<UPerlinNoiseCameraShakePattern>(TEXT("PerlinPattern"));
	//RootShakePattern = Pattern;

	//if (!Pattern) return;

	//// 基线设为"重击"级别
	//// 通过 StartCameraShake 的 Scale 参数缩放得到轻击(0.5x) / 处决(2.0x)
	//Pattern->Duration = 0.2f;
	//Pattern->BlendInTime = 0.02f;
	//Pattern->BlendOutTime = 0.1f;

	//// ---- 位置震荡（模拟冲击力） ----
	//Pattern->Location.X.Amplitude = 4.0f;
	//Pattern->Location.X.Frequency = 7.0f;
	//Pattern->Location.X.InitialOffset = EInitialOscillatorOffset::Random;

	//Pattern->Location.Y.Amplitude = 2.0f;
	//Pattern->Location.Y.Frequency = 6.0f;
	//Pattern->Location.Y.InitialOffset = EInitialOscillatorOffset::Random;

	//// ---- 旋转震荡（模拟视角晃动） ----
	//Pattern->Rotation.Pitch.Amplitude = 1.0f;
	//Pattern->Rotation.Pitch.Frequency = 6.0f;
	//Pattern->Rotation.Pitch.InitialOffset = EInitialOscillatorOffset::Random;

	//Pattern->Rotation.Yaw.Amplitude = 0.5f;
	//Pattern->Rotation.Yaw.Frequency = 6.0f;
	//Pattern->Rotation.Yaw.InitialOffset = EInitialOscillatorOffset::Random;

	//// ---- FOV 震荡（冲击感） ----
	//Pattern->FOV.Amplitude = 0.5f;
	//Pattern->FOV.Frequency = 8.0f;
}
