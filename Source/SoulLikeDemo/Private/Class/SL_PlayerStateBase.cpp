// Private/Class/SL_PlayerStateBase.cpp
#include "SL_PlayerStateBase.h"
#include "Net/UnrealNetwork.h"
#include "HUD_LobbyScreen.h"
#include "UIManagerSubsystem.h"
#include <SL_CharacterBase.h>

ASL_PlayerStateBase::ASL_PlayerStateBase()
{
	PlayerClassID = 1001; // 默认职业ID
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
		*GetPlayerName(), InNewClassID);
}

/************************************************************************/
/*                               内部调用                               */
/************************************************************************/

void ASL_PlayerStateBase::OnRep_PlayerClassID()
{
	UE_LOG(LogTemp, Log, TEXT("ASL_PlayerStateBase::OnRep_ClassID - ClassID replicated to %d"), PlayerClassID);

	// 如果当前拥有者是本机客户端，通知 LobbyScreen 刷新职业名
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC && PC->IsLocalController())
	{
		// UI上面的同步
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

	// 角色同步
	if (ASL_CharacterBase* PosseedActor = Cast<ASL_CharacterBase>(GetPawn()))
	{
		PosseedActor->SetClassID(PlayerClassID);
	}
	
}
