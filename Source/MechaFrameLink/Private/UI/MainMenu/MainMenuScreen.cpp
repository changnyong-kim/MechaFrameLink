#include "UI/MainMenu/MainMenuScreen.h"

#include "CommonButtonBase.h"
#include "Engine/GameInstance.h"
#include "Equipment/MechaEquipmentSubsystem.h"
#include "UI/Core/FrontendUISubsystem.h"

void UMainMenuScreen::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (ensureMsgf(SortieButton != nullptr, TEXT("MainMenuScreen requires SortieButton.")))
    {
        SortieButton->OnClicked().AddUObject(this, &ThisClass::HandleSortieClicked);
    }

    if (ensureMsgf(HangarButton != nullptr, TEXT("MainMenuScreen requires HangarButton.")))
    {
        HangarButton->OnClicked().AddUObject(this, &ThisClass::HandleHangarClicked);
    }
}

UWidget* UMainMenuScreen::NativeGetDesiredFocusTarget() const
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[UI] FocusTarget: %s"),
        *GetNameSafe(SortieButton.Get()));

    return SortieButton.Get();
}

void UMainMenuScreen::HandleSortieClicked()
{
    UGameInstance* Instance = GetGameInstance();
    UMechaEquipmentSubsystem* Equipment = Instance ? Instance->GetSubsystem<UMechaEquipmentSubsystem>() : nullptr;
    FText Error;
    if (!Equipment)
    {
        Error = NSLOCTEXT("MechaMainMenu", "NoEquipment", "Equipment system is unavailable.");
    }
    if (!Equipment || !Equipment->CompleteSetup(Error))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] Lobby entry blocked: %s"), *Error.ToString());
        OnLobbyEntryFailed(Error);
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[UI] MainMenu: Active=%d, SupportsFocus=%d, HasPlayer=%d"),
        IsActivated() ? 1 : 0,
        SupportsActivationFocus() ? 1 : 0,
        GetOwningLocalPlayer() != nullptr ? 1 : 0);

    PushScreen(LobbyScreenClass, TEXT("Lobby"));
}

void UMainMenuScreen::HandleHangarClicked()
{
    PushScreen(HangarScreenClass, TEXT("Hangar"));
}

void UMainMenuScreen::PushScreen(
    TSubclassOf<UFrontendActivatableWidget> ScreenClass,
    const TCHAR* ScreenName)
{
    if (!ensureMsgf(ScreenClass != nullptr, TEXT("%sScreenClass is not configured."), ScreenName))
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (!ensureMsgf(GameInstance != nullptr, TEXT("MainMenu has no GameInstance.")))
    {
        return;
    }

    UFrontendUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFrontendUISubsystem>();
    if (!ensureMsgf(UISubsystem != nullptr, TEXT("FrontendUISubsystem is unavailable.")))
    {
        return;
    }

    UISubsystem->PushMainScreen(ScreenClass);
}
