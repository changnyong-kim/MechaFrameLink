#include "UI/Common/FrontendButtonBase.h"

#include "Components/TextBlock.h"

void UFrontendButtonBase::SetButtonText(const FText& InButtonText)
{
    ButtonText = InButtonText;
    RefreshButtonText();
}

void UFrontendButtonBase::NativePreConstruct()
{
    Super::NativePreConstruct();
    RefreshButtonText();
}

void UFrontendButtonBase::RefreshButtonText()
{
    if (LabelText)
    {
        LabelText->SetText(ButtonText);
    }
}
