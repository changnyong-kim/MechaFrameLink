#pragma once

#include "CoreMinimal.h"
#include "UI/Core/FrontendActivatableWidget.h"
#include "MainMenuScreen.generated.h"

class UCommonButtonBase;

UCLASS(Abstract, Blueprintable)
class MECHAFRAMELINK_API UMainMenuScreen : public UFrontendActivatableWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Mecha|Lobby")
    void OnLobbyEntryFailed(const FText& Error);

private:
    void HandleSortieClicked();
    void HandleHangarClicked();
    void PushScreen(TSubclassOf<UFrontendActivatableWidget> ScreenClass, const TCHAR* ScreenName);

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    TObjectPtr<UCommonButtonBase> SortieButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    TObjectPtr<UCommonButtonBase> HangarButton;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UFrontendActivatableWidget> LobbyScreenClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UFrontendActivatableWidget> HangarScreenClass;
};
