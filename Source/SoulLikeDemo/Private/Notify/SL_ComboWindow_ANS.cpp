#include "SL_ComboWindow_ANS.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemGlobals.h"

void USL_ComboWindow_ANS::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	AActor* Owner = MeshComp->GetOwner();
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	if (!ASC) return;

	switch (WindowEvent)
	{
	case EComboWindowEvent::BeginWindow:
		if (ComboWindowTag.IsValid())
		{
			ASC->AddLooseGameplayTag(ComboWindowTag);
			UE_LOG(LogTemp, Verbose, TEXT("[ComboNotify] Begin Window: %s"), *ComboWindowTag.ToString());
		}
		break;

	case EComboWindowEvent::AllowBlend:
		// 发送GameplayEvent通知当前GA
	{
		FGameplayEventData EventData;
		EventData.Instigator = Owner;
		EventData.Target = Owner;
		// 将窗口Tag传递给GA，方便GA做日志/判断
		EventData.EventTag = ComboWindowTag;

		ASC->HandleGameplayEvent(
			FGameplayTag::RequestGameplayTag(TEXT("Event.Combo.AllowBlend")),
			&EventData
		);
		UE_LOG(LogTemp, Verbose, TEXT("[ComboNotify] AllowBlend: %s"), *ComboWindowTag.ToString());
	}
	break;

	case EComboWindowEvent::EndWindow:
		if (ComboWindowTag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(ComboWindowTag);
			UE_LOG(LogTemp, Verbose, TEXT("[ComboNotify] End Window: %s"), *ComboWindowTag.ToString());
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