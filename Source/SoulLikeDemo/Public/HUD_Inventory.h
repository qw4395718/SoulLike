#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "HUD_Inventory.generated.h"

class UTileView;
class UScrollBox;
class UTextBlock;
class UButton;
class USL_InventoryComponent;
class UUI_InventorySlot;

/**
 * 主背包界面
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_Inventory : public UUserWidget
{
    GENERATED_BODY()

public:
    // 设置库存组件引用
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetInventoryComponent(USL_InventoryComponent* NewInventoryComponent);

    // 刷新背包显示
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RefreshInventory();

    // 打开/关闭背包
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ToggleInventory();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    // 绑定UI元素
    UPROPERTY(meta = (BindWidget))
    UTileView* InventoryTileView;

    UPROPERTY(meta = (BindWidget))
    UScrollBox* InventoryScrollBox;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* WeightText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SlotCountText;

    UPROPERTY(meta = (BindWidget))
    UButton* SortButton;

    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;

    // 背包格子Widget类
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    TSubclassOf<UUI_InventorySlot> SlotWidgetClass;

private:
    // 库存组件引用
    UPROPERTY()
    USL_InventoryComponent* InventoryComponent;

    // 当前选中的格子
    UPROPERTY()
    UUI_InventorySlot* SelectedSlot;

    // 是否正在拖拽
    bool bIsDragging;

    // 拖拽开始位置
    FVector2D DragStartPosition;

    // 初始化UI
    void InitializeUI();

    // 绑定事件
    void BindEvents();

    // 更新背包信息
    void UpdateInventoryInfo();

    // UFUNCTION 事件处理
    UFUNCTION()
    void OnInventoryUpdated();

    UFUNCTION()
    void OnSlotClicked(UUI_InventorySlot* ClickedSlot);

    UFUNCTION()
    void OnSlotHovered(UUI_InventorySlot* HoveredSlot);

    UFUNCTION()
    void OnSortButtonClicked();

    UFUNCTION()
    void OnCloseButtonClicked();

    // 拖拽功能
    void StartDrag(UUI_InventorySlot* Slot);
    void StopDrag();
    bool TryDropItem(int32 TargetSlotIndex);
};