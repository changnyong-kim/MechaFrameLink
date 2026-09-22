#include "UI/Core/FrontendActivatableWidget.h"

TOptional<FUIInputConfig> UFrontendActivatableWidget::GetDesiredInputConfig() const
{
    UE_LOG(LogTemp, Warning, TEXT("[UI] InputConfig: %s"), *GetName());

    return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, false);
}