// UIManagerSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulLikeGameGlobal.h"
#include "UIManagerSubsystem.generated.h"

// UI创建参数基结构（可根据需求扩展）
USTRUCT(BlueprintType)
struct FUICreateParams
{
	GENERATED_BODY()

		// 1. 基础信息：所有UI都可能需要
		UPROPERTY(BlueprintReadWrite, EditAnywhere)
		EWidgetType Type; // 你原来的枚举

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UObject* WorldContextObject = nullptr; // 用于获取世界

		// 2. 玩家专属的屏幕UI（旧系统）可能不需要额外参数

		// 3. 世界空间UI专用参数
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		AActor* TargetActor = nullptr; // 要跟随的玩家角色

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FVector WorldOffset = FVector(0, 0, 150); // 头部偏移量，可配置

		// 4. 动态销毁参数（用于解决你的问题2）
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool bEnableDistanceCulling = true; // 是否根据距离销毁

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float DestroyDistance = 3000.0f; // 销毁距离

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
	
	// 获取单例实例
	UFUNCTION(BlueprintPure, Category = "UIManagerSubsystem", meta = (WorldContext = "WorldContextObject"))
		static UUIManagerSubsystem* Get(const UObject* WorldContextObject);
    
    // 根据路径加载BP资源
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
		void RegisterWidgetFromBPPath(EWidgetType WidgetType,const FString& WidgetClassPathstr);

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
	void OpenScreenWidget(EWidgetType WidgetType);

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void OpenWorldWidgetWithActor(const FUICreateParams& CreateParam);

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void CloseWidget(EWidgetType WidgetType);
    
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
    
    // 页面栈管理
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
    /*                                血量系统相关接口                                      */
    /************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "UI_Manager_Health")
    void UpdateHealthUI(float OldHealth);

private:
    // 存储所有注册的界面-蓝图调用
    UPROPERTY()
    TMap<EWidgetType, TSubclassOf<UUserWidget>> RegisteredWidgets;
    
    // 屏幕空间-活动界面（当前打开的）
    UPROPERTY()
    TMap<EWidgetType, UUserWidget*> ActiveWidgets;

    // 世界空间-活动界面(FString key = ActorName + EWidgetType)
	UPROPERTY()
	TMap<FString, UUserWidget*> ActiveWorldWidgets;
    
    // 页面栈
    UPROPERTY()
    TArray<EWidgetType> WidgetStack;
    
    // 当前焦点界面
    UPROPERTY()
    EWidgetType FocusedWidgetName;

};