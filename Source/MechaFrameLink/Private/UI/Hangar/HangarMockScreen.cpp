#include "UI/Hangar/HangarMockScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Equipment/MechaEquipmentLibrary.h"
#include "Equipment/MechaEquipmentSubsystem.h"

namespace
{
    constexpr EMechaEquipmentSlot AllSlots[] = {
        EMechaEquipmentSlot::Frame, EMechaEquipmentSlot::LeftWeapon, EMechaEquipmentSlot::RightWeapon,
        EMechaEquipmentSlot::LeftShoulder, EMechaEquipmentSlot::RightShoulder, EMechaEquipmentSlot::Back
    };

    FString SlotLabel(EMechaEquipmentSlot EquipmentSlot)
    {
        return StaticEnum<EMechaEquipmentSlot>()->GetNameStringByValue(static_cast<int64>(EquipmentSlot));
    }

    FString StatLine(const TCHAR* Label, const FMechaPartStats& Stats)
    {
        return FString::Printf(TEXT("%s\nAP %.0f\nAttack %.0f\nDefense %.0f\nWeight %.0f\n"),
            Label, Stats.AP, Stats.Attack, Stats.Defense, Stats.Weight);
    }
}

void UHangarMockButton::SetAction(FSimpleDelegate InAction)
{
    Action = MoveTemp(InAction);
    OnClicked.AddUniqueDynamic(this, &ThisClass::HandlePressed);
}

void UHangarMockButton::HandlePressed()
{
    const FSimpleDelegate Callback = Action;
    Callback.ExecuteIfBound();
}

UTextBlock* UHangarMockScreen::MakeText(const FString& Text, int32 Size)
{
    UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
    Label->SetText(FText::FromString(Text));
    FSlateFontInfo Font = Label->GetFont();
    Font.Size = Size;
    Label->SetFont(Font);
    Label->SetAutoWrapText(true);
    Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    return Label;
}

UHangarMockButton* UHangarMockScreen::MakeButton(const FString& Text, FSimpleDelegate Action)
{
    UHangarMockButton* Button = WidgetTree->ConstructWidget<UHangarMockButton>();
    Button->SetBackgroundColor(FLinearColor(0.14f, 0.17f, 0.21f));
    Button->AddChild(MakeText(Text));
    Button->SetAction(MoveTemp(Action));
    return Button;
}

