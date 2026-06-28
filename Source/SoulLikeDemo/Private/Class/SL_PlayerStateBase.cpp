// Private/Class/SL_PlayerStateBase.cpp
#include "SL_PlayerStateBase.h"
#include "Net/UnrealNetwork.h"
#include "HUD_LobbyScreen.h"
#include "UIManagerSubsystem.h"
#include <SL_CharacterBase.h>
#include "EngineUtils.h"

ASL_PlayerStateBase::ASL_PlayerStateBase()
{
	bReplicates = true;
	PlayerClassID = 0; // 默认职业ID
}

/************************************************************************/
/*                               继承实现                               */
/************************************************************************/

void ASL_PlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASL_PlayerStateBase, PlayerClassID);
}

/************************************************************************/
/*                               外部调用                               */
/************************************************************************/

void ASL_PlayerStateBase::RequestSetClassID(int32 InNewClassID)
{
	if (HasAuthority())
	{
		// 服务器直接执行
		Server_SetClassID_Implementation(InNewClassID);
	}
	else
	{
		// 客户端发送 Server RPC
		Server_SetClassID(InNewClassID);
	}
}

/************************************************************************/
/*                              Server RPC                              */
/************************************************************************/

bool ASL_PlayerStateBase::Server_SetClassID_Validate(int32 InNewClassID)
{
	return InNewClassID >= 0;
}

void ASL_PlayerStateBase::Server_SetClassID_Implementation(int32 InNewClassID)
{
	PlayerClassID = InNewClassID;

	UE_LOG(LogTemp, Log, TEXT("ASL_PlayerStateBase::Server_SetClassID - Player %s set ClassID to %d"),
		*GetPlayerName(), PlayerClassID);
}

/************************************************************************/
/*                               内部调用                               */
/************************************************************************/

void ASL_PlayerStateBase::OnRep_PlayerClassID()
{
	UE_LOG(LogTemp, Log, TEXT("ASL_PlayerStateBase::OnRep_ClassID - ClassID replicated to %d"), PlayerClassID);

	// ===== UI 刷新（仅本机玩家） =====
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (PC->IsLocalController())
		{
			UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(PC);
			if (UIManager)
			{
				UUserWidget* Widget = UIManager->GetWidget(EWidgetType::EWIDGET_LobbyScreen);
				if (UHUD_LobbyScreen* Lobby = Cast<UHUD_LobbyScreen>(Widget))
				{
					Lobby->RefreshClassDisplay();
				}
			}
		}
	}

	// ===== 查找此 PlayerState 对应的角色 =====
	// 方式一：通过 Owner PC 获取（本机玩家）
	ASL_CharacterBase* TargetPawn = nullptr;
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		TargetPawn = Cast<ASL_CharacterBase>(PC->GetPawn());
	}

	// 方式二：远程玩家（PC 不在本客户端），遍历世界中的 Pawn 匹配 PlayerState
	if (!TargetPawn && GetWorld())
	{
		for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
		{
			ACharacter* Pawn = *It;
			if (Pawn && Pawn->GetPlayerState() == this)
			{
				TargetPawn = Cast<ASL_CharacterBase>(Pawn);
				break;
			}
		}
	}

	// ===== 应用外观 =====
	if (TargetPawn)
	{
		TargetPawn->ApplyClassAppearance(PlayerClassID);
	}

	// ===== [诊断] 验证远程玩家 Pawn 匹配失败的原因 =====
	if (!TargetPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnRep_PlayerClassID] FAILED to find Pawn | PS=%s | Role=%d | Owner=%s | ClassID=%d"),
			*GetName(),
			(int32)GetLocalRole(),
			*GetNameSafe(GetOwner()),
			PlayerClassID);

		if (GetWorld())
		{
			int32 PawnIdx = 0;
			for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
			{
				ACharacter* Pawn = *It;
				UE_LOG(LogTemp, Log, TEXT("  [Pawn %d] Char=%s | GetPlayerState=%s | GetController=%s | Controller.PlayerState=%s"),
					PawnIdx++,
					*GetNameSafe(Pawn),
					*GetNameSafe(Pawn ? Pawn->GetPlayerState() : nullptr),
					*GetNameSafe(Pawn ? Pawn->GetController() : nullptr),
					*GetNameSafe(Pawn && Pawn->GetController() ? Pawn->GetController()->PlayerState : nullptr));
			}
		}

	}
	
}
