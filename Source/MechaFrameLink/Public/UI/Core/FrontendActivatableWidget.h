#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Input/UIActionBindingHandle.h"
#include "FrontendActivatableWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class MECHAFRAMELINK_API UFrontendActivatableWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
};