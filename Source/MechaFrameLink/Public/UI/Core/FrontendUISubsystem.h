#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FrontendUISubsystem.generated.h"

class UCommonActivatableWidget;
class UFrontendRootWidget;

UCLASS()
class MECHAFRAMELINK_API UFrontendUISubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void RegisterRoot(UFrontendRootWidget* InRoot);
    void UnregisterRoot(UFrontendRootWidget* InRoot);

    UCommonActivatableWidget* PushMainScreen(TSubclassOf<UCommonActivatableWidget> ScreenClass);

private:
    UPROPERTY(Transient)
    TWeakObjectPtr<UFrontendRootWidget> RootWidget;
};