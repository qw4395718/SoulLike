// UIManagerSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulLikeGameGlobal.h"
#include "Manager/UI/SL_UILayerDefine.h"
#include "Manager/UI/SL_UINavigationTypes.h"
#include "UIManagerSubsystem.generated.h"

// UI创建参数基结构（可根据需求扩展）
USTRUCT(BlueprintType)
struct FUICreateParams
{
	GENERATED_BODY()
	/************************************************************************/
	/*                                基础信息                                      */
	/************************************************************************/
	// UI类型
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		EWidgetType Type;

	// 用于获取世界
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UObject* WorldContextObject = nullptr;

	// Z-Order（屏幕空间UI的层叠顺序，越大越靠前）
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 ZOrder = 0;

	/************************************************************************/
	/*                                视口UI所需信息                                      */
	/************************************************************************/


	/************************************************************************/
	/*                                世界UI所需信息                                      */
	/************************************************************************/
	// 要跟随的玩家角色
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		AActor* TargetActor = nullptr;

	// 绑定在玩家哪个部位
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FName TargetBodyTag = FName("HeadUI");
	// 绑定部位的偏移
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FVector WorldOffset = FVector(0, 0, 150);

	// 动态销毁参数-是否可开启距离检测
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool bEnableDistanceCulling = true;

	// 销毁距离
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float DestroyDistance = 3000.0f;

		// 构造函数方便快速创建
	FUICreateParams() {}

	static FUICreateParams ForScreen(EWidgetType InType, UObject* WorldContext)
	{
		FUICreateParams Params;
		Params.Type = InType;
		Params.WorldContextObject = WorldContext;
		return Params;
	}

	static FUICreateParams ForWorldActor(EWidgetType InType, AActor* InTargetActor, float InDestroyDistance = 3000.0f)
	{
		FUICreateParams Params;
		Params.Type = InType;
		Params.TargetActor = InTargetActor;
		Params.WorldContextObject = InTargetActor;
		Params.bEnableDistanceCulling = true;
		Params.DestroyDistance = InDestroyDistance;
		// 可以在这里根据Type自动设置偏移，比如血量条默认在头顶
		return Params;
	}
};

UCLASS(BlueprintType, Blueprintable)
class SOULLIKEDEMO_API UUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/************************************************************************/
	/* 平台识别                                                                     */
	/************************************************************************/
	// 获取当前运行平台
	UFUNCTION(BlueprintPure, Category = "UI Manager")
	EPlatformType GetPlatform() const;

	// 获取单例实例
	UFUNCTION(BlueprintPure, Category = "UIManagerSubsystem", meta = (WorldContext = "WorldContextObject"))
		static UUIManagerSubsystem* Get(const UObject* WorldContextObject);

    // 根据路径加载BP资源
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void RegisterWidgetFromBPPath(EWidgetType WidgetType, const FString& WidgetClassPathstr);

	// 注册指定平台的 Widget（路径格式同 RegisterWidgetFromBPPath）
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void RegisterPlatformWidgetFromBPPath(EWidgetType WidgetType, const FString& WidgetClassPathstr, EPlatformType InPlatform);

	// 根据平台获取已注册的 Widget 类
	TSubclassOf<UUserWidget> GetWidgetClassForPlatform(EWidgetType WidgetType) const;

    /************************************************************************/
    /*                                通用方法                                      */
    /************************************************************************/
	// 注册/注销界面,蓝图调用
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void RegisterWidget(EWidgetType WidgetType,TSubclassOf<UUserWidget> WidgetClass);

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void UnregisterWidget(EWidgetType WidgetType);

	// 打开/关闭界面
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void OpenWidget(const FUICreateParams& CreateParam);

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void OpenScreenWidget(EWidgetType WidgetType, int32 ZOrder = 0);

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void OpenWorldWidgetWithActor(const FUICreateParams& CreateParam);

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void CloseWidget(const FUICreateParams& CreateParam);

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void CloseScreenWidget(EWidgetType WidgetType);

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void CloseWorldWidgetWithActor(const FUICreateParams& CreateParam);

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void CloseAllWidgets();

    // 设置界面可视性
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void SetWidgetVisible(EWidgetType WidgetType);

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void ToggleWidgetVisible(EWidgetType WidgetType);

	// 获取界面
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    UUserWidget* GetWidget(EWidgetType WidgetType) const;

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    bool IsWidgetOpen(EWidgetType WidgetType) const;

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	bool IsWorldWidget(EWidgetType WidgetType) const;

    // 页面栈管理（旧接口，委托给导航系统）
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void PushWidget(EWidgetType WidgetType);

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void PopWidget(EWidgetType WidgetType);

    // 焦点管理
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void SetFocusToWidget(EWidgetType WidgetType);

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    FName GetFocusedWidgetName() const;

    /************************************************************************/
    /*                                导航系统（新模式）                               */
    /************************************************************************/
	// 语义化导航：打开Widget + 自动计算ZOrder + 入导航栈 + 触发生命周期
	// InMode: Push(压栈), ReplaceTop(替换栈顶), ClearStack(清空栈后压入)
	UFUNCTION(BlueprintCallable, Category = "UI|Navigation")
	void NavigateTo(EWidgetType WidgetType,
		const FUINavigationPayload& InPayload,
		EUINavigationMode InMode = EUINavigationMode::Push);

	// 返回上一页 / 上N页
	UFUNCTION(BlueprintCallable, Category = "UI|Navigation")
	void NavigateBack(int32 InStep = 1);

	// 导航栈查询
	UFUNCTION(BlueprintPure, Category = "UI|Navigation")
	int32 GetNavigationStackSize() const;

	UFUNCTION(BlueprintPure, Category = "UI|Navigation")
	EWidgetType GetTopNavigationWidgetType() const;

    /************************************************************************/
    /*                                血量系统相关接口                                      */
    /************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "UI_Manager_Health")
    void UpdateHealthUI(float OldHealth);

private:
    // 存储所有注册的界面-蓝图调用
    UPROPERTY()
    TMap<EWidgetType, TSubclassOf<UUserWidget>> RegisteredWidgets;

    // 移动端专用 Widget
    UPROPERTY()
    TMap<EWidgetType, TSubclassOf<UUserWidget>> MobileWidgets;

    UPROPERTY()
    TMap<EWidgetType, TSubclassOf<UUserWidget>> ConsoleWidgets;

    // 屏幕空间-活动界面（当前打开的）
    UPROPERTY()
    TMap<EWidgetType, UUserWidget*> ActiveWidgets;

    // 世界空间-活动界面(FString key = ActorName + EWidgetType)
	UPROPERTY()
	TMap<FString, UUserWidget*> ActiveWorldWidgets;

    // 当前焦点界面
    UPROPERTY()
    EWidgetType FocusedWidgetName;

	/************************************************************************/
	/* InputMode自动管理（新模式）                                                    */
	/************************************************************************/
	// 保存的InputMode状态快照
	struct FUISavedInputState
	{
		uint8 bShowMouseCursor : 1;
		uint8 bEnableClickEvents : 1;
		uint8 bEnableMouseOverEvents : 1;
		uint8 ModeType : 2; // 0=GameOnly 1=GameAndUI 2=UIOnly
	};

	// InputMode栈（嵌套UI正确恢复）
	TArray<FUISavedInputState> InputModeStack;

	// 导航栈（替代旧的 WidgetStack）
	TArray<FUINavigationEntry> NavigationStack;

	// 内部辅助方法
	APlayerController* GetPlayerController() const;
	void ApplyInputModeForWidget(UUserWidget* InWidget, EWidgetType InWidgetType);
	void RestorePreviousInputMode();
	int32 CalculateZOrderForWidget(EWidgetType WidgetType) const;
	bool ShouldPushToNavigation(EWidgetType WidgetType, UUserWidget* InWidget) const;
	void RemoveFromNavigationStack(EWidgetType WidgetType);

};
