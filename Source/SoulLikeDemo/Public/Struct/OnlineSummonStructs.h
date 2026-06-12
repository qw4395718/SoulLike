#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OnlineSummonStructs.generated.h"

// ===== 召唤标记状态 =====
UENUM(BlueprintType)
enum class ESummonSignState : uint8
{
	Active			UMETA(DisplayName = "活跃"),
	BeingSummoned	UMETA(DisplayName = "被召唤中"),
	Expired			UMETA(DisplayName = "已过期"),
	Removed			UMETA(DisplayName = "已移除")
};

// ===== 玩家联机状态 =====
UENUM(BlueprintType)
enum class EOnlinePlayerState : uint8
{
	Solo				UMETA(DisplayName = "单人模式"),
	PlacingSign			UMETA(DisplayName = "放置标记中"),
	SignActive			UMETA(DisplayName = "标记已放置"),
	BeingSummoned		UMETA(DisplayName = "被召唤中"),
	SummonedAsPhantom	UMETA(DisplayName = "作为灵体"),
	SummoningOther		UMETA(DisplayName = "召唤其他玩家"),
	HasPhantom			UMETA(DisplayName = "世界中有灵体")
};

// ===== 灵体返回原因 =====
UENUM(BlueprintType)
enum class EReturnReason : uint8
{
	PhantomDied			UMETA(DisplayName = "灵体死亡"),
	SummonerDied		UMETA(DisplayName = "召唤者死亡"),
	BossDefeated		UMETA(DisplayName = "首领被击败"),
	ManualReturn		UMETA(DisplayName = "主动返回"),
	Disconnected		UMETA(DisplayName = "网络断开"),
	Timeout				UMETA(DisplayName = "超时")
};

// ===== 匹配配置 =====
USTRUCT(BlueprintType)
struct FSummonMatchConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LevelRange = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponLevelRange = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnablePassword = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SummonPassword;
};

// ===== 召唤标记数据 =====
USTRUCT(BlueprintType)
struct FSummonSignInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid SignID;

	UPROPERTY()
	FString OwnerPlayerName;

	UPROPERTY()
	int32 OwnerLevel = 0;

	UPROPERTY()
	int32 OwnerWeaponLevel = 0;

	UPROPERTY()
	FName CurrentLevelName;

	UPROPERTY()
	FTransform SignTransform;

	UPROPERTY()
	float TimeRemaining = 300.0f;

	UPROPERTY()
	ESummonSignState State = ESummonSignState::Active;

	UPROPERTY()
	FName WorldInstanceID;
};

// ===== 灵体数据（跨世界传输） =====
USTRUCT(BlueprintType)
struct FPhantomData
{
	GENERATED_BODY()

	// 角色外观
	UPROPERTY(BlueprintReadWrite)
	FString CharacterMeshPath;

	// 所有材质覆盖的路径列表（按材质索引顺序）
	UPROPERTY(BlueprintReadWrite)
	TArray<FString> MaterialPaths;

	// 动画蓝图路径
	UPROPERTY(BlueprintReadWrite)
	FString AnimBlueprintPath;

	// 装备武器路径列表
	UPROPERTY(BlueprintReadWrite)
	TArray<FString> EquipmentPaths;

	// 角色属性
	UPROPERTY(BlueprintReadWrite)
	int32 Level = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 WeaponLevel = 0;

	UPROPERTY(BlueprintReadWrite)
	float HealthPercent = 1.0f;

	// 身份标识
	UPROPERTY(BlueprintReadWrite)
	FString OwnerName;

	UPROPERTY(BlueprintReadWrite)
	FGuid SummonSessionID;

	// 可以转换为 JSON 字符串（供网络传输）
	FString ToJSON() const;
};
