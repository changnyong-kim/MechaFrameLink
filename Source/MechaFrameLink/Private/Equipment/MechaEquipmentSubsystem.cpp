#include "Equipment/MechaEquipmentSubsystem.h"

#include "Equipment/MechaEquipmentLibrary.h"
#include "Engine/AssetManager.h"

void UMechaEquipmentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSubsystemActive = true;
    // In editor builds the initial asset scan may finish after GameInstance startup.
    UAssetManager::CallOrRegister_OnCompletedInitialScan(
        FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::HandleInitialAssetScanCompleted));
}

void UMechaEquipmentSubsystem::Deinitialize()
{
    bSubsystemActive = false;
    bInitialized = false;
    bSetupComplete = false;
    Catalog = nullptr;
    CurrentLoadout = FMechaLoadout();
    CurrentStats = FMechaPartStats();
    OnLoadoutChanged.Clear();
    Super::Deinitialize();
}

void UMechaEquipmentSubsystem::HandleInitialAssetScanCompleted()
{
    // The weak UObject delegate can outlive a stopped PIE GameInstance until GC.
    if (!bSubsystemActive)
    {
        return;
    }
    FText Error;
    if (!EnsureEquipmentReady(Error))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Equipment] Startup initialization failed: %s"), *Error.ToString());
    }
}

bool UMechaEquipmentSubsystem::EnsureEquipmentReady(FText& OutError)
{
    OutError = FText::GetEmpty();
    if (bInitialized)
    {
        return true;
    }

    if (!UAssetManager::Get().HasInitialScanCompleted())
    {
        OutError = NSLOCTEXT("MechaEquipment", "CatalogLoading", "Part catalog is still loading. Please try again shortly.");
        return false;
    }

    UMechaPartCatalog* NewCatalog = NewObject<UMechaPartCatalog>(this);
    if (!NewCatalog->LoadFromAssetManager(OutError))
    {
        return false;
    }
    const TArray<UMechaPartDefinition*> Frames = NewCatalog->GetParts(EMechaPartCategory::Frame);
    if (Frames.IsEmpty())
    {
        OutError = NSLOCTEXT("MechaEquipment", "NoFrames", "At least one frame part is required to initialize equipment.");
        return false;
    }
    FMechaLoadout InitialLoadout;
    InitialLoadout.FrameId = Frames[0]->PartId;
    FMechaPartStats InitialStats;
    if (!UMechaEquipmentLibrary::CalculateStats(NewCatalog, InitialLoadout, InitialStats, OutError))
    {
        return false;
    }

    Catalog = NewCatalog;
    CurrentLoadout = InitialLoadout;
    CurrentStats = InitialStats;
    bInitialized = true;
    OnLoadoutChanged.Broadcast(CurrentLoadout);
    return true;
}

TArray<UMechaPartDefinition*> UMechaEquipmentSubsystem::GetPartsForSlot(EMechaEquipmentSlot Slot) const
{
    EMechaPartCategory Category;
    if (Catalog && UMechaEquipmentLibrary::GetSlotCategory(Slot, Category))
    {
        return Catalog->GetParts(Category);
    }
    return {};
}

bool UMechaEquipmentSubsystem::CompareCandidate(EMechaEquipmentSlot Slot, FName PartId,
    FMechaEquipmentComparison& OutComparison, FText& OutError) const
{
    return UMechaEquipmentLibrary::ComparePart(Catalog, CurrentLoadout, Slot, PartId, OutComparison, OutError);
}

bool UMechaEquipmentSubsystem::EquipPart(EMechaEquipmentSlot Slot, FName PartId, FText& OutError)
{
    if (PartId.IsNone())
    {
        OutError = NSLOCTEXT("MechaEquipment", "EmptyPartId", "Select a part to equip.");
        return false;
    }
    return ApplyChange(Slot, PartId, OutError);
}

bool UMechaEquipmentSubsystem::UnequipPart(EMechaEquipmentSlot Slot, FText& OutError)
{
    return ApplyChange(Slot, NAME_None, OutError);
}

bool UMechaEquipmentSubsystem::ApplyChange(EMechaEquipmentSlot Slot, FName PartId, FText& OutError)
{
    FMechaEquipmentComparison Comparison;
    if (!CompareCandidate(Slot, PartId, Comparison, OutError))
    {
        return false;
    }
    if (CurrentLoadout == Comparison.PreviewLoadout)
    {
        return true;
    }
    CurrentLoadout = Comparison.PreviewLoadout;
    CurrentStats = Comparison.PreviewStats;
    bSetupComplete = false;
    OnLoadoutChanged.Broadcast(CurrentLoadout);
    return true;
}

bool UMechaEquipmentSubsystem::CompleteSetup(FText& OutError)
{
    // Both direct lobby entry and hangar completion use this gate.
    if (!EnsureEquipmentReady(OutError))
    {
        bSetupComplete = false;
        return false;
    }
    FMechaPartStats ValidatedStats;
    if (!UMechaEquipmentLibrary::CalculateStats(Catalog, CurrentLoadout, ValidatedStats, OutError))
    {
        bSetupComplete = false;
        return false;
    }
    CurrentStats = ValidatedStats;
    bSetupComplete = true;
    return true;
}
