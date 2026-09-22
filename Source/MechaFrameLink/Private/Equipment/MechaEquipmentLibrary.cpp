#include "Equipment/MechaEquipmentLibrary.h"

namespace
{
    constexpr EMechaEquipmentSlot Slots[] = {
        EMechaEquipmentSlot::Frame, EMechaEquipmentSlot::LeftWeapon, EMechaEquipmentSlot::RightWeapon,
        EMechaEquipmentSlot::LeftShoulder, EMechaEquipmentSlot::RightShoulder, EMechaEquipmentSlot::Back
    };

    FName* FindSlot(FMechaLoadout& Loadout, EMechaEquipmentSlot Slot)
    {
        switch (Slot)
        {
        case EMechaEquipmentSlot::Frame: return &Loadout.FrameId;
        case EMechaEquipmentSlot::LeftWeapon: return &Loadout.LeftWeaponId;
        case EMechaEquipmentSlot::RightWeapon: return &Loadout.RightWeaponId;
        case EMechaEquipmentSlot::LeftShoulder: return &Loadout.LeftShoulderId;
        case EMechaEquipmentSlot::RightShoulder: return &Loadout.RightShoulderId;
        case EMechaEquipmentSlot::Back: return &Loadout.BackId;
        default: return nullptr;
        }
    }
}

bool UMechaEquipmentLibrary::GetSlotCategory(EMechaEquipmentSlot Slot, EMechaPartCategory& OutCategory)
{
    OutCategory = EMechaPartCategory::Frame;
    switch (Slot)
    {
    case EMechaEquipmentSlot::Frame: return true;
    case EMechaEquipmentSlot::LeftWeapon:
    case EMechaEquipmentSlot::RightWeapon: OutCategory = EMechaPartCategory::Weapon; return true;
    case EMechaEquipmentSlot::LeftShoulder:
    case EMechaEquipmentSlot::RightShoulder: OutCategory = EMechaPartCategory::Shoulder; return true;
    case EMechaEquipmentSlot::Back: OutCategory = EMechaPartCategory::Back; return true;
    default: return false;
    }
}

FName UMechaEquipmentLibrary::GetEquippedPartId(const FMechaLoadout& Loadout, EMechaEquipmentSlot Slot)
{
    FMechaLoadout Copy = Loadout;
    const FName* Id = FindSlot(Copy, Slot);
    return Id ? *Id : NAME_None;
}

bool UMechaEquipmentLibrary::CalculateStats(const UMechaPartCatalog* Catalog, const FMechaLoadout& Loadout,
    FMechaPartStats& OutStats, FText& OutError)
{
    OutStats = FMechaPartStats();
    OutError = FText::GetEmpty();
    if (!IsValid(Catalog))
    {
        OutError = NSLOCTEXT("MechaEquipment", "NoCatalog", "Part catalog is unavailable.");
        return false;
    }
    if (Loadout.FrameId.IsNone())
    {
        OutError = NSLOCTEXT("MechaEquipment", "FrameRequired", "A frame must be equipped.");
        return false;
    }

    FMechaPartStats Total;
    for (EMechaEquipmentSlot Slot : Slots)
    {
        const FName Id = GetEquippedPartId(Loadout, Slot);
        if (Id.IsNone())
        {
            continue;
        }
        EMechaPartCategory RequiredCategory;
        GetSlotCategory(Slot, RequiredCategory);
        const UMechaPartDefinition* Part = Catalog->FindPart(Id);
        if (!Part || Part->Category != RequiredCategory || !Part->Stats.IsValid())
        {
            OutError = FText::Format(NSLOCTEXT("MechaEquipment", "InvalidPart", "Part {0} is missing, invalid, or incompatible with this slot."), FText::FromName(Id));
            return false;
        }
        // Count each occupied position, including identical parts on both sides.
        Total += Part->Stats;
    }
    if (!Total.IsValid())
    {
        OutError = NSLOCTEXT("MechaEquipment", "Overflow", "Equipment stats exceed the supported numeric range.");
        return false;
    }
    OutStats = Total;
    return true;
}

bool UMechaEquipmentLibrary::ComparePart(const UMechaPartCatalog* Catalog, const FMechaLoadout& Current,
    EMechaEquipmentSlot Slot, FName CandidatePartId, FMechaEquipmentComparison& OutComparison, FText& OutError)
{
    OutComparison = FMechaEquipmentComparison();
    FMechaEquipmentComparison Result;
    Result.PreviewLoadout = Current;
    FName* Target = FindSlot(Result.PreviewLoadout, Slot);
    if (!Target)
    {
        OutError = NSLOCTEXT("MechaEquipment", "InvalidSlot", "Unknown equipment slot.");
        return false;
    }
    if (!CalculateStats(Catalog, Current, Result.CurrentStats, OutError))
    {
        return false;
    }
    *Target = CandidatePartId;
    if (!CalculateStats(Catalog, Result.PreviewLoadout, Result.PreviewStats, OutError))
    {
        return false;
    }
    Result.Delta = Result.PreviewStats - Result.CurrentStats;
    OutComparison = Result;
    return true;
}
