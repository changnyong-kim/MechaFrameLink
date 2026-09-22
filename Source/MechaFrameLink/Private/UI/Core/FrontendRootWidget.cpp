#include "UI/Core/FrontendRootWidget.h"

#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

UCommonActivatableWidget* UFrontendRootWidget::PushMainScreen(TSubclassOf<UCommonActivatableWidget> ScreenClass)
{
    if (!ensureMsgf(MainStack != nullptr, TEXT("FrontendRootWidget requires MainStack.")))
    {
        return nullptr;
    }

    if (!ensureMsgf(ScreenClass != nullptr, TEXT("ScreenClass is not configured.")))
    {
        return nullptr;
    }

    return MainStack->AddWidget<UCommonActivatableWidget>(ScreenClass);
}