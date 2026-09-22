#include "Data/MechaPartCatalog.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMechaPartCatalogTest, "MechaFrameLink.Hangar.PartCatalog",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMechaPartCatalogTest::RunTest(const FString& Parameters)
{
    UMechaPartCatalog* Catalog = NewObject<UMechaPartCatalog>();
    UMechaPartDefinition* Frame = NewObject<UMechaPartDefinition>();
    Frame->PartId = TEXT("frame.standard");
    Frame->DisplayName = FText::FromString(TEXT("Standard frame"));
    Frame->Stats.AP = 1000.0f;
    UMechaPartDefinition* Weapon = NewObject<UMechaPartDefinition>();
    Weapon->PartId = TEXT("weapon.rifle");
    Weapon->DisplayName = FText::FromString(TEXT("Rifle"));
    Weapon->Category = EMechaPartCategory::Weapon;
    Weapon->Stats.Attack = 100.0f;

    FText Error;
    TestTrue(TEXT("Build valid definitions"), Catalog->Build({Frame, Weapon}, Error));
    TestEqual(TEXT("Weapon list excludes frame"), Catalog->GetParts(EMechaPartCategory::Weapon).Num(), 1);
    TestEqual(TEXT("Resolve gameplay ID"), Catalog->FindPart(Weapon->PartId), Weapon);
    TestNull(TEXT("Unknown ID has no fallback"), Catalog->FindPart(TEXT("missing")));

    UMechaPartDefinition* Duplicate = NewObject<UMechaPartDefinition>();
    Duplicate->PartId = Weapon->PartId;
    Duplicate->DisplayName = FText::FromString(TEXT("Duplicate"));
    TestFalse(TEXT("Reject duplicate IDs"), Catalog->Build({Frame, Weapon, Duplicate}, Error));
    TestFalse(TEXT("Duplicate produces a reason"), Error.IsEmpty());
    TestEqual(TEXT("Failed rebuild preserves existing catalog"), Catalog->FindPart(Weapon->PartId), Weapon);

    Duplicate->PartId = TEXT("invalid.stats");
    Duplicate->Stats.Weight = -1.0f;
    TestFalse(TEXT("Reject negative base stats"), Catalog->Build({Duplicate}, Error));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMechaPartDiscoveryTest, "MechaFrameLink.Hangar.AssetDiscovery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMechaPartDiscoveryTest::RunTest(const FString& Parameters)
{
    UMechaPartCatalog* Catalog = NewObject<UMechaPartCatalog>();
    FText Error;
    TestTrue(TEXT("Load configured catalog"), Catalog->LoadFromAssetManager(Error));
    TestNotNull(TEXT("Discover sample frame through Asset Manager"), Catalog->FindPart(TEXT("frame.standard")));
    TestNotNull(TEXT("Discover sample weapon through Asset Manager"), Catalog->FindPart(TEXT("weapon.rifle")));
    return true;
}
#endif
