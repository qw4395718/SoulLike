#include "SL_ComboWindow_ANS.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemGlobals.h"
#include <SL_AbilitySystemComponent.h>
#include <AT/AbilityTask_ComboMontage.h>
#include <SL_ComboManagerComponent.h>

void USL_ComboWindow_ANS::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	// 获取ASC
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(MeshComp->GetOwner());
	if (!ASCInterface) return;

	// 获取ComboManager
	USL_ComboManagerComponent* comboManagerComp = MeshComp->GetOwner()->FindComponentByClass<USL_ComboManagerComponent>();
	if (!comboManagerComp) return;

	USL_AbilitySystemComponent* ASC = Cast<USL_AbilitySystemComponent>(ASCInterface->GetAbilitySystemComponent());
	if (!ASC) return;

	switch (WindowEvent)
	{
	case EComboWindowEvent::BeginWindow:
		// 清理上一轮的窗口Tag
		// 给角色添加窗口Tag
		if (ComboWindowTag.IsValid())
		{
			ASC->AddLooseGameplayTag(ComboWindowTag);
			comboManagerComp->SetNeedClearTag(ComboWindowTag);
			UE_LOG(LogTemp, Warning, TEXT("WindowTag_Notify BeginWindow: %s"), *ComboWindowTag.ToString());

		}
		break;

	case EComboWindowEvent::AllowBlend:
		// 通知当前激活的ComboTask：可以物理混合了
		// Task会绑定在Ability上，我们通过ASC查找
	{
		TArray<UGameplayTask*> ActiveTasks = ASC->GetCurrentlyActiveTasks();
		for (UGameplayTask* Task : ActiveTasks)
		{
			UAbilityTask_ComboMontage* ComboTask = Cast<UAbilityTask_ComboMontage>(Task);
			if (ComboTask && ComboTask->IsActive())
			{
				ComboTask->OnAllowBlendReached(ComboWindowTag);
				break;
			}
		}
	}
	break;

	case EComboWindowEvent::EndWindow:
		// 移除窗口Tag
		if (ComboWindowTag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(ComboWindowTag);
			UE_LOG(LogTemp, Warning, TEXT("WindowTag_Notify EndWindow: %s"), *ComboWindowTag.ToString());
		}
		break;
	}
}

FString USL_ComboWindow_ANS::GetNotifyName_Implementation() const
{
	FString EventName;
	switch (WindowEvent)
	{
	case EComboWindowEvent::BeginWindow: EventName = TEXT("Begin"); break;
	case EComboWindowEvent::AllowBlend:  EventName = TEXT("Blend"); break;
	case EComboWindowEvent::EndWindow:   EventName = TEXT("End"); break;
	}
	return FString::Printf(TEXT("Combo [%s] - %s"), *ComboWindowTag.ToString(), *EventName);
}