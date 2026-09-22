#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "UI/Hangar/HangarScreen.h"
#include "HangarMockScreen.generated.h"

class UTextBlock;
class UVerticalBox;

/** Routes dynamically created UMG buttons to a native callback. */
UCLASS()
class MECHAFRAMELINK_API UHangarMockButton : public UButton
{
    GENERATED_BODY()
public:
    void SetAction(FSimpleDelegate InAction);
private:
    UFUNCTION()
    void HandlePressed();
    FSimpleDelegate Action;
};

/** Text-only, executable hangar. All layout is created in C++; no art assets required. */
UCLASS(Blueprintable)
class MECHAFRAMELINK_API UHangarMockScreen : public UHangarScreen
{
    GENERATED_BODY()
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeOnActivated() override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;
    virtual void HandleLoadoutChanged(const FMechaLoadout& Loadout) override;

private:
    UTextBlock* MakeText(const FString& Text, int32 Size = 16);
    UHangarMockButton* MakeButton(const FString& Text, FSimpleDelegate Action);
    void Refresh();
    void SelectSlot(EMechaEquipmentSlot EquipmentSlot);
    void SelectCandidate(FName PartId);
    void ApplyCandidate();
    void Unequip();
    void OpenLobby();

    EMechaEquipmentSlot SelectedSlot = EMechaEquipmentSlot::Frame;
    FName CandidateId;
    bool bHasCandidate = false;
    FText Status;

    UPROPERTY(Transient) TObjectPtr<UVerticalBox> SlotsPanel;
    UPROPERTY(Transient) TObjectPtr<UVerticalBox> PartsPanel;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> StatsText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> StatusText;
    UPROPERTY(Transient) TObjectPtr<UButton> EquipButton;
    UPROPERTY(Transient) TObjectPtr<UButton> UnequipButton;
    UPROPERTY(Transient) TObjectPtr<UButton> CompleteButton;
    UPROPERTY(Transient) TObjectPtr<UButton> SelectedSlotButton;
};
