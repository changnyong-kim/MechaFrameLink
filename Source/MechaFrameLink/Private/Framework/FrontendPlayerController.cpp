#include "Framework/FrontendPlayerController.h"

#include "Engine/GameInstance.h"
#include "UI/Core/FrontendRootWidget.h"
#include "UI/Core/FrontendUISubsystem.h"
#include "CommonActivatableWidget.h"

void AFrontendPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (!ensureMsgf(GameInstance != nullptr, TEXT("GameInstance is unavailable.")))
    {
        return;
    }

    UFrontendUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFrontendUISubsystem>();
    if (!ensureMsgf(UISubsystem != nullptr, TEXT("FrontendUISubsystem is unavailable.")))
    {
        return;
    }

    if (!ensureMsgf(FrontendRootWidgetClass != nullptr, TEXT("FrontendRootWidgetClass is not configured.")))
    {
        return;
    }

    FrontendRootWidget = CreateWidget<UFrontendRootWidget>(this, FrontendRootWidgetClass);
    if (!ensureMsgf(FrontendRootWidget != nullptr, TEXT("Failed to create FrontendRootWidget.")))
    {
        return;
    }

    UISubsystem->RegisterRoot(FrontendRootWidget.Get());

    if (!FrontendRootWidget->AddToPlayerScreen())
    {
        UISubsystem->UnregisterRoot(FrontendRootWidget.Get());
        FrontendRootWidget = nullptr;

        UE_LOG(LogTemp, Error, TEXT("Failed to add FrontendRootWidget to player screen."));
        return;
    }

    bShowMouseCursor = true;
}

void AFrontendPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UFrontendUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFrontendUISubsystem>())
        {
            UISubsystem->UnregisterRoot(FrontendRootWidget.Get());
        }
    }

    if (FrontendRootWidget)
    {
        FrontendRootWidget->RemoveFromParent();
        FrontendRootWidget = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}