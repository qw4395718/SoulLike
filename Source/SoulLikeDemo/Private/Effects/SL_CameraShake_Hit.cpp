// Effects/SL_CameraShake_Hit.cpp
// 打击感：相机震动 —— UE4.26 兼容版，使用旧版 UCameraShake + FOscillator
// 初始测试参数，根据实际手感调整

#include "SL_CameraShake_Hit.h"

USL_CameraShake_Hit::USL_CameraShake_Hit(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//// 震荡持续时间（-1表示无限，0表示无震荡）
	//OscillationDuration = 1.0f;

	//// 渐入渐出时间
	//OscillationBlendInTime = 0.1f;
	//OscillationBlendOutTime = 0.2f;

	//// === 位置震荡 ===
	//// X->角色朝向方向,一般来说这个方向的振幅就够了,本身也能代表fov的变化
	//LocOscillation.X.Amplitude = 10.0f;   // X轴振幅
	//LocOscillation.X.Frequency = 20.0f;   // X轴频率
	//LocOscillation.X.Waveform = EOscillatorWaveform::SineWave;

	//LocOscillation.Y.Amplitude = 0.0f;   // Y轴振幅
	//LocOscillation.Y.Frequency = 20.0f;   // Y轴频率

	//LocOscillation.Z.Amplitude = 0.0f;    // Z轴振幅
	//LocOscillation.Z.Frequency = 15.0f;   // Z轴频率

	//// === 旋转震荡 ===
	//RotOscillation.Pitch.Amplitude = 0.0f;
	//RotOscillation.Pitch.Frequency = 25.0f;

	//RotOscillation.Yaw.Amplitude = 0.0f;
	//RotOscillation.Yaw.Frequency = 25.0f;

	//RotOscillation.Roll.Amplitude = 0.0f;
	//RotOscillation.Roll.Frequency = 25.0f;

	//// === FOV震荡 ===
	//FOVOscillation.Amplitude = 0.0f;  // 不使用FOV变化
	//FOVOscillation.Frequency = 0.0f;
}
