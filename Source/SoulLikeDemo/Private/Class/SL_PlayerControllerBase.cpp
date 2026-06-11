// Private/Class/SL_PlayerControllerBase.cpp
#include "SL_PlayerControllerBase.h"
#include "SL_SummonSign.h"
#include "UMG/HUD/HUD_InterActBtnPanel.h"
#include "SL_CharacterBase.h"
#include "SL_InventoryComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include <UIManagerSubsystem.h>
#include <HUD_ItemUseUI.h>
#include <SL_SummonSessionComponent.h>
#include <SL_ComboManagerComponent.h>
#include <HUD_BeginPlayScreen.h>
#include <SL_GameModeBase.h>
#include <GlobalDelegatesManager.h>
#include "LevelManager.h"
#include "HUD_PauseMenuScreen.h"
#include "Pop_DeathScreen.h"
#include "Kismet/GameplayStatics.h"
#include "DamageFloatingTextManagerComponent.h"

// 构造函数
ASL_PlayerControllerBase::ASL_PlayerControllerBase()
{
	// 设置输入模式默认值
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	UIManager = nullptr;
	bIsUIInitialized = false;
	FloatingTextManager = nullptr;
	SummonSessionCmp = nullptr;
}

// 游戏开始
void ASL_PlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	// 创建伤害飘字管理器
	FloatingTextManager = NewObject<UDamageFloatingTextManagerComponent>(this, TEXT("FloatingTextManager"));
	if (FloatingTextManager)
	{
		FloatingTextManager->RegisterComponent();
		UE_LOG(LogTemp, Log, TEXT("DamageFloatingTextManagerComponent created"));
	}

	// 创建联机召唤会话组件
	SummonSessionCmp = NewObject<USL_SummonSessionComponent>(this, TEXT("SummonSession"));
	if (SummonSessionCmp)
	{
		SummonSessionCmp->RegisterComponent();
		UE_LOG(LogTemp, Log, TEXT("SummonSessionComponent created"));
	}

	// 设置初始输入模式为游戏模式
	SetInputModeGameOnly();

	// 获取子系统
	UIManager = UUIManagerSubsystem::Get(this);

	 // 延迟一帧执行，确保GameMode已经初始化完成
    FTimerHandle DelayHandle;
    GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateLambda([this]()
    {
        // 显示开始界面
        ShowBeginPlayScreen();

	// 客户端启动定期扫描召唤标记（用于显示交互提示）
	if (GetNetMode() != NM_DedicatedServer)
	{
		GetWorld()->GetTimerManager().SetTimer(SummonSignScanTimerHandle, this, &ASL_PlayerControllerBase::Client_CheckNearbySummonSigns, 0.15f, true);
	}

        // 绑定角色死亡事件监听（需在 GameMode 初始化完成后）
        BindPlayerDeathEvent();
    }), 0.1f, false);

}

void ASL_PlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 清理UI
    HideBeginPlayScreen();

    Super::EndPlay(EndPlayReason);
}

// 当控制器控制一个Pawn时调用
void ASL_PlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 检查是否为我们的角色类
	if (ASL_CharacterBase* MyCharacter = Cast<ASL_CharacterBase>(InPawn))
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerController possessed: %s"), *MyCharacter->GetName());
	}
}

// 当控制器放弃控制一个Pawn时调用
void ASL_PlayerControllerBase::OnUnPossess()
{
	// 清除所有缓存引用
	CachedComboManager.Reset();
	CachedEquipmentComp.Reset();
	CachedInventoryComp.Reset();

	UE_LOG(LogTemp, Log, TEXT("PlayerController unpossessed"));

	Super::OnUnPossess();
}