TSharedRef<SWidget> UHangarMockScreen::RebuildWidget()
{
    if (WidgetTree && !WidgetTree->RootWidget)
    {
        UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
        Background->SetBrushColor(FLinearColor(0.025f, 0.03f, 0.04f, 1.0f));
        Background->SetPadding(FMargin(24));
        WidgetTree->RootWidget = Background;
        UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>();
        Background->AddChild(Layout);
        Layout->AddChildToVerticalBox(MakeText(TEXT("MechaFrameLink / HANGAR"), 24));
        Layout->AddChildToVerticalBox(MakeText(TEXT("1. Select slot   2. Select candidate   3. Equip / Unequip   4. Complete setup")))->SetPadding(FMargin(0, 8));

        UHorizontalBox* Columns = WidgetTree->ConstructWidget<UHorizontalBox>();
        Layout->AddChildToVerticalBox(Columns)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        auto AddColumn = [this, Columns](const TCHAR* Heading, float Fill)
        {
            UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
            UHorizontalBoxSlot* ColumnSlot = Columns->AddChildToHorizontalBox(Column);
            FSlateChildSize Size(ESlateSizeRule::Fill);
            Size.Value = Fill;
            ColumnSlot->SetSize(Size);
            ColumnSlot->SetPadding(FMargin(0, 8, 16, 8));
            Column->AddChildToVerticalBox(MakeText(Heading, 20))->SetPadding(FMargin(0, 0, 0, 10));
            return Column;
        };
        UVerticalBox* SlotColumn = AddColumn(TEXT("SLOTS / EQUIPPED"), 1.1f);
        UScrollBox* SlotScroll = WidgetTree->ConstructWidget<UScrollBox>();
        SlotColumn->AddChildToVerticalBox(SlotScroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        SlotsPanel = WidgetTree->ConstructWidget<UVerticalBox>();
        SlotScroll->AddChild(SlotsPanel);

        UVerticalBox* PartColumn = AddColumn(TEXT("AVAILABLE PARTS"), 1.1f);
        UScrollBox* PartScroll = WidgetTree->ConstructWidget<UScrollBox>();
        PartColumn->AddChildToVerticalBox(PartScroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        PartsPanel = WidgetTree->ConstructWidget<UVerticalBox>();
        PartScroll->AddChild(PartsPanel);

        UVerticalBox* StatsColumn = AddColumn(TEXT("STATS / COMPARISON"), 0.8f);
        UScrollBox* StatsScroll = WidgetTree->ConstructWidget<UScrollBox>();
        StatsColumn->AddChildToVerticalBox(StatsScroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        StatsText = MakeText(TEXT("Loading equipment..."));
        StatsScroll->AddChild(StatsText);

        StatusText = MakeText(TEXT("Select a slot and a candidate."));
        Layout->AddChildToVerticalBox(StatusText)->SetPadding(FMargin(0, 10));
        UHorizontalBox* Actions = WidgetTree->ConstructWidget<UHorizontalBox>();
        Layout->AddChildToVerticalBox(Actions);
        auto AddAction = [this, Actions](const TCHAR* Label, FSimpleDelegate Callback)
        {
            UHangarMockButton* Button = MakeButton(Label, MoveTemp(Callback));
            Actions->AddChildToHorizontalBox(Button)->SetPadding(FMargin(0, 0, 12, 0));
            return Button;
        };
        EquipButton = AddAction(TEXT("Equip candidate"), FSimpleDelegate::CreateUObject(this, &ThisClass::ApplyCandidate));
        UnequipButton = AddAction(TEXT("Unequip slot"), FSimpleDelegate::CreateUObject(this, &ThisClass::Unequip));
        AddAction(TEXT("Clear comparison"), FSimpleDelegate::CreateWeakLambda(this, [this]
        {
            bHasCandidate = false;
            Status = FText::FromString(TEXT("Comparison cleared. Equipped parts are unchanged."));
            Refresh();
        }));
        CompleteButton = AddAction(TEXT("Complete setup > Lobby"), FSimpleDelegate::CreateUObject(this, &ThisClass::OpenLobby));
        AddAction(TEXT("Back to menu"), FSimpleDelegate::CreateWeakLambda(this, [this] { DeactivateWidget(); }));
    }
    return Super::RebuildWidget();
}

void UHangarMockScreen::NativeOnActivated()
{
    bHasCandidate = false;
    Status = FText::FromString(TEXT("Select a slot and a candidate. Frame can be replaced, not removed."));
    Super::NativeOnActivated();
    Refresh();
}

UWidget* UHangarMockScreen::NativeGetDesiredFocusTarget() const
{
    return SelectedSlotButton;
}

void UHangarMockScreen::HandleLoadoutChanged(const FMechaLoadout& Loadout)
{
    Super::HandleLoadoutChanged(Loadout);
    Refresh();
}

void UHangarMockScreen::Refresh()
{
    if (!SlotsPanel || !PartsPanel || !StatsText)
    {
        return;
    }
    const bool bReady = Equipment && Equipment->IsEquipmentReady();
    EquipButton->SetIsEnabled(false);
    UnequipButton->SetIsEnabled(bReady && SelectedSlot != EMechaEquipmentSlot::Frame);
    CompleteButton->SetIsEnabled(bReady);
    if (!bReady)
    {
        StatusText->SetText(FText::FromString(TEXT("Equipment unavailable. Return to menu and retry; check the part catalog.")));
        return;
    }
    SlotsPanel->ClearChildren();
    PartsPanel->ClearChildren();
    const FMechaLoadout Loadout = Equipment->GetCurrentLoadout();
    for (EMechaEquipmentSlot EquipmentSlot : AllSlots)
    {
        const FName Id = UMechaEquipmentLibrary::GetEquippedPartId(Loadout, EquipmentSlot);
        const UMechaPartDefinition* Part = Equipment->GetCatalog()->FindPart(Id);
        const FString Label = FString::Printf(TEXT("%s%s\n%s | %s"), EquipmentSlot == SelectedSlot ? TEXT("> ") : TEXT(""),
            *SlotLabel(EquipmentSlot), *Id.ToString(), Part ? *Part->DisplayName.ToString() : TEXT("Empty"));
        UHangarMockButton* Button = MakeButton(Label, FSimpleDelegate::CreateWeakLambda(this, [this, EquipmentSlot] { SelectSlot(EquipmentSlot); }));
        SlotsPanel->AddChildToVerticalBox(Button)->SetPadding(FMargin(0, 0, 0, 8));
        if (EquipmentSlot == SelectedSlot)
        {
            SelectedSlotButton = Button;
        }
    }
    const FName EquippedId = UMechaEquipmentLibrary::GetEquippedPartId(Loadout, SelectedSlot);
    for (UMechaPartDefinition* Part : Equipment->GetPartsForSlot(SelectedSlot))
    {
        const FName Id = Part->PartId;
        const FString Label = FString::Printf(TEXT("%s%s\n%s%s"), bHasCandidate && CandidateId == Id ? TEXT("> ") : TEXT(""),
            *Id.ToString(), *Part->DisplayName.ToString(), EquippedId == Id ? TEXT(" [Equipped]") : TEXT(""));
        PartsPanel->AddChildToVerticalBox(MakeButton(Label,
            FSimpleDelegate::CreateWeakLambda(this, [this, Id] { SelectCandidate(Id); })))->SetPadding(FMargin(0, 0, 0, 8));
    }
    if (SelectedSlot != EMechaEquipmentSlot::Frame)
    {
        PartsPanel->AddChildToVerticalBox(MakeButton(TEXT("None / Preview unequip"),
            FSimpleDelegate::CreateWeakLambda(this, [this] { SelectCandidate(NAME_None); })));
    }
    FString Stats = StatLine(TEXT("CURRENT"), Equipment->GetCurrentStats());
    if (bHasCandidate)
    {
        FMechaEquipmentComparison Comparison;
        FText Error;
        if (Equipment->CompareCandidate(SelectedSlot, CandidateId, Comparison, Error))
        {
            Stats += TEXT("\n") + StatLine(TEXT("AFTER CHANGE"), Comparison.PreviewStats);
            Stats += FString::Printf(TEXT("\nDELTA (after - current)\nAP %+.0f\nAttack %+.0f\nDefense %+.0f\nWeight %+.0f\n\nLower weight is better."),
                Comparison.Delta.AP, Comparison.Delta.Attack, Comparison.Delta.Defense, Comparison.Delta.Weight);
            EquipButton->SetIsEnabled(!CandidateId.IsNone() && CandidateId != EquippedId);
        }
        else
        {
            Status = Error;
        }
    }
    else
    {
        Stats += TEXT("\nChoose a candidate to compare.\nSelection does not equip the part.");
    }
    StatsText->SetText(FText::FromString(Stats));
    StatusText->SetText(Status);
}

void UHangarMockScreen::SelectSlot(EMechaEquipmentSlot EquipmentSlot)
{
    SelectedSlot = EquipmentSlot;
    bHasCandidate = false;
    Status = FText::FromString(TEXT("Selected slot: ") + SlotLabel(EquipmentSlot));
    Refresh();
}

void UHangarMockScreen::SelectCandidate(FName PartId)
{
    CandidateId = PartId;
    bHasCandidate = true;
    Status = FText::FromString(TEXT("Preview only: ") + PartId.ToString());
    Refresh();
}

void UHangarMockScreen::ApplyCandidate()
{
    FText Error;
    if (Equipment && bHasCandidate && Equipment->EquipPart(SelectedSlot, CandidateId, Error))
    {
        Status = FText::FromString(TEXT("Equipped: ") + CandidateId.ToString());
        bHasCandidate = false;
    }
    else { Status = Error; }
    Refresh();
}

void UHangarMockScreen::Unequip()
{
    FText Error;
    if (Equipment && Equipment->UnequipPart(SelectedSlot, Error))
    {
        Status = FText::FromString(TEXT("Unequipped: ") + SlotLabel(SelectedSlot));
        bHasCandidate = false;
    }
    else { Status = Error; }
    Refresh();
}

void UHangarMockScreen::OpenLobby()
{
    FText Error;
    if (!CompleteSetupAndOpenLobby(Error))
    {
        Status = Error;
        Refresh();
    }
}
