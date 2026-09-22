#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Equipment/MechaPartStats.h"
#include "MechaPartDefinition.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UTexture2D;

/** Part family, intentionally independent of left/right mounting position. */
UENUM(BlueprintType)
enum class EMechaPartCategory : uint8
{
    Frame,
    Weapon,
    Shoulder,
    Back
};

/** Immutable catalog data. Ownership, selection and equipment state live elsewhere. */
UCLASS(BlueprintType)
class MECHAFRAMELINK_API UMechaPartDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    static const FPrimaryAssetType AssetType;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Part")
    FName PartId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Part")
    EMechaPartCategory Category = EMechaPartCategory::Frame;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Part")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Part", meta = (MultiLine = "true"))
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Part")
    int32 SortOrder = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Part")
    FMechaPartStats Stats;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance", meta = (EditCondition = "Category == EMechaPartCategory::Frame", EditConditionHides))
    TSoftObjectPtr<USkeletalMesh> FrameMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance", meta = (EditCondition = "Category != EMechaPartCategory::Frame", EditConditionHides))
    TSoftObjectPtr<UStaticMesh> AttachmentMesh;

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
