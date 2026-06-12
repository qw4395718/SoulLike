// Private/Component/Online/SL_MatchClientSubsystem.cpp

#include "SL_MatchClientSubsystem.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/PlatformProcess.h"

USL_MatchClientSubsystem::USL_MatchClientSubsystem()
	: ServerPort(7777)
	, MatchSocket(nullptr)
	, SocketSubsystem(nullptr)
	, bIsConnected(false)
{
}

void USL_MatchClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Phase 2 验证：延迟一帧自动连接中间匹配服务
	FTimerHandle DummyHandle;
	GetGameInstance()->GetTimerManager().SetTimer(DummyHandle,
		FTimerDelegate::CreateLambda([this]()
		{
			if (Connect(TEXT("127.0.0.1"), 7777))
			{
				FString UniqueID = FString::Printf(TEXT("Instance_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Short));
			RegisterInstance(UniqueID, GetGameInstance()->GetWorld()->GetMapName(), TEXT("127.0.0.1"), 17777);
			}
		}), 0.1f, false);
}

void USL_MatchClientSubsystem::Deinitialize()
{
	Disconnect();
	Super::Deinitialize();
}

/************************************************************************/
/*                               外部调用                               */
/************************************************************************/

bool USL_MatchClientSubsystem::Connect(const FString& InIP, int32 InPort)
{
	if (bIsConnected)
	{
		Disconnect();
	}

	ServerIP = InIP;
	ServerPort = InPort;

	SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("SL_MatchClientSubsystem::Connect - Failed to get socket subsystem"));
		return false;
	}

	MatchSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("MatchClientSocket"), false);
	if (!MatchSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("SL_MatchClientSubsystem::Connect - Failed to create socket"));
		return false;
	}

	MatchSocket->SetNonBlocking(true);

	TSharedRef<FInternetAddr> RemoteAddr = SocketSubsystem->CreateInternetAddr();
	bool bIsValidIP = false;
	RemoteAddr->SetIp(*InIP, bIsValidIP);
	RemoteAddr->SetPort(InPort);

	if (!bIsValidIP)
	{
		UE_LOG(LogTemp, Error, TEXT("SL_MatchClientSubsystem::Connect - Invalid IP: %s"), *InIP);
		CleanUpSocket();
		return false;
	}

	MatchSocket->Connect(*RemoteAddr);

	double StartTime = FPlatformTime::Seconds();
	double Timeout = 3.0;

	while (FPlatformTime::Seconds() - StartTime < Timeout)
	{
		if (MatchSocket->GetConnectionState() == SCS_Connected)
		{
			bIsConnected = true;
			break;
		}

		bool bPending = false;
		if (MatchSocket->HasPendingConnection(bPending) && bPending)
		{
			FPlatformProcess::Sleep(0.05f);
			continue;
		}

		FPlatformProcess::Sleep(0.1f);
	}

	if (!bIsConnected && MatchSocket->GetConnectionState() == SCS_Connected)
	{
		bIsConnected = true;
	}

	if (!bIsConnected)
	{
		UE_LOG(LogTemp, Error, TEXT("SL_MatchClientSubsystem::Connect - Connection to %s:%d timed out"),
			*InIP, InPort);
		CleanUpSocket();
		return false;
	}

	TickHandle = FTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &USL_MatchClientSubsystem::Tick),
		0.05f
	);

	UE_LOG(LogTemp, Log, TEXT("SL_MatchClientSubsystem::Connect - Connected to match server %s:%d"),
		*InIP, InPort);

	OnConnected.Broadcast(true);
	return true;
}

void USL_MatchClientSubsystem::Disconnect()
{
	if (TickHandle.IsValid())
	{
		FTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}

	if (bIsConnected && !InstanceID.IsEmpty())
	{
		SendMessage(FString::Printf(
			TEXT("{\"type\":\"unregister_instance\",\"instance_id\":\"%s\"}"),
			*InstanceID));
	}

	CleanUpSocket();
	bIsConnected = false;
	InstanceID.Empty();
	ReceiveBuffer.Empty();

	UE_LOG(LogTemp, Log, TEXT("SL_MatchClientSubsystem::Disconnect - Disconnected from match server"));
}

/************************************************************************/
/*                         消息发送                                     */
/************************************************************************/

void USL_MatchClientSubsystem::RegisterInstance(const FString& InInstanceID,
	const FString& InMapName, const FString& InIP, int32 InPort)
{
	InstanceID = InInstanceID;

	FString Msg = FString::Printf(
		TEXT("{\"type\":\"register_instance\",\"instance_id\":\"%s\",\"map\":\"%s\",\"ip\":\"%s\",\"port\":%d}"),
		*InInstanceID, *InMapName, *InIP, InPort);
	SendMessage(Msg);
}