void ASL_PlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (!InputComponent) return;

	// LockOn / 索敌
	InputComponent->BindAction("LockOn", IE_Pressed, this, &ASL_PlayerControllerBase::OnLockOnPressed);
	// 暂停菜单
	InputComponent->BindAction("OpenPauseMenu", IE_Pressed, this, &ASL_PlayerControllerBase::OnOpenPauseMenuPressed);

	// 方式一：使用 EInputEvent 精确控制按键时序
	// IE_Pressed:  按下瞬间
	// IE_Released: 松开瞬间
	// IE_Repeat:   按住持续触发

	// 轻攻击（X键 / 鼠标左键）
	InputComponent->BindAction("LightAttack", IE_Pressed, this, &ASL_PlayerControllerBase::OnLightAttackPressed);
	InputComponent->BindAction("LightAttack", IE_Released, this, &ASL_PlayerControllerBase::OnLightAttackReleased);

	// 重攻击（Y键 / 鼠标右键）
	InputComponent->BindAction("HeavyAttack", IE_Pressed, this, &ASL_PlayerControllerBase::OnHeavyAttackPressed);
	InputComponent->BindAction("HeavyAttack", IE_Released, this, &ASL_PlayerControllerBase::OnHeavyAttackReleased);

	// 特殊攻击（B键）
	InputComponent->BindAction("SpecialAttack", IE_Pressed, this, &ASL_PlayerControllerBase::OnSpecialAttackPressed);

	// 闪避（A键）
	InputComponent->BindAction("Dodge", IE_Pressed, this, &ASL_PlayerControllerBase::OnDodgePressed);

	// ===== 道具使用 =====
	InputComponent->BindAction("UseItem", IE_Pressed, this, &ASL_PlayerControllerBase::OnUseItemPressed);
	InputComponent->BindAction("SelectPrevItem", IE_Pressed, this, &ASL_PlayerControllerBase::OnSelectPrevItemPressed);
	InputComponent->BindAction("SelectNextItem", IE_Pressed, this, &ASL_PlayerControllerBase::OnSelectNextItemPressed);

	// ===== 交互 =====
	InputComponent->BindAction("Interact", IE_Pressed, this, &ASL_PlayerControllerBase::OnInteractPressed);

}

/************************************************************************/
/************************************************************************/
/* 输入回调                                                                     */
/************************************************************************/

void ASL_PlayerControllerBase::OnLockOnPressed()
{
	ASL_CharacterBase* MyCharacter = GetMyPlayerCharacter();
	if (!MyCharacter) return;

	USL_LockOnComponent* LockOnCmp = MyCharacter->GetLockOnComponentRef();
	if (LockOnCmp)
	{
		LockOnCmp->TryLockOnTarget();
	}
}

void ASL_PlayerControllerBase::OnOpenPauseMenuPressed()
{
	UHUD_PauseMenuScreen::OpenPauseMenu(this);
}

/*                               外部调用                               */
/************************************************************************/

