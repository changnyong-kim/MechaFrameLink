#pragma once

#include "CoreMinimal.h"
#include "UI/Core/FrontendActivatableWidget.h"
#include "Equipment/MechaLoadout.h"
#include "HangarScreen.generated.h"

class UMechaEquipmentSubsystem;

/** Blueprint layout with a session-owned equipment model and validated lobby transition. */
UCLASS(Abstract, Blueprintable)
class MECHAFRAMELINK_API UHangarScreen : public UFrontendActivatableWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Mecha|Hangar")
    bool CompleteSetupAndOpenLobby(FText& OutError);

protected:
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "Mecha|Hangar")
    TObjectPtr<UMechaEquipmentSubsystem> Equipment;

    UPROPERTY(EditDefaultsOnly, Category = "Mecha|Hangar")
    TSubclassOf<UFrontendActivatableWidget> LobbyScreenClass;

    /** Read Equipment for initial display and after confirmed changes. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Mecha|Hangar")
    void OnEquipmentUpdated(const FMechaLoadout& Loadout);

    UFUNCTION(BlueprintImplementableEvent, Category = "Mecha|Hangar")
    void OnHangarInitializationFailed(const FText& Error);

protected:
    UFUNCTION()
    virtual void HandleLoadoutChanged(const FMechaLoadout& Loadout);
};
