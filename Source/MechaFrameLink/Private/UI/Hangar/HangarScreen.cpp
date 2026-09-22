#include "UI/Hangar/HangarScreen.h"

#include "Engine/GameInstance.h"
#include "Equipment/MechaEquipmentSubsystem.h"
#include "UI/Core/FrontendUISubsystem.h"

void UHangarScreen::NativeOnActivated()
{
    Super::NativeOnActivated();
    UGameInstance* Instance = GetGameInstance();
    Equipment = Instance ? Instance->GetSubsystem<UMechaEquipmentSubsystem>() : nullptr;
    FText Error;
    if (!Equipment)
    {
        Error = NSLOCTEXT("MechaHangar", "NoEquipment", "Equipment system is unavailable.");
    }
    if (!Equipment || !Equipment->EnsureEquipmentReady(Error))
    {
        OnHangarInitializationFailed(Error);
        return;
    }
    Equipment->OnLoadoutChanged.AddUniqueDynamic(this, &ThisClass::HandleLoadoutChanged);
    HandleLoadoutChanged(Equipment->GetCurrentLoadout());
}

void UHangarScreen::NativeOnDeactivated()
{
    if (Equipment)
    {
        Equipment->OnLoadoutChanged.RemoveDynamic(this, &ThisClass::HandleLoadoutChanged);
    }
    Super::NativeOnDeactivated();
}

void UHangarScreen::HandleLoadoutChanged(const FMechaLoadout& Loadout)
{
    OnEquipmentUpdated(Loadout);
}

bool UHangarScreen::CompleteSetupAndOpenLobby(FText& OutError)
{
    UGameInstance* Instance = GetGameInstance();
    UFrontendUISubsystem* UI = Instance ? Instance->GetSubsystem<UFrontendUISubsystem>() : nullptr;
    if (!IsActivated() || !Equipment || !UI || !LobbyScreenClass)
    {
        OutError = NSLOCTEXT("MechaHangar", "LobbyUnavailable", "Activate the hangar and configure LobbyScreenClass before continuing.");
        return false;
    }
    if (!Equipment->CompleteSetup(OutError))
    {
        return false;
    }
    if (!UI->PushMainScreen(LobbyScreenClass))
    {
        OutError = NSLOCTEXT("MechaHangar", "LobbyOpenFailed", "Could not open the lobby screen.");
        return false;
    }
    OutError = FText::GetEmpty();
    return true;
}
