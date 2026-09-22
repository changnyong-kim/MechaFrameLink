#include "Data/MechaPartCatalog.h"

#include "Engine/AssetManager.h"

bool UMechaPartCatalog::LoadFromAssetManager(FText& OutError)
{
    UAssetManager& Manager = UAssetManager::Get();
    TArray<FPrimaryAssetId> AssetIds;
    Manager.GetPrimaryAssetIdList(UMechaPartDefinition::AssetType, AssetIds);
    TArray<UMechaPartDefinition*> Definitions;
    for (const FPrimaryAssetId& AssetId : AssetIds)
    {
        UMechaPartDefinition* Definition = Cast<UMechaPartDefinition>(Manager.GetPrimaryAssetPath(AssetId).TryLoad());
        if (!Definition)
        {
            OutError = FText::Format(NSLOCTEXT("MechaCatalog", "LoadFailed", "Could not load part asset: {0}"), FText::FromString(AssetId.ToString()));
            return false;
        }
        Definitions.Add(Definition);
    }
    return Build(Definitions, OutError);
}

bool UMechaPartCatalog::Build(const TArray<UMechaPartDefinition*>& Definitions, FText& OutError)
{
    TMap<FName, TObjectPtr<UMechaPartDefinition>> NewParts;
    for (UMechaPartDefinition* Part : Definitions)
    {
        if (!IsValid(Part) || Part->PartId.IsNone() || Part->DisplayName.IsEmpty() || !Part->Stats.IsValid()
            || !StaticEnum<EMechaPartCategory>()->IsValidEnumValue(static_cast<int64>(Part->Category)))
        {
            OutError = NSLOCTEXT("MechaCatalog", "InvalidPart", "Part catalog contains an invalid definition.");
            return false;
        }
        if (NewParts.Contains(Part->PartId))
        {
            OutError = FText::Format(NSLOCTEXT("MechaCatalog", "DuplicateId", "Duplicate PartId: {0}"), FText::FromName(Part->PartId));
            return false;
        }
        NewParts.Add(Part->PartId, Part);
    }
    Parts = MoveTemp(NewParts);
    OutError = FText::GetEmpty();
    return true;
}

UMechaPartDefinition* UMechaPartCatalog::FindPart(FName PartId) const
{
    const TObjectPtr<UMechaPartDefinition>* Found = Parts.Find(PartId);
    return Found ? Found->Get() : nullptr;
}

TArray<UMechaPartDefinition*> UMechaPartCatalog::GetParts(EMechaPartCategory Category) const
{
    TArray<UMechaPartDefinition*> Result;
    for (const auto& Entry : Parts)
    {
        if (Entry.Value->Category == Category)
        {
            Result.Add(Entry.Value.Get());
        }
    }
    Result.Sort([](const UMechaPartDefinition& A, const UMechaPartDefinition& B)
    {
        return A.SortOrder == B.SortOrder ? A.PartId.LexicalLess(B.PartId) : A.SortOrder < B.SortOrder;
    });
    return Result;
}
