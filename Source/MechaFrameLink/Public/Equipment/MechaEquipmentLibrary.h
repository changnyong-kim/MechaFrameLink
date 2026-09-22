#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/MechaPartCatalog.h"
#include "Equipment/MechaLoadout.h"
#include "MechaEquipmentLibrary.generated.h"

/** Pure equipment rules shared by the hangar and future lobby validation. */
UCLASS()
class MECHAFRAMELINK_API UMechaEquipmentLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Mecha|Equipment")
    static bool GetSlotCategory(EMechaEquipmentSlot Slot, EMechaPartCategory& OutCategory);

    UFUNCTION(BlueprintPure, Category = "Mecha|Equipment")
    static FName GetEquippedPartId(const FMechaLoadout& Loadout, EMechaEquipmentSlot Slot);

    /** Empty attachment positions are valid; a frame is always required. */
    UFUNCTION(BlueprintCallable, Category = "Mecha|Equipment")
    static bool CalculateStats(const UMechaPartCatalog* Catalog, const FMechaLoadout& Loadout,
        FMechaPartStats& OutStats, FText& OutError);

    /** NAME_None previews unequipping. This never changes the supplied loadout. */
    UFUNCTION(BlueprintCallable, Category = "Mecha|Equipment")
    static bool ComparePart(const UMechaPartCatalog* Catalog, const FMechaLoadout& Current,
        EMechaEquipmentSlot Slot, FName CandidatePartId,
        FMechaEquipmentComparison& OutComparison, FText& OutError);
};
