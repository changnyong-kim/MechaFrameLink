#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FrontendPlayerController.generated.h"

class UFrontendRootWidget;
class UCommonActivatableWidget;

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
    UPROPERTY(Transient)
    TObjectPtr<UFrontendRootWidget> FrontendRootWidget;
};