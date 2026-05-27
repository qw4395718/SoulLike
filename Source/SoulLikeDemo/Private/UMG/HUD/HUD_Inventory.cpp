#include "HUD_Inventory.h"
#include "SL_InventoryComponent.h"
#include "Components/TileView.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI_InventorySlot.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include <UIManagerSubsystem.h>
#include <SoulLikeGameGlobal.h>

void UHUD_Inventory::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

void UHUD_Inventory::NativeConstruct()
{
    Super::NativeConstruct();

    SelectedSlot = nullptr;
    bIsDragging = false;

    InitializeUI();
    BindEvents();

    // 默认隐藏，等待打开
    SetVisibility(ESlateVisibility::Collapsed);
}

void UHUD_Inventory::NativeDestruct()
{
	// 通知 UIManager 关闭（更新 ActiveWidgets 状态）
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->CloseScreenWidget(EWidgetType::EWIDGET_MainMenu);
	}

    Super::NativeDestruct();
}

FReply UHUD_Inventory::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // 按I键打开/关闭背包
    if (InKeyEvent.GetKey() == EKeys::I)
    {
        ToggleInventory();
        return FReply::Handled();
    }
    // 按ESC关闭背包
    else if (InKeyEvent.GetKey() == EKeys::Escape && GetVisibility() == ESlateVisibility::Visible)
    {
        SetVisibility(ESlateVisibility::Collapsed);
        
        // 恢复游戏输入
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;
        }
        
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UHUD_Inventory::SetInventoryComponent(USL_InventoryComponent* NewInventoryComponent)
{
	//if (InventoryComponent && InventoryComponent->OnInventoryUpdated.IsAlreadyBound(this, &UHUD_Inventory::OnInventoryUpdated))
	//{
	//	InventoryComponent->OnInventoryUpdated.RemoveDynamic(this, &UHUD_Inventory::OnInventoryUpdated);
	//}

	//InventoryComponent = NewInventoryComponent;

	//if (InventoryComponent)
	//{
	//	InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UHUD_Inventory::OnInventoryUpdated);
	//	RefreshInventory();
	//}
}

void UHUD_Inventory::RefreshInventory()
{
    //if (!InventoryComponent || !InventoryTileView) return;

    //// 清空当前显示
    //InventoryTileView->ClearListItems();

    //// 获取所有物品
    //TArray<UItemData*> Items = InventoryComponent->GetInventoryItems();

    //// 添加到Tile View
    //for (UItemData* Item : Items)
    //{
    //    InventoryTileView->AddItem(Item);
    //}

    //// 更新背包信息
    //UpdateInventoryInfo();
}

void UHUD_Inventory::ToggleInventory()
{
    if (GetVisibility() == ESlateVisibility::Visible)
    {
        // 关闭背包
        SetVisibility(ESlateVisibility::Collapsed);
        
        // 恢复游戏输入
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;
        }
    }
    else
    {
        // 打开背包
        SetVisibility(ESlateVisibility::Visible);
        
        // 刷新显示
        RefreshInventory();
        
        // 设置UI输入模式
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            FInputModeGameAndUI InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            InputMode.SetHideCursorDuringCapture(false);
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;
        }
    }
}

void UHUD_Inventory::InitializeUI()
{
    if (!SlotWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SlotWidgetClass is not set in WBP_Inventory!"));
        return;
    }

    // 配置Tile View
    if (InventoryTileView)
    {
        InventoryTileView->SetEntryWidth(100.0f);
        InventoryTileView->SetEntryHeight(100.0f);
        InventoryTileView->SetSelectionMode(ESelectionMode::Single);
       // InventoryTileView->SetItemAlignment(EListItemAlignment::LeftAligned);
       // InventoryTileView->SetOrientation(Orient_Vertical);
        InventoryTileView->SetScrollbarVisibility(ESlateVisibility::Visible);
       // InventoryTileView->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
        
        // 设置Widget生成类
        //InventoryTileView->SetEntryWidgetClass(SlotWidgetClass);
    }
}

void UHUD_Inventory::BindEvents()
{
    if (SortButton)
    {
        SortButton->OnClicked.AddDynamic(this, &UHUD_Inventory::OnSortButtonClicked);
    }

    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UHUD_Inventory::OnCloseButtonClicked);
    }
}

void UHUD_Inventory::UpdateInventoryInfo()
{
 /*    if (!InventoryComponent || !WeightText || !SlotCountText) return;

    // 更新负重信息
    float TotalWeight = InventoryComponent->GetTotalWeight();
    WeightText->SetText(FText::FromString(FString::Printf(TEXT("Weight: %.1f"), TotalWeight)));

    // 更新格子使用情况
   int32 UsedSlots = InventoryComponent->MaxInventorySlots - InventoryComponent->GetEmptySlotCount();
	SlotCountText->SetText(FText::FromString(FString::Printf(TEXT("Slots: %d/%d"),
		UsedSlots, InventoryComponent->MaxInventorySlots)));*/
}

void UHUD_Inventory::OnInventoryUpdated()
{
    RefreshInventory();
}

void UHUD_Inventory::OnSlotClicked(UUI_InventorySlot* ClickedSlot)
{
    if (!ClickedSlot || !InventoryComponent) return;

    // 清除之前选中的格子
    if (SelectedSlot && SelectedSlot != ClickedSlot)
    {
        //SelectedSlot->SetSelected(false);
    }

    // 设置新的选中格子
    SelectedSlot = ClickedSlot;
   // SelectedSlot->SetSelected(true);

    //UItemData* ItemData = ClickedSlot->GetItemData();
    //if (ItemData)
    //{
    //    // 如果有物品，尝试使用
    //    if (ItemData->bCanUse)
    //    {
    //        InventoryComponent->UseItem(ItemData);
    //    }
    //}

    //// 拖拽逻辑
    //if (ItemData && !bIsDragging)
    //{
    //    StartDrag(ClickedSlot);
    //}
}

void UHUD_Inventory::OnSlotHovered(UUI_InventorySlot* HoveredSlot)
{
    if (!HoveredSlot || !InventoryComponent) return;

    //UItemData* ItemData = HoveredSlot->GetItemData();
    //if (ItemData)
    //{
    //    // 这里可以显示物品提示框
    //    // 例如：ShowTooltip(ItemData);
    //}
}

void UHUD_Inventory::OnSortButtonClicked()
{
    //if (InventoryComponent)
    //{
    //    InventoryComponent->SortInventory();
    //}
}

void UHUD_Inventory::OnCloseButtonClicked()
{
    ToggleInventory();
}

void UHUD_Inventory::StartDrag(UUI_InventorySlot* DragSlot)
{
    if (!DragSlot || bIsDragging) return;

    bIsDragging = true;
    SelectedSlot = DragSlot;
    DragStartPosition = FVector2D::ZeroVector;

    // 这里可以开始拖拽视觉效果
}

void UHUD_Inventory::StopDrag()
{
    bIsDragging = false;
    SelectedSlot = nullptr;
    // 清理拖拽视觉效果
}

//bool UHUD_Inventory::TryDropItem(int32 TargetSlotIndex)
//{
//    if (!SelectedSlot || !InventoryComponent) return false;
//
//    int32 SourceIndex = SelectedSlot->GetSlotIndex();
//    if (SourceIndex == TargetSlotIndex) return false;
//
//    // 交换物品
//    bool bSuccess = InventoryComponent->SwapItems(SourceIndex, TargetSlotIndex);
//    
//    StopDrag();
//    return bSuccess;
//}