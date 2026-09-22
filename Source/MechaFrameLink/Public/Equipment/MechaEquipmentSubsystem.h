#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Equipment/MechaLoadout.h"
#include "MechaEquipmentSubsystem.generated.h"

class UMechaPartCatalog;
class UMechaPartDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMechaLoadoutChanged, const FMechaLoadout&, Loadout);

/** Owns one local configuration for this game session, across hangar/lobby screens. */
UCLASS()
class MECHAFRAMELINK_API UMechaEquipmentSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Idempotent readiness check/retry for any screen. Never replaces an existing loadout. */
    UFUNCTION(BlueprintCallable, Category = "Mecha|Equipment")
    bool EnsureEquipmentReady(FText& OutError);

    UFUNCTION(BlueprintPure, Category = "Mecha|Equipment")
    bool IsEquipmentReady() const { return bInitialized; }

    UFUNCTION(BlueprintPure, Category = "Mecha|Equipment")
    FMechaLoadout GetCurrentLoadout() const { return CurrentLoadout; }

    UFUNCTION(BlueprintPure, Category = "Mecha|Equipment")
    FMechaPartStats GetCurrentStats() const { return CurrentStats; }

    UFUNCTION(BlueprintPure, Category = "Mecha|Equipment")
    UMechaPartCatalog* GetCatalog() const { return Catalog; }

    UFUNCTION(BlueprintPure, Category = "Mecha|Equipment")
    TArray<UMechaPartDefinition*> GetPartsForSlot(EMechaEquipmentSlot Slot) const;

    UFUNCTION(BlueprintCallable, Category = "Mecha|Equipment")
    bool CompareCandidate(EMechaEquipmentSlot Slot, FName PartId,
        FMechaEquipmentComparison& OutComparison, FText& OutError) const;

    UFUNCTION(BlueprintCallable, Category = "Mecha|Equipment")
    bool EquipPart(EMechaEquipmentSlot Slot, FName PartId, FText& OutError);

    UFUNCTION(BlueprintCallable, Category = "Mecha|Equipment")
    bool UnequipPart(EMechaEquipmentSlot Slot, FText& OutError);

    UFUNCTION(BlueprintCallable, Category = "Mecha|Equipment")
    bool CompleteSetup(FText& OutError);

    UFUNCTION(BlueprintPure, Category = "Mecha|Equipment")
    bool IsSetupComplete() const { return bSetupComplete; }

    UPROPERTY(BlueprintAssignable, Category = "Mecha|Equipment")
    FMechaLoadoutChanged OnLoadoutChanged;

private:
    void HandleInitialAssetScanCompleted();
    bool ApplyChange(EMechaEquipmentSlot Slot, FName PartId, FText& OutError);

    UPROPERTY(Transient)
    TObjectPtr<UMechaPartCatalog> Catalog;

    UPROPERTY(Transient)
    FMechaLoadout CurrentLoadout;

    UPROPERTY(Transient)
    FMechaPartStats CurrentStats;

    bool bInitialized = false;
    bool bSetupComplete = false;
    bool bSubsystemActive = false;
};
