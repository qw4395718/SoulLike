#include "SL_WeaponAnimSet.h"

UAnimMontage* USL_WeaponAnimSet::GetComboMontageByTag(const FGameplayTag& InTag) const
{
    if (!InTag.IsValid()) return nullptr;

    const TSoftObjectPtr<UAnimMontage>* Found = ComboMontages.Find(InTag);
    if (Found && !Found->IsNull())
    {
        return Found->LoadSynchronous();
    }
    return nullptr;
}
