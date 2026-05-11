// Private/Save/USL_GameSaveData.cpp
#include "SL_GameSaveData.h"

USL_GameSaveData::USL_GameSaveData()
{
	ResetToDefault();
}

void USL_GameSaveData::ResetToDefault()
{
	SavedLevelID = 1;           // 默认从第1关开始
	SavedPlayerClassID = 1001;  // 默认职业ID
	SaveTime = FDateTime::Now();
}