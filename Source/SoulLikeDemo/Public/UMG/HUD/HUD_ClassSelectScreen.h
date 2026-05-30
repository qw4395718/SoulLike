#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD_ClassSelectScreen.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;
class UVerticalBox;
class UImage;
class UUI_ListItemBase;

/**
 * 职业选择界面Widget
 *
 * 布局：
 *   右上角 — 关闭按钮
 *   左侧   — ScrollBox 内垂直排列职业选择按钮
 *   右侧   — 预览图片 + 职业名 + 职业描述 + 属性概览
 *   下方   — 确认切换按钮
 *
 * 流程：
 *   点击左侧职业按钮 → 高亮切换 → 右侧同步更新预览内容
 *   点击确认切换 → 存档保存新职业ID → 通知LobbyScreen刷新 → 关闭
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class SOULLIKEDEMO_API UHUD_ClassSelectScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UHUD_ClassSelectScreen(const FObjectInitializer& ObjectInitializer);

	/************************************************************************/
	/* 继承实现                                                                     */
	/************************************************************************/
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 初始化职业选择界面：加载配置 → 构建列表 → 默认选中当前职业
	UFUNCTION(BlueprintCallable, Category = "ClassSelectScreen")
	void InitializeSelectScreen();

	// 供外部（LobbyScreen）绑定，职业切换后的通知回调
	FSimpleDelegate OnClassChanged;

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	// 从 ClassConfigInfoTable 加载所有职业，构建左侧按钮列表
	void BuildClassList();

	// 为指定职业ID创建单个选择按钮
	UUI_ListItemBase* CreateClassButton(int32 InClassID, const FText& InDisplayName);

	// 更新右侧预览区域：图片、职业名、描述、属性
	void UpdatePreview(int32 InClassID);

	// 关闭界面
	void CloseScreen();

	// 按钮点击回调
	UFUNCTION()
	void OnClassButtonClicked(int32 InClassID);

	UFUNCTION()
	void OnConfirmClicked();

	UFUNCTION()
	void OnCloseClicked();

protected:
	/************************************************************************/
	/* 内部变量 - BindWidget控件绑定                                                */
	/************************************************************************/

	// 右上角 - 关闭按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ClassSelectScreen")
		UButton* m_closeButton;

	// 左侧 - 职业列表滚动容器
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ClassSelectScreen")
		UVerticalBox* m_classListContainer;

	// 右侧 - 职业预览图片
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "ClassSelectScreen")
		UImage* m_previewImage;

	// 右侧 - 职业名称
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ClassSelectScreen")
		UTextBlock* m_classNameText;

	// 右侧 - 职业描述
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ClassSelectScreen")
		UTextBlock* m_classDescText;

	// 右侧 - 属性概览
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ClassSelectScreen")
		UTextBlock* m_attrText;

	// 下方 - 确认切换按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ClassSelectScreen")
		UButton* m_confirmButton;

	// 按钮模板类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassSelectScreen")
		TSubclassOf<UUI_ListItemBase> m_ClassSelectItemWidgetClass;

protected:
	/************************************************************************/
	/* 内部变量 - 数据                                                             */
	/************************************************************************/

	UPROPERTY()
		class UUIManagerSubsystem* UIManager;

	// 当前玩家职业ID（从存档读取）
	int32 m_playerClassID;

	// 当前选中的职业ID（点击确认时写入存档）
	int32 m_selectedClassID;

	// 已创建的职业按钮列表
	UPROPERTY()
		TArray<UUI_ListItemBase*> m_classButtons;

	// 按钮 → 职业ID 映射
	UPROPERTY()
		TMap<int32 ,UUI_ListItemBase*> m_buttonClassMap;

	// 上一个选中的按钮（用于清除高亮）
	UPROPERTY()
		UUI_ListItemBase* m_lastSelectedButton;
};