void USL_MatchClientSubsystem::UnregisterInstance()
{
	FString Msg = FString::Printf(
		TEXT("{\"type\":\"unregister_instance\",\"instance_id\":\"%s\"}"),
		*InstanceID);
	SendMessage(Msg);
}

void USL_MatchClientSubsystem::RegisterSign(const FString& InOwnerName,
	int32 InLevel, int32 InWeaponLevel, const FString& InMapName,
	const FString& InTransformJSON, float InTimeRemaining)
{
	FString Msg = FString::Printf(
		TEXT("{\"type\":\"register_sign\",\"sign_data\":{")
		TEXT("\"owner_name\":\"%s\",\"owner_level\":%d,\"weapon_level\":%d,")
		TEXT("\"map\":\"%s\",\"transform\":%s,\"time_remaining\":%f}")
		TEXT("}"),
		*InOwnerName, InLevel, InWeaponLevel, *InMapName, *InTransformJSON, InTimeRemaining);
	SendMessage(Msg);
}

void USL_MatchClientSubsystem::UnregisterSign(const FString& InSignID)
{
	FString Msg = FString::Printf(
		TEXT("{\"type\":\"unregister_sign\",\"sign_id\":\"%s\"}"),
		*InSignID);
	SendMessage(Msg);
}

void USL_MatchClientSubsystem::QuerySigns(const FString& InMapName,
	int32 InLevel, int32 InWeaponLevel)
{
	FString Msg = FString::Printf(
		TEXT("{\"type\":\"query_signs\",\"map\":\"%s\",\"level\":%d,\"weapon_level\":%d}"),
		*InMapName, InLevel, InWeaponLevel);
	SendMessage(Msg);
}

void USL_MatchClientSubsystem::RequestSummon(const FString& InSignID,
	const FString& InRequesterName, const FString& InRequesterInstance,
	int32 InRequesterLevel)
{
	FString Msg = FString::Printf(
		TEXT("{\"type\":\"request_summon\",\"sign_id\":\"%s\",")
		TEXT("\"requester_name\":\"%s\",\"requester_instance\":\"%s\",\"requester_level\":%d}"),
		*InSignID, *InRequesterName, *InRequesterInstance, InRequesterLevel);
	SendMessage(Msg);
}

void USL_MatchClientSubsystem::AcceptSummon(const FString& InSignID,
	const FString& InRequesterInstance)
{
	FString Msg = FString::Printf(
		TEXT("{\"type\":\"accept_summon\",\"sign_id\":\"%s\",\"requester_instance\":\"%s\"}"),
		*InSignID, *InRequesterInstance);
	SendMessage(Msg);
}

void USL_MatchClientSubsystem::DeclineSummon(const FString& InSignID,
	const FString& InRequesterInstance)
{
	FString Msg = FString::Printf(
		TEXT("{\"type\":\"decline_summon\",\"sign_id\":\"%s\",\"requester_instance\":\"%s\"}"),
		*InSignID, *InRequesterInstance);
	SendMessage(Msg);
}

void USL_MatchClientSubsystem::TransferPhantomData(const FString& InTargetInstance,
	const FString& InDataJSON)
{
	FString Msg = FString::Printf(
		TEXT("{\"type\":\"transfer_phantom_data\",\"target_instance\":\"%s\",\"data\":%s}"),
		*InTargetInstance, *InDataJSON);
	SendMessage(Msg);
}

/************************************************************************/
/*                               内部调用                               */
/************************************************************************/

void USL_MatchClientSubsystem::SendMessage(const FString& InJSON)
{
	if (!MatchSocket || !bIsConnected) return;

	FTCHARToUTF8 Converter(*InJSON);
	TArray<uint8> Data;
	Data.Append(reinterpret_cast<const uint8*>(Converter.Get()), Converter.Length());
	Data.Add(static_cast<uint8>('\n'));

	int32 BytesSent = 0;
	if (!MatchSocket->Send(Data.GetData(), Data.Num(), BytesSent))
	{
		UE_LOG(LogTemp, Verbose, TEXT("SL_MatchClientSubsystem::SendMessage - Send failed"));
	}
}

bool USL_MatchClientSubsystem::Tick(float DeltaTime)
{
	if (!MatchSocket || !bIsConnected) return true;

	if (MatchSocket->GetConnectionState() != SCS_Connected)
	{
		UE_LOG(LogTemp, Warning, TEXT("SL_MatchClientSubsystem::Tick - Connection lost"));
		bIsConnected = false;
		OnConnected.Broadcast(false);
		return true;
	}

	uint32 PendingDataSize = 0;
	while (MatchSocket->HasPendingData(PendingDataSize) && PendingDataSize > 0)
	{
		TArray<uint8> TempBuffer;
		TempBuffer.SetNumUninitialized(PendingDataSize);
		int32 BytesRead = 0;

		if (!MatchSocket->Recv(TempBuffer.GetData(), TempBuffer.Num(), BytesRead, ESocketReceiveFlags::None))
		{
			bIsConnected = false;
			OnConnected.Broadcast(false);
			return true;
		}

		ReceiveBuffer.Append(
			UTF8_TO_TCHAR(reinterpret_cast<const char*>(TempBuffer.GetData())),
			BytesRead);

		int32 NewLineIdx;
		while (ReceiveBuffer.FindChar('\n', NewLineIdx))
		{
			FString Line = ReceiveBuffer.Left(NewLineIdx);
			ReceiveBuffer = ReceiveBuffer.Mid(NewLineIdx + 1);
			Line.TrimStartAndEndInline();

			if (!Line.IsEmpty())
			{
				ProcessMessage(Line);
			}
		}
	}

	return true;
}

void USL_MatchClientSubsystem::ProcessMessage(const FString& InLine)
{
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InLine);
	TSharedPtr<FJsonObject> JsonObj;
	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SL_MatchClientSubsystem::ProcessMessage - Failed to parse JSON: %s"),
			*InLine.Left(200));
		return;
	}

	FString MsgType = JsonObj->GetStringField(TEXT("type"));

	if (MsgType == TEXT("register_instance_ack"))
	{
		bool bSuccess = JsonObj->GetBoolField(TEXT("success"));
		UE_LOG(LogTemp, Log, TEXT("MatchClient: Instance registration %s"),
			bSuccess ? TEXT("succeeded") : TEXT("failed"));
	}
	else if (MsgType == TEXT("register_sign_ack"))
	{
		bool bSuccess = JsonObj->GetBoolField(TEXT("success"));
		FString SignID = JsonObj->GetStringField(TEXT("sign_id"));
		UE_LOG(LogTemp, Log, TEXT("MatchClient: Sign %s registered (success=%d)"),
			*SignID, bSuccess);
	}
	else if (MsgType == TEXT("query_signs_result"))
	{
		FString ResultJSON;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultJSON);
		FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
		OnSignQueryResult.Broadcast(ResultJSON);
	}
	else if (MsgType == TEXT("summon_request"))
	{
		FString SignID = JsonObj->GetStringField(TEXT("sign_id"));
		FString RequesterName = JsonObj->GetStringField(TEXT("requester_name"));
		FString RequesterInstance = JsonObj->GetStringField(TEXT("requester_instance"));
		FString RequesterIP = JsonObj->GetStringField(TEXT("requester_ip"));
		int32 RequesterPort = JsonObj->GetIntegerField(TEXT("requester_port"));
		OnSummonRequested.Broadcast(SignID, RequesterName, RequesterInstance, RequesterIP, RequesterPort);
	}
	else if (MsgType == TEXT("request_summon_ack"))
	{
		bool bSuccess = JsonObj->GetBoolField(TEXT("success"));
		UE_LOG(LogTemp, Log, TEXT("MatchClient: Summon request %s"),
			bSuccess ? TEXT("sent") : TEXT("failed"));
	}
	else if (MsgType == TEXT("summon_accepted"))
	{
		FString SignID = JsonObj->GetStringField(TEXT("sign_id"));
		OnSummonAccepted.Broadcast(SignID);
	}
	else if (MsgType == TEXT("summon_declined"))
	{
		FString SignID = JsonObj->GetStringField(TEXT("sign_id"));
		OnSummonDeclined.Broadcast(SignID);
	}
	else if (MsgType == TEXT("phantom_data_received"))
	{
		TSharedPtr<FJsonObject> DataObj = JsonObj->GetObjectField(TEXT("data"));
		if (DataObj.IsValid())
		{
			FString DataJSON;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&DataJSON);
			FJsonSerializer::Serialize(DataObj.ToSharedRef(), Writer);
			OnPhantomDataReceived.Broadcast(DataJSON);
		}
	}
	else if (MsgType == TEXT("pong"))
	{
	}
	else if (MsgType == TEXT("error"))
	{
		FString ErrorMsg = JsonObj->GetStringField(TEXT("message"));
		UE_LOG(LogTemp, Warning, TEXT("MatchClient: Server error - %s"), *ErrorMsg);
	}
}

void USL_MatchClientSubsystem::CleanUpSocket()
{
	if (MatchSocket)
	{
		MatchSocket->Close();
		SocketSubsystem->DestroySocket(MatchSocket);
		MatchSocket = nullptr;
	}
}