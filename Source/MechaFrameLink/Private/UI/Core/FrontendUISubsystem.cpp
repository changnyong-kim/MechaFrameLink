#include "UI/Core/FrontendUISubsystem.h"

#include "CommonActivatableWidget.h"
#include "UI/Core/FrontendRootWidget.h"

void UFrontendUISubsystem::RegisterRoot(UFrontendRootWidget* InRoot)
{
    if (!ensureMsgf(IsValid(InRoot), TEXT("Cannot register an invalid UI root.")))
    {
        return;
    }

    RootWidget = InRoot;
}

void UFrontendUISubsystem::UnregisterRoot(UFrontendRootWidget* InRoot)
{
    if (RootWidget.Get() == InRoot)
    {
        RootWidget.Reset();
    }
}

UCommonActivatableWidget* UFrontendUISubsystem::PushMainScreen(TSubclassOf<UCommonActivatableWidget> ScreenClass)
{
    UFrontendRootWidget* Root = RootWidget.Get();

    if (!ensureMsgf(Root != nullptr, TEXT("Frontend UI root is not registered.")))
    {
        return nullptr;
    }

    return Root->PushMainScreen(ScreenClass);
}