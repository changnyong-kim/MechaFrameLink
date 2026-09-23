#include "Framework/FrontendPlayerController.h"

#include "Engine/GameInstance.h"
#include "UI/Core/FrontendRootWidget.h"
#include "UI/Core/FrontendUISubsystem.h"
#include "CommonActivatableWidget.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "InputKeyEventArgs.h"

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

#if !UE_BUILD_SHIPPING
    // A viewport has one override delegate. Only the primary local player owns it.
    if (GetLocalPlayer() == GameInstance->GetFirstGamePlayer())
    {
        if (UGameViewportClient* Viewport = GameInstance->GetGameViewportClient())
        {
            // PIE already owns this delegate for editor commands. Preserve its priority.
            PreviousInputKeyHandler = Viewport->OnOverrideInputKey();
            Viewport->OnOverrideInputKey().BindUObject(this, &ThisClass::HandleDebugInputKey);
            DebugInputViewport = Viewport;
        }
    }
#endif
}

void AFrontendPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if !UE_BUILD_SHIPPING
    if (UGameViewportClient* Viewport = DebugInputViewport.Get())
    {
        if (Viewport->OnOverrideInputKey().IsBoundToObject(this))
        {
            Viewport->OnOverrideInputKey() = PreviousInputKeyHandler;
        }
    }
    DebugInputViewport.Reset();
    PreviousInputKeyHandler.Unbind();
    bDebugToggleKeyDown = false;
#endif

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

#if !UE_BUILD_SHIPPING
bool AFrontendPlayerController::HandleDebugInputKey(FInputKeyEventArgs& EventArgs)
{
    if (PreviousInputKeyHandler.IsBound() && PreviousInputKeyHandler.Execute(EventArgs))
    {
        return true;
    }

    if (EventArgs.Key != EKeys::F1 || !FrontendRootWidget || !FSlateApplication::IsInitialized())
    {
        return false;
    }

    // Consume the matching release even if Ctrl was released first.
    if (EventArgs.Event == IE_Released)
    {
        const bool bHandled = bDebugToggleKeyDown;
        bDebugToggleKeyDown = false;
        return bHandled;
    }
    if (EventArgs.Event == IE_Repeat)
    {
        return bDebugToggleKeyDown;
    }

    const FModifierKeysState Modifiers = FSlateApplication::Get().GetModifierKeys();
    if (EventArgs.Event == IE_Pressed && Modifiers.IsControlDown()
        && !Modifiers.IsAltDown() && !Modifiers.IsShiftDown() && !Modifiers.IsCommandDown())
    {
        bDebugToggleKeyDown = true;
        FrontendRootWidget->DebugToggleFrontendVisibility();
        UE_LOG(LogTemp, Display, TEXT("Frontend debug visibility: %s"),
            *UEnum::GetValueAsString(FrontendRootWidget->GetVisibility()));
        return true;
    }
    return false;
}
#endif