// 设置纯UI输入模式（鼠标控制UI）
void ASL_PlayerControllerBase::SetInputModeUIOnly(UWidget* InWidgetToFocus)
{
	FInputModeUIOnly InputMode;
	if (InWidgetToFocus)
	{
		InputMode.SetWidgetToFocus(InWidgetToFocus->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);

	SetInputMode(InputMode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

// 设置纯游戏输入模式（隐藏鼠标）
void ASL_PlayerControllerBase::SetInputModeGameOnly()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

// 设置游戏+UI混合输入模式
void ASL_PlayerControllerBase::SetInputModeGameAndUI(UWidget* InWidgetToFocus, bool bLockMouseToViewport)
{
	FInputModeGameAndUI InputMode;
	if (InWidgetToFocus)
	{
		InputMode.SetWidgetToFocus(InWidgetToFocus->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(bLockMouseToViewport ? EMouseLockMode::LockOnCapture : EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

// 获取当前控制的角色
ASL_CharacterBase* ASL_PlayerControllerBase::GetMyPlayerCharacter() const
{
	return Cast<ASL_CharacterBase>(GetPawn());
}

void ASL_PlayerControllerBase::ShowBeginPlayScreen()
{
    UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("SL_PlayerControllerBase::ShowBeginPlayScreen - UIManagerSubsystem not found!"));
        return;
    }

    // 通过UIManager打开界面（ZOrder=100确保在最上层）
    UIManager->OpenScreenWidget(EWidgetType::EWIDGET_BeginPlayScreen, 100);

    // 检查存档状态并初始化按钮
    if (UUserWidget* Widget = UIManager->GetWidget(EWidgetType::EWIDGET_BeginPlayScreen))
    {
        if (UHUD_BeginPlayScreen* BeginPlay = Cast<UHUD_BeginPlayScreen>(Widget))
        {
            bool bHasSaveData = false;
            if (ASL_GameModeBase* GameMode = Cast<ASL_GameModeBase>(GetWorld()->GetAuthGameMode()))
            {
                bHasSaveData = GameMode->HasSaveData();
            }
            BeginPlay->InitializeScreen(bHasSaveData);

            UE_LOG(LogTemp, Log, TEXT("SL_PlayerControllerBase::ShowBeginPlayScreen - BeginPlayScreen displayed, HasSaveData: %d"), bHasSaveData);
        }
    }
}

void ASL_PlayerControllerBase::HideBeginPlayScreen()
{
    if (UIManager)
    {
        // 通过UIManager关闭界面（Widget内部NativeDestruct自动处理输入模式恢复）
        UIManager->CloseScreenWidget(EWidgetType::EWIDGET_BeginPlayScreen);

        UE_LOG(LogTemp, Log, TEXT("SL_PlayerControllerBase::HideBeginPlayScreen - BeginPlayScreen hidden"));
    }
}

bool ASL_PlayerControllerBase::IsBeginPlayScreenVisible() const
{
    return UIManager && UIManager->IsWidgetOpen(EWidgetType::EWIDGET_BeginPlayScreen);
}

/************************************************************************/
/*                               界面通用UI-玩家状态UI                   */
/************************************************************************/

// 创建玩家状态UI
void ASL_PlayerControllerBase::CreatePlayerStatusUI()
{
	// 检测是否持有有效的UI管理子系统,如无则获取,若获取不到则返回
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager == nullptr)
	{
		UE_LOG(LogTemp, Verbose, TEXT("CreatePlayerStatusUI :Can not Get Valid UUIManagerSubsystem Refence"));
		return;
	}
	// 创建参数
	FUICreateParams createParam;
	createParam.Type = EWidgetType::EWIDGET_PlayerStatus;
	// 创建UI实例
	UIManager->OpenWidget(createParam);
	// 获取玩家属性进行初始化
	UUserWidget* playerStatus = UIManager->GetWidget(EWidgetType::EWIDGET_PlayerStatus);
	if (playerStatus != nullptr)
	{
		//playerStatus->
	}
}

// 销毁血量UI
void ASL_PlayerControllerBase::DestroyPlayerStatusUI()
{
	// 检测是否持有有效的UI管理子系统,如无则获取,若获取不到则返回
	if (UIManager == nullptr && (UIManager = UUIManagerSubsystem::Get(this)) == nullptr)
	{
		UE_LOG(LogTemp, Verbose, TEXT("DestroyPlayerStatusUI :Can not Get Valid UUIManagerSubsystem Refence"));
		return;
	}
	UIManager->CloseScreenWidget(EWidgetType::EWIDGET_PlayerStatus);
}

/************************************************************************/
/*                               输入处理                               */
/************************************************************************/

void ASL_PlayerControllerBase::OnLightAttackPressed()
{
	ProcessComboInput(EComboInputActionType::EComboInputAction_Light);
}

void ASL_PlayerControllerBase::OnLightAttackReleased()
{

}

void ASL_PlayerControllerBase::OnHeavyAttackPressed()
{
	ProcessComboInput(EComboInputActionType::EComboInputAction_Height);
}

void ASL_PlayerControllerBase::OnHeavyAttackReleased()
{
	USL_ComboManagerComponent* ComboMgr = GetComboManagerComponent();
	if (ComboMgr && ComboMgr->IsCharging())
	{
		ComboMgr->ReleaseCharge();
	}
}

void ASL_PlayerControllerBase::OnSpecialAttackPressed()
{
	ProcessComboInput(EComboInputActionType::EComboInputAction_Special);
}

void ASL_PlayerControllerBase::OnDodgePressed()
{

}

// ===== 道具使用处理 =====
void ASL_PlayerControllerBase::OnUseItemPressed()
{
	// 1. 从 UI 获取当前选中的道具 ID
	UHUD_ItemUseUI* ItemUI = GetItemUseUIWidget();
	if (!ItemUI)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ASL_PlayerControllerBase::OnUseItemPressed - ItemUseUI widget not found"));
		return;
	}

	FName SelectedItemID = ItemUI->GetCurrentItemID();
	if (SelectedItemID.IsNone())
	{
		UE_LOG(LogTemp, Verbose, TEXT("ASL_PlayerControllerBase::OnUseItemPressed - No item selected in UI"));
		return;
	}

	// 2. 获取背包组件
	USL_InventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ASL_PlayerControllerBase::OnUseItemPressed - InventoryComponent not found"));
		return;
	}

	// 3. 客户端做轻量检查（快速过滤）
	if (!Inventory->CanUseItem(SelectedItemID))
	{
		UE_LOG(LogTemp, Verbose, TEXT("ASL_PlayerControllerBase::OnUseItemPressed - Item %s cannot be used now"),
			*SelectedItemID.ToString());
		return;
	}

	// 4. 发 RPC 到服务器再做权威检查并执行
	Server_UseItem(SelectedItemID);

	UE_LOG(LogTemp, Log, TEXT("ASL_PlayerControllerBase::OnUseItemPressed - Item use requested: %s"), *SelectedItemID.ToString());
}

bool ASL_PlayerControllerBase::Server_UseItem_Validate(FName InItemID)
{
	return !InItemID.IsNone();
}

void ASL_PlayerControllerBase::Server_UseItem_Implementation(FName InItemID)
{
	USL_InventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory || !Inventory->CanUseItem(InItemID)) return;

	Inventory->SetSelectedItemID(InItemID);
	Inventory->UseItemByID(InItemID);

	UE_LOG(LogTemp, Log, TEXT("ASL_PlayerControllerBase::Server_UseItem - Server executed item use: %s"), *InItemID.ToString());
}

void ASL_PlayerControllerBase::OnSelectPrevItemPressed()
{
	UHUD_ItemUseUI* ItemUI = GetItemUseUIWidget();
	if (ItemUI)
	{
		ItemUI->SelectPrevious();
	}
}

void ASL_PlayerControllerBase::OnSelectNextItemPressed()
{
	UHUD_ItemUseUI* ItemUI = GetItemUseUIWidget();
	if (ItemUI)
	{
		ItemUI->SelectNext();
	}
}

// ===== 交互处理 =====
void ASL_PlayerControllerBase::OnInteractPressed()
{
	if (!SummonSessionCmp) return;

	ACharacter* MyCharacter = GetCharacter();
	if (!MyCharacter) return;

	// 在当前世界中查找所有 SummonSign，找最近的、与玩家重叠且未锁定的标记
	TArray<AActor*> FoundSigns;
	UGameplayStatics::GetAllActorsOfClass(this, ASL_SummonSign::StaticClass(), FoundSigns);

	ASL_SummonSign* BestSign = nullptr;
	float BestDist = FLT_MAX;

	for (AActor* Actor : FoundSigns)
	{
		ASL_SummonSign* Sign = Cast<ASL_SummonSign>(Actor);
		if (!Sign || Sign->bIsLocked || !Sign->IsOverlappedBy(MyCharacter))
		{
			continue;
		}

		float Dist = FVector::DistSquared(MyCharacter->GetActorLocation(), Sign->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestSign = Sign;
		}
	}

	if (BestSign)
	{
		SummonSessionCmp->InteractWithSign(BestSign);
	}
}

static const float SUMMON_SIGN_INTERACT_RANGE = 120.0f;

void ASL_PlayerControllerBase::Client_CheckNearbySummonSigns()
{
	if (!IsLocalController()) return;

	ACharacter* MyChar = GetCharacter();
	if (!MyChar) return;

	TArray<AActor*> FoundSigns;
	UGameplayStatics::GetAllActorsOfClass(this, ASL_SummonSign::StaticClass(), FoundSigns);

	ASL_SummonSign* BestSign = nullptr;
	float BestDist = FLT_MAX;

	for (AActor* Actor : FoundSigns)
	{
		ASL_SummonSign* Sign = Cast<ASL_SummonSign>(Actor);
		if (!Sign || Sign->bIsLocked || Sign->bIsRemoteSign == false) continue;

		float Dist = FVector::Dist2D(MyChar->GetActorLocation(), Sign->GetActorLocation());
		if (Dist < SUMMON_SIGN_INTERACT_RANGE && Dist < BestDist)
		{
			BestDist = Dist;
			BestSign = Sign;
		}
	}

	if (BestSign != CurrentSummonSignTarget)
	{
		CurrentSummonSignTarget = BestSign;
		UpdateSummonSignPrompt(BestSign);
	}
}

void ASL_PlayerControllerBase::UpdateSummonSignPrompt(ASL_SummonSign* InSign)
{
	UUIManagerSubsystem* UIMgr = UUIManagerSubsystem::Get(this);
	if (!UIMgr) return;

	if (InSign)
	{	
		// 检查是否已经打开了该界面
		if (UIMgr->IsWidgetOpen(EWidgetType::EWIDGET_InterActPanel) == false)
		{
			UIMgr->OpenScreenWidget(EWidgetType::EWIDGET_InterActPanel);

		}
		
		if (UUserWidget* Widget = UIMgr->GetWidget(EWidgetType::EWIDGET_InterActPanel))
		{
			if (UHUD_InterActBtnPanel* Panel = Cast<UHUD_InterActBtnPanel>(Widget))
			{
				FString Prompt = FString::Printf(TEXT("召唤 %s [G]"),
					*InSign->GetSignInfo().OwnerPlayerName);
				TArray<FInterActOptionInfo> Options;
				Options.Add(FInterActOptionInfo(0, nullptr, Prompt));
				Panel->UpdateBatch(Options);
				Panel->SetVisible(true);
			}
		}
	}
	else
	{
		UUserWidget* Widget = UIMgr->GetWidget(EWidgetType::EWIDGET_InterActPanel);
		if (UHUD_InterActBtnPanel* Panel = Cast<UHUD_InterActBtnPanel>(Widget))
			Panel->SetVisible(false);
	}
}

/************************************************************************/
/*                               组件获取                               */
/************************************************************************/

USL_ComboManagerComponent* ASL_PlayerControllerBase::GetComboManagerComponent() const
{
	// 使用缓存
	if (CachedComboManager.IsValid())
	{
		return CachedComboManager.Get();
	}

	// 从当前控制的 Pawn 上查找
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		CachedComboManager = ControlledPawn->FindComponentByClass<USL_ComboManagerComponent>();
	}

	return CachedComboManager.Get();
}

USL_EquipmentComponent* ASL_PlayerControllerBase::GetEquipmentComponent() const
{
	// 使用缓存
	if (CachedEquipmentComp.IsValid())
	{
		return CachedEquipmentComp.Get();
	}

	// 从当前控制的 Pawn 上查找
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		CachedEquipmentComp = ControlledPawn->FindComponentByClass<USL_EquipmentComponent>();
	}

	return CachedEquipmentComp.Get();
}

USL_InventoryComponent* ASL_PlayerControllerBase::GetInventoryComponent() const
{
	// 使用缓存
	if (CachedInventoryComp.IsValid())
	{
		return CachedInventoryComp.Get();
	}

	// 从当前控制的 Pawn 上查找
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		CachedInventoryComp = ControlledPawn->FindComponentByClass<USL_InventoryComponent>();
	}

	return CachedInventoryComp.Get();
}

UHUD_ItemUseUI* ASL_PlayerControllerBase::GetItemUseUIWidget() const
{
	UUIManagerSubsystem* UIMgr = UUIManagerSubsystem::Get(const_cast<ASL_PlayerControllerBase*>(this));
	if (!UIMgr)
	{
		return nullptr;
	}

	return Cast<UHUD_ItemUseUI>(UIMgr->GetWidget(EWidgetType::EWIDGET_ItemUseUI));
}

void ASL_PlayerControllerBase::ProcessComboInput(EComboInputActionType InputType)
{
	USL_ComboManagerComponent* ComboMgr = GetComboManagerComponent();
	if (ComboMgr)
	{
		ComboMgr->HandleInputPressed(InputType);
	}
}

void ASL_PlayerControllerBase::CheckSaveDataAndInitScreen()
{
    // 实际逻辑已在 ShowBeginPlayScreen 中实现
    // 这里保留接口方便后续扩展
}

/************************************************************************/
/*                               死亡事件                               */
/************************************************************************/

// 内部调用：绑定角色死亡事件监听
void ASL_PlayerControllerBase::BindPlayerDeathEvent()
{
    UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this);
    if (DelegateMgr)
    {
        // 防止重复绑定
        if (!OnPlayerDiedHandle.IsValid())
        {
            OnPlayerDiedHandle = DelegateMgr->OnCharacterDied.AddUObject(
                this, &ASL_PlayerControllerBase::OnPlayerDiedHandler);
            UE_LOG(LogTemp, Log, TEXT("PlayerController: Bound to OnCharacterDied event"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController: Failed to get GlobalDelegatesManager"));
    }
}

// 内部调用：角色死亡事件响应
void ASL_PlayerControllerBase::OnPlayerDiedHandler(AActor* InDeadActor, AActor* InInstigator)
{
    // 只处理自己控制的角色死亡
    if (InDeadActor != GetPawn()) return;

    UE_LOG(LogTemp, Log, TEXT("PlayerController: Player character died - closing UI"));

    // 1. 关闭所有 UI 界面
    if (UIManager)
    {
        UIManager->CloseAllWidgets();
    }

    // 2. 禁用玩家输入（UI Only 模式）
    SetInputModeUIOnly(nullptr);

    // 3. 通知 LevelManager 处理死亡逻辑（暂停波次、播放死亡特效）
    ALevelManager* LevelMgr = Cast<ALevelManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ALevelManager::StaticClass()));
    if (LevelMgr)
    {
        LevelMgr->OnPlayerDied();
    }

    // 4. 延迟显示死亡界面（给布娃娃动画播放时间）
    FTimerHandle DeathScreenDelayHandle;
    GetWorld()->GetTimerManager().SetTimer(
        DeathScreenDelayHandle,
        FTimerDelegate::CreateLambda([this]()
        {
            UE_LOG(LogTemp, Log, TEXT("PlayerController: Showing death screen after delay"));
            if (UIManager && UIManager->IsWidgetOpen(EWidgetType::EWIDGET_DeathScreen))
            {
                // 死亡界面已显示，跳过
                return;
            }
            UIManager->OpenScreenWidget(EWidgetType::EWIDGET_DeathScreen, 200);
        }),
        2.5f,  // 延迟 2.5 秒
        false  // 只执行一次
    );
}

/************************************************************************/
/*                         联机RPC：游戏状态操作                         */
/************************************************************************/

void ASL_PlayerControllerBase::RequestNewGame()
{
	if (HasAuthority())
	{
		if (ASL_GameModeBase* GameMode = Cast<ASL_GameModeBase>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->SetUseSaveData(false);
		}
	}
	else
	{
		Server_RequestNewGame();
	}
}

bool ASL_PlayerControllerBase::Server_RequestNewGame_Validate()
{
	return true;
}

void ASL_PlayerControllerBase::Server_RequestNewGame_Implementation()
{
	if (ASL_GameModeBase* GameMode = Cast<ASL_GameModeBase>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->SetUseSaveData(false);
	}
}

void ASL_PlayerControllerBase::RequestLoadGame()
{
	if (HasAuthority())
	{
		if (ASL_GameModeBase* GameMode = Cast<ASL_GameModeBase>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->SetUseSaveData(true);
		}
	}
	else
	{
		Server_RequestLoadGame();
	}
}

bool ASL_PlayerControllerBase::Server_RequestLoadGame_Validate()
{
	return true;
}

void ASL_PlayerControllerBase::Server_RequestLoadGame_Implementation()
{
	if (ASL_GameModeBase* GameMode = Cast<ASL_GameModeBase>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->SetUseSaveData(true);
	}
}

void ASL_PlayerControllerBase::RequestLoadLevel(int32 InLevelID)
{
	if (HasAuthority())
	{
		if (ASL_GameModeBase* GameMode = Cast<ASL_GameModeBase>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->StartTargetLevel(InLevelID);
		}
	}
	else
	{
		Server_RequestLoadLevel(InLevelID);
	}
}

bool ASL_PlayerControllerBase::Server_RequestLoadLevel_Validate(int32 InLevelID)
{
	return InLevelID >= 0;
}

void ASL_PlayerControllerBase::Server_RequestLoadLevel_Implementation(int32 InLevelID)
{
	if (ASL_GameModeBase* GameMode = Cast<ASL_GameModeBase>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->StartTargetLevel(InLevelID);
	}
}