#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "FrontendRootWidget.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

UCLASS()
class MECHAFRAMELINK_API UFrontendRootWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    UCommonActivatableWidget* PushMainScreen(TSubclassOf<UCommonActivatableWidget> ScreenClass);

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> MainStack;
};