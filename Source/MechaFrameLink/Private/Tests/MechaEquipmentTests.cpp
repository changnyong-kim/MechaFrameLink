#include "Equipment/MechaEquipmentLibrary.h"
#include "Equipment/MechaEquipmentSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

namespace
{
    UMechaPartDefinition* MakePart(FName Id, EMechaPartCategory Category, float Attack, float Weight)
    {
        UMechaPartDefinition* Part = NewObject<UMechaPartDefinition>();
        Part->PartId = Id;
        Part->DisplayName = FText::FromName(Id);
        Part->Category = Category;
        Part->Stats.Attack = Attack;
        Part->Stats.Weight = Weight;
        return Part;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMechaEquipmentRulesTest, "MechaFrameLink.Hangar.EquipmentRules",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMechaEquipmentRulesTest::RunTest(const FString& Parameters)
{
    UMechaPartCatalog* Catalog = NewObject<UMechaPartCatalog>();
    UMechaPartDefinition* Frame = MakePart(TEXT("frame"), EMechaPartCategory::Frame, 0, 1000);
    Frame->Stats.AP = 1000;
    UMechaPartDefinition* HeavyFrame = MakePart(TEXT("heavy"), EMechaPartCategory::Frame, 0, 1600);
    HeavyFrame->Stats.AP = 1500;
    UMechaPartDefinition* Rifle = MakePart(TEXT("rifle"), EMechaPartCategory::Weapon, 100, 120);
    UMechaPartDefinition* Cannon = MakePart(TEXT("cannon"), EMechaPartCategory::Weapon, 180, 240);
    UMechaPartDefinition* Shoulder = MakePart(TEXT("shoulder"), EMechaPartCategory::Shoulder, 80, 150);
    UMechaPartDefinition* Back = MakePart(TEXT("back"), EMechaPartCategory::Back, 0, 100);
    FText Error;
    if (!TestTrue(TEXT("Create rule-test catalog"), Catalog->Build({Frame, HeavyFrame, Rifle, Cannon, Shoulder, Back}, Error)))
    {
        return false;
    }

    FMechaLoadout Current;
    Current.FrameId = Frame->PartId;
    Current.LeftWeaponId = Current.RightWeaponId = Rifle->PartId;
    Current.LeftShoulderId = Current.RightShoulderId = Shoulder->PartId;
    Current.BackId = Back->PartId;
    const FMechaLoadout Original = Current;
    FMechaPartStats Total;
    TestTrue(TEXT("All six positions are compatible"), UMechaEquipmentLibrary::CalculateStats(Catalog, Current, Total, Error));
    TestEqual(TEXT("Same part on both sides counts twice"), Total.Attack, 360.0f);
    TestEqual(TEXT("Frame plus five attachments"), Total.Weight, 1640.0f);

    FMechaEquipmentComparison Comparison;
    TestTrue(TEXT("Compare left weapon replacement"), UMechaEquipmentLibrary::ComparePart(Catalog, Current, EMechaEquipmentSlot::LeftWeapon, Cannon->PartId, Comparison, Error));
    TestEqual(TEXT("Compare replaces instead of adding on top"), Comparison.Delta.Attack, 80.0f);
    TestEqual(TEXT("Weight cost delta"), Comparison.Delta.Weight, 120.0f);
    TestEqual(TEXT("Other side is preserved"), Comparison.PreviewLoadout.RightWeaponId, Rifle->PartId);
    TestTrue(TEXT("Comparison leaves current configuration untouched"), Current == Original);

    TestTrue(TEXT("Preview attachment removal"), UMechaEquipmentLibrary::ComparePart(Catalog, Current, EMechaEquipmentSlot::RightShoulder, NAME_None, Comparison, Error));
    TestEqual(TEXT("Removal has negative delta"), Comparison.Delta.Attack, -80.0f);
    TestEqual(TEXT("Only requested shoulder becomes empty"), Comparison.PreviewLoadout.RightShoulderId, NAME_None);
    TestEqual(TEXT("Left shoulder remains mounted"), Comparison.PreviewLoadout.LeftShoulderId, Shoulder->PartId);

    TestTrue(TEXT("Whole frame is replaceable"), UMechaEquipmentLibrary::ComparePart(Catalog, Current, EMechaEquipmentSlot::Frame, HeavyFrame->PartId, Comparison, Error));
    TestEqual(TEXT("Frame AP comparison"), Comparison.Delta.AP, 500.0f);
    TestEqual(TEXT("Frame replacement keeps back attachment ID"), Comparison.PreviewLoadout.BackId, Back->PartId);

    TestFalse(TEXT("Frame cannot be removed"), UMechaEquipmentLibrary::ComparePart(Catalog, Current, EMechaEquipmentSlot::Frame, NAME_None, Comparison, Error));
    TestFalse(TEXT("Incompatible slot rejected"), UMechaEquipmentLibrary::ComparePart(Catalog, Current, EMechaEquipmentSlot::Back, Rifle->PartId, Comparison, Error));
    TestFalse(TEXT("Unknown ID rejected"), UMechaEquipmentLibrary::ComparePart(Catalog, Current, EMechaEquipmentSlot::LeftWeapon, TEXT("missing"), Comparison, Error));
    TestFalse(TEXT("Unknown slot rejected"), UMechaEquipmentLibrary::ComparePart(Catalog, Current, static_cast<EMechaEquipmentSlot>(255), Rifle->PartId, Comparison, Error));
    TestTrue(TEXT("All failed comparisons preserve original"), Current == Original);
    TestTrue(TEXT("Failed output reset"), Comparison.PreviewLoadout.FrameId.IsNone());

    FMechaLoadout Empty;
    TestFalse(TEXT("Missing frame rejected"), UMechaEquipmentLibrary::CalculateStats(Catalog, Empty, Total, Error));
    Empty.FrameId = Frame->PartId;
    TestTrue(TEXT("All five attachments may be empty"), UMechaEquipmentLibrary::CalculateStats(Catalog, Empty, Total, Error));
    Current.BackId = Rifle->PartId;
    TestFalse(TEXT("Validate entire loadout, not just candidate"), UMechaEquipmentLibrary::ComparePart(Catalog, Current, EMechaEquipmentSlot::Frame, HeavyFrame->PartId, Comparison, Error));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMechaEquipmentSessionTest, "MechaFrameLink.Hangar.EquipmentSession",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMechaEquipmentSessionTest::RunTest(const FString& Parameters)
{
    UGameInstance* Instance = NewObject<UGameInstance>();
    UMechaEquipmentSubsystem* Equipment = NewObject<UMechaEquipmentSubsystem>(Instance);
    FText Error;
    if (!TestTrue(TEXT("Initialize real sample catalog"), Equipment->EnsureEquipmentReady(Error)))
    {
        AddError(Error.ToString());
        return false;
    }
    TestFalse(TEXT("Initial setup not yet confirmed"), Equipment->IsSetupComplete());
    TestTrue(TEXT("Shared weapon catalog"), Equipment->GetPartsForSlot(EMechaEquipmentSlot::LeftWeapon) == Equipment->GetPartsForSlot(EMechaEquipmentSlot::RightWeapon));
    TestTrue(TEXT("Equip left rifle"), Equipment->EquipPart(EMechaEquipmentSlot::LeftWeapon, TEXT("weapon.rifle"), Error));
    TestTrue(TEXT("Equip right rifle"), Equipment->EquipPart(EMechaEquipmentSlot::RightWeapon, TEXT("weapon.rifle"), Error));
    const FMechaLoadout Equipped = Equipment->GetCurrentLoadout();

    FMechaEquipmentComparison Comparison;
    TestTrue(TEXT("Preview cannon"), Equipment->CompareCandidate(EMechaEquipmentSlot::LeftWeapon, TEXT("weapon.cannon"), Comparison, Error));
    TestTrue(TEXT("Discarding preview requires no rollback"), Equipment->GetCurrentLoadout() == Equipped);
    TestTrue(TEXT("Complete setup"), Equipment->CompleteSetup(Error));
    TestTrue(TEXT("Completion flag set"), Equipment->IsSetupComplete());
    TestTrue(TEXT("Re-entering hangar is idempotent"), Equipment->EnsureEquipmentReady(Error));
    TestTrue(TEXT("Configuration persists on re-entry"), Equipment->GetCurrentLoadout() == Equipped);
    TestTrue(TEXT("No-op equip succeeds"), Equipment->EquipPart(EMechaEquipmentSlot::LeftWeapon, TEXT("weapon.rifle"), Error));
    TestTrue(TEXT("No-op preserves completion"), Equipment->IsSetupComplete());

    TestFalse(TEXT("Unknown ID cannot mutate configuration"), Equipment->EquipPart(EMechaEquipmentSlot::Back, TEXT("bad.id"), Error));
    TestFalse(TEXT("Empty ID must use explicit unequip"), Equipment->EquipPart(EMechaEquipmentSlot::LeftWeapon, NAME_None, Error));
    TestFalse(TEXT("Cannot remove frame"), Equipment->UnequipPart(EMechaEquipmentSlot::Frame, Error));
    TestTrue(TEXT("Rejected mutations preserve completion"), Equipment->IsSetupComplete());
    TestTrue(TEXT("Rejected mutations preserve configuration"), Equipment->GetCurrentLoadout() == Equipped);

    TestTrue(TEXT("Unequip left weapon"), Equipment->UnequipPart(EMechaEquipmentSlot::LeftWeapon, Error));
    TestTrue(TEXT("Removed slot is empty"), Equipment->GetCurrentLoadout().LeftWeaponId.IsNone());
    TestEqual(TEXT("Right weapon remains"), Equipment->GetCurrentLoadout().RightWeaponId, FName(TEXT("weapon.rifle")));
    TestFalse(TEXT("Actual change requires reconfirmation"), Equipment->IsSetupComplete());
    TestEqual(TEXT("Cached total updated after unequip"), Equipment->GetCurrentStats().Attack, 100.0f);

    UGameInstance* OtherInstance = NewObject<UGameInstance>();
    UMechaEquipmentSubsystem* OtherEquipment = NewObject<UMechaEquipmentSubsystem>(OtherInstance);
    TestTrue(TEXT("Independent game instance initializes"), OtherEquipment->EnsureEquipmentReady(Error));
    TestTrue(TEXT("No cross-instance loadout leakage"), OtherEquipment->GetCurrentLoadout().RightWeaponId.IsNone());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMechaDirectLobbyTest, "MechaFrameLink.Hangar.DirectLobbyStartup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMechaDirectLobbyTest::RunTest(const FString& Parameters)
{
    // Exercise the actual GameInstance subsystem lifecycle, without constructing a hangar widget.
    UGameInstance* Instance = NewObject<UGameInstance>(GEngine);
    Instance->InitializeStandalone();
    UWorld* World = Instance->GetWorld();
    UMechaEquipmentSubsystem* Equipment = Instance->GetSubsystem<UMechaEquipmentSubsystem>();
    if (TestNotNull(TEXT("GameInstance creates equipment subsystem"), Equipment))
    {
        TestTrue(TEXT("Startup prepares equipment before any screen opens"), Equipment->IsEquipmentReady());
        TestFalse(TEXT("Startup loadout has a frame"), Equipment->GetCurrentLoadout().FrameId.IsNone());
        TestTrue(TEXT("Startup stats calculated"), Equipment->GetCurrentStats().AP > 0.0f);
        TestFalse(TEXT("Data readiness is separate from setup confirmation"), Equipment->IsSetupComplete());
        FText Error;
        TestTrue(TEXT("Direct lobby gate succeeds without visiting hangar"), Equipment->CompleteSetup(Error));
        TestTrue(TEXT("Direct lobby validates setup"), Equipment->IsSetupComplete());

        TestTrue(TEXT("Later hangar edit equips a weapon"), Equipment->EquipPart(EMechaEquipmentSlot::LeftWeapon, TEXT("weapon.cannon"), Error));
        const FMechaLoadout Edited = Equipment->GetCurrentLoadout();
        TestTrue(TEXT("Another screen can check readiness"), Equipment->EnsureEquipmentReady(Error));
        TestTrue(TEXT("Checking readiness never resets edited loadout"), Equipment->GetCurrentLoadout() == Edited);
        TestTrue(TEXT("Lobby accepts the edited setup"), Equipment->CompleteSetup(Error));

        // Invalidate this instance's catalog without changing any on-disk assets.
        Equipment->GetCatalog()->Build({}, Error);
        TestFalse(TEXT("Lobby gate rejects a configuration whose parts cannot resolve"), Equipment->CompleteSetup(Error));
        TestFalse(TEXT("Validation failure revokes old completion state"), Equipment->IsSetupComplete());
        TestFalse(TEXT("Lobby rejection explains the error"), Error.IsEmpty());
        TestTrue(TEXT("Rejected entry preserves loadout IDs"), Equipment->GetCurrentLoadout() == Edited);
    }
    Instance->Shutdown();
    if (World)
    {
        GEngine->DestroyWorldContext(World);
        World->DestroyWorld(false);
    }

    // Also cover the completion gate's recovery path when invoked before explicit preparation.
    UGameInstance* OtherInstance = NewObject<UGameInstance>();
    UMechaEquipmentSubsystem* OtherEquipment = NewObject<UMechaEquipmentSubsystem>(OtherInstance);
    FText Error;
    TestFalse(TEXT("Unprepared test instance starts empty"), OtherEquipment->IsEquipmentReady());
    TestTrue(TEXT("Completion gate prepares missing equipment itself"), OtherEquipment->CompleteSetup(Error));
    TestTrue(TEXT("Recovery leaves usable data"), OtherEquipment->IsEquipmentReady());
    return true;
}
#endif
