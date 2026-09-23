#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "FrontendPlayerController.generated.h"

class UFrontendRootWidget;
class UCommonActivatableWidget;
struct FInputKeyEventArgs;

UCLASS()
class MECHAFRAMELINK_API AFrontendPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UFrontendRootWidget> FrontendRootWidgetClass;

private:
#if !UE_BUILD_SHIPPING
    bool HandleDebugInputKey(FInputKeyEventArgs& EventArgs);
    TWeakObjectPtr<UGameViewportClient> DebugInputViewport;
    FOverrideInputKeyHandler PreviousInputKeyHandler;
    bool bDebugToggleKeyDown = false;
#endif

    UPROPERTY(Transient)
    TObjectPtr<UFrontendRootWidget> FrontendRootWidget;
};
