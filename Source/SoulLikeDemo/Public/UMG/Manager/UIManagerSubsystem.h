// UIManagerSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulLikeGameGlobal.h"
#include "UIManagerSubsystem.generated.h"

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

	// 注册/注销界面,蓝图调用
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void RegisterWidget(EWidgetType WidgetType,TSubclassOf<UUserWidget> WidgetClass);
    
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void UnregisterWidget(EWidgetType WidgetType);

	// 打开/关闭界面
	  UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void OpenWidget(EWidgetType WidgetType);
    
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void CloseWidget(EWidgetType WidgetType);
    
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void CloseAllWidgets();

	// 获取界面
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    UUserWidget* GetWidget(EWidgetType WidgetType) const;
    
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    bool IsWidgetOpen(EWidgetType WidgetType) const;
    
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

private:
    // 存储所有注册的界面-蓝图调用
    UPROPERTY()
    TMap<EWidgetType, TSubclassOf<UUserWidget>> RegisteredWidgets;
    
    // 活动界面（当前打开的）
    UPROPERTY()
    TMap<EWidgetType, UUserWidget*> ActiveWidgets;
    
    // 页面栈
    UPROPERTY()
    TArray<EWidgetType> WidgetStack;
    
    // 当前焦点界面
    UPROPERTY()
    EWidgetType FocusedWidgetName;

};