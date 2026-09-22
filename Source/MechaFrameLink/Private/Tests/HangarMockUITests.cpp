#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Equipment/MechaEquipmentSubsystem.h"
#include "Misc/Paths.h"
#include "UI/Core/FrontendRootWidget.h"
#include "UI/Core/FrontendUISubsystem.h"
#include "UI/Hangar/HangarMockScreen.h"
#include "UI/MainMenu/MainMenuScreen.h"
#include "UnrealClient.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "UObject/UnrealType.h"

namespace
{
    class FHangarMockInteraction : public IAutomationLatentCommand
    {
    public:
        FHangarMockInteraction(FAutomationTestBase* InTest, UGameInstance* InInstance,
            UCommonActivatableWidgetStack* InStack, UClass* InHangarClass)
            : Test(InTest), Instance(InInstance), Stack(InStack), HangarClass(InHangarClass) {}

        virtual bool Update() override
        {
            if (FPlatformTime::Seconds() < NextTime) { return false; }
            UMechaEquipmentSubsystem* Equipment = Instance->GetSubsystem<UMechaEquipmentSubsystem>();
            auto Click = [this](const FString& Text)
            {
                TArray<UWidget*> Widgets;
                Screen->WidgetTree->GetAllWidgets(Widgets);
                for (UWidget* Widget : Widgets)
                {
                    UHangarMockButton* Button = Cast<UHangarMockButton>(Widget);
                    UTextBlock* Label = Button ? Cast<UTextBlock>(Button->GetContent()) : nullptr;
                    if (Label && Label->GetText().ToString().Contains(Text))
                    {
                        if (!Test->TestTrue(TEXT("UI button is enabled: ") + Text, Button->GetIsEnabled())) { return false; }
                        Button->OnClicked.Broadcast();
                        return true;
                    }
                }
                Test->AddError(TEXT("Missing mock button: ") + Text);
                return false;
            };

            if (Step == 0)
            {
                Screen = Cast<UHangarMockScreen>(Instance->GetSubsystem<UFrontendUISubsystem>()->PushMainScreen(HangarClass));
                if (!Test->TestNotNull(TEXT("Configured hangar screen opens"), Screen.Get())) { return true; }
            }
            else if (Step == 1)
            {
                if (!Test->TestTrue(TEXT("Hangar is active in actual game stack"), Screen->IsActivated())) { return true; }
                const FMechaLoadout Original = Equipment->GetCurrentLoadout();
                if (!Click(TEXT("LeftWeapon")) || !Click(TEXT("weapon.cannon"))) { return true; }
                Test->TestTrue(TEXT("Candidate UI preserves current configuration"), Equipment->GetCurrentLoadout() == Original);
                if (!Click(TEXT("Equip candidate"))) { return true; }
                Test->TestEqual(TEXT("Equip UI changes left weapon"), Equipment->GetCurrentLoadout().LeftWeaponId, FName(TEXT("weapon.cannon")));
                if (!Click(TEXT("Unequip slot"))) { return true; }
                Test->TestTrue(TEXT("Unequip UI clears selected position"), Equipment->GetCurrentLoadout().LeftWeaponId.IsNone());
                if (!Click(TEXT("weapon.rifle")) || !Click(TEXT("Clear comparison"))) { return true; }
                Test->TestTrue(TEXT("Cancel UI leaves slot empty"), Equipment->GetCurrentLoadout().LeftWeaponId.IsNone());
                if (!Click(TEXT("Frame")) || !Click(TEXT("frame.heavy")) || !Click(TEXT("Equip candidate"))) { return true; }
                Test->TestEqual(TEXT("Frame UI replaces whole frame"), Equipment->GetCurrentLoadout().FrameId, FName(TEXT("frame.heavy")));
                if (!Click(TEXT("Complete setup > Lobby"))) { return true; }
            }
            else if (Step == 2)
            {
                UCommonActivatableWidget* Active = Stack->GetActiveWidget();
                if (!Test->TestNotNull(TEXT("Lobby screen exists"), Active)) { return true; }
                Test->TestEqual(TEXT("Complete UI opens configured lobby"), Active->GetClass()->GetName(), FString(TEXT("WBP_Lobby_C")));
                Test->TestTrue(TEXT("Complete UI confirms equipment"), Equipment->IsSetupComplete());
                Active->DeactivateWidget();
            }
            else if (Step == 3)
            {
                Test->TestTrue(TEXT("Returning from lobby restores hangar"), Screen->IsActivated());
                if (!Click(TEXT("Back to menu"))) { return true; }
            }
            else if (Step == 4)
            {
                Test->TestTrue(TEXT("Back button restores main menu"), Stack->GetActiveWidget() && Stack->GetActiveWidget()->IsA<UMainMenuScreen>());
                Screen = Cast<UHangarMockScreen>(Instance->GetSubsystem<UFrontendUISubsystem>()->PushMainScreen(HangarClass));
            }
            else if (Step == 5)
            {
                Test->TestEqual(TEXT("Reopened UI preserves edited frame"), Equipment->GetCurrentLoadout().FrameId, FName(TEXT("frame.heavy")));
                // Leave the user's demonstration session on a readable comparison screen.
                if (!Click(TEXT("LeftWeapon")) || !Click(TEXT("weapon.rifle"))) { return true; }
            }
            else
            {
                FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Screenshots/HangarMock.png"), true, false);
                return true;
            }
            ++Step;
            NextTime = FPlatformTime::Seconds() + 0.75;
            return false;
        }
    private:
        FAutomationTestBase* Test;
        TWeakObjectPtr<UGameInstance> Instance;
        TWeakObjectPtr<UCommonActivatableWidgetStack> Stack;
        TWeakObjectPtr<UHangarMockScreen> Screen;
        UClass* HangarClass;
        int32 Step = 0;
        double NextTime = 0;
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHangarMockUITest, "MechaFrameLink.Hangar.MockUI",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FHangarMockUITest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        if (Context.WorldType == EWorldType::Game) { World = Context.World(); break; }
    }
    if (!TestNotNull(TEXT("Run this test in the frontend game window (-game)"), World)) { return false; }
    TArray<UUserWidget*> Roots;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, Roots, UFrontendRootWidget::StaticClass(), true);
    if (!TestEqual(TEXT("Frontend root is present"), Roots.Num(), 1)) { return false; }
    UCommonActivatableWidgetStack* Stack = Cast<UCommonActivatableWidgetStack>(Roots[0]->GetWidgetFromName(TEXT("MainStack")));
    if (!TestNotNull(TEXT("Main stack exists"), Stack)) { return false; }
    UMainMenuScreen* Menu = Cast<UMainMenuScreen>(Stack->GetActiveWidget());
    if (!TestNotNull(TEXT("Initial screen is main menu"), Menu)) { return false; }
    FClassProperty* Property = FindFProperty<FClassProperty>(UMainMenuScreen::StaticClass(), TEXT("HangarScreenClass"));
    UClass* HangarClass = Property ? Cast<UClass>(Property->GetObjectPropertyValue_InContainer(Menu)) : nullptr;
    if (!TestTrue(TEXT("Main menu targets the new mock"), HangarClass && HangarClass->IsChildOf<UHangarMockScreen>())) { return false; }
    ADD_LATENT_AUTOMATION_COMMAND(FHangarMockInteraction(this, World->GetGameInstance(), Stack, HangarClass));
    return true;
}
#endif
