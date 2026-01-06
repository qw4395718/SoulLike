// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "WeaponCoordinatorControl_IF.h"
#include "WeaponComboCoordinatorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API UWeaponComboCoordinatorComponent : public UActorComponent,public IWeaponCoordinatorControl_IF
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComboCoordinatorComponent();
	/************************************************************************/
	/*接口实现                                                                     */
	/************************************************************************/
		// 武器伤害获取(int -> enum)
	UFUNCTION()
		float GetAttackComboDamage(int type) override;

	// 武器行为消耗获取(int -> enum)
	UFUNCTION()
		float GetAttackStateCost(int type, int CostType) override;

	/************************************************************************/
	/*外部调用                                                                     */
	/************************************************************************/
	void InitComboCoordinatorComponet(const TMap<EWeaponModeTyoe, FComboCoordinatorInfo> Map);

	UFUNCTION()
		void ActiveComboWindowInputState(EWeaponModeTyoe type);

	UFUNCTION()
		void InActiveComboWindowInputState(EWeaponModeTyoe type);

	UFUNCTION()
		int GetNextComboNum(EWeaponModeTyoe type);

protected:
	/************************************************************************/
	/*内部调用                                                                     */
	/************************************************************************/


protected:
	/************************************************************************/
	/*内部变量                                                                     */
	/************************************************************************/
	// 管理器数据
	TMap<EWeaponModeTyoe, FComboCoordinatorInfo> ComboCoordinatorInfoMap;
};
