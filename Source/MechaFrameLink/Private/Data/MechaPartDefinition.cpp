#include "Data/MechaPartDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

const FPrimaryAssetType UMechaPartDefinition::AssetType(TEXT("MechaPart"));

FPrimaryAssetId UMechaPartDefinition::GetPrimaryAssetId() const
{
    // Asset identity and gameplay PartId stay separate so duplicate PartIds can be detected.
    return FPrimaryAssetId(AssetType, GetFName());
}

#if WITH_EDITOR
EDataValidationResult UMechaPartDefinition::IsDataValid(FDataValidationContext& Context) const
{
    Super::IsDataValid(Context);
    bool bValid = true;
    if (PartId.IsNone())
    {
        Context.AddError(NSLOCTEXT("MechaPart", "MissingId", "PartId must not be empty."));
        bValid = false;
    }
    if (DisplayName.IsEmpty())
    {
        Context.AddError(NSLOCTEXT("MechaPart", "MissingName", "DisplayName must not be empty."));
        bValid = false;
    }
    if (!StaticEnum<EMechaPartCategory>()->IsValidEnumValue(static_cast<int64>(Category)) || !Stats.IsValid())
    {
        Context.AddError(NSLOCTEXT("MechaPart", "InvalidData", "Category and non-negative finite stats are required."));
        bValid = false;
    }
    if ((Category == EMechaPartCategory::Frame && !AttachmentMesh.IsNull())
        || (Category != EMechaPartCategory::Frame && !FrameMesh.IsNull()))
    {
        Context.AddError(NSLOCTEXT("MechaPart", "WrongMesh", "Frames use FrameMesh; equipment uses AttachmentMesh."));
        bValid = false;
    }
    if (Icon.IsNull() || (Category == EMechaPartCategory::Frame ? FrameMesh.IsNull() : AttachmentMesh.IsNull()))
    {
        // Data-only samples are usable before final art arrives.
        Context.AddWarning(NSLOCTEXT("MechaPart", "MissingArt", "Icon or mesh is missing; this part currently supports data-only use."));
    }
    return bValid ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}
#endif
