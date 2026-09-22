#pragma once

#include "CoreMinimal.h"
#include "Data/MechaPartDefinition.h"
#include "MechaPartCatalog.generated.h"

/** Small in-memory index of definitions; meshes and icons remain soft references. */
UCLASS(BlueprintType)
class MECHAFRAMELINK_API UMechaPartCatalog : public UObject
{
    GENERATED_BODY()

public:
    bool LoadFromAssetManager(FText& OutError);
    bool Build(const TArray<UMechaPartDefinition*>& Definitions, FText& OutError);

    UFUNCTION(BlueprintPure, Category = "Mecha|Parts")
    UMechaPartDefinition* FindPart(FName PartId) const;

    UFUNCTION(BlueprintPure, Category = "Mecha|Parts")
    TArray<UMechaPartDefinition*> GetParts(EMechaPartCategory Category) const;

private:
    UPROPERTY(Transient)
    TMap<FName, TObjectPtr<UMechaPartDefinition>> Parts;
};
