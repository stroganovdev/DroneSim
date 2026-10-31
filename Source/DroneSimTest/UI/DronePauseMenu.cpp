#include "DronePauseMenu.h"
#include "../DroneTestPlayerController.h"
#include "../Settings/DroneFlightSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "Components/ScaleBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SpinBox.h"
#include "Components/CheckBox.h"
#include "DroneSimulatorInput/Public/DroneInputSubsystem.h"
#include "InputCoreTypes.h"

namespace
{
    UTextBlock* Text(UWidgetTree* Tree, const FString& Label, int32 Size = 18)
    {
        auto* T = Tree->ConstructWidget<UTextBlock>();
        T->SetText(FText::FromString(Label));
        auto Font = T->GetFont(); Font.Size = Size; T->SetFont(Font);
        T->SetColorAndOpacity(FSlateColor(FLinearColor(0.90f, 0.94f, 1.f)));
        T->SetAutoWrapText(true);
        return T;
    }
    void StyleCombo(UComboBoxString* Combo)
    {
        auto Rows = Combo->GetItemStyle();
        Rows.SetTextColor(FSlateColor(FLinearColor(0.90f,0.94f,1.f)));
        Rows.SetSelectedTextColor(FSlateColor(FLinearColor::White));
        Combo->SetItemStyle(Rows);
    }
    FString AxisLabel(FKey Key)
    {
        if (Key == EKeys::Gamepad_LeftX) return TEXT("Левый стик — горизонталь");
        if (Key == EKeys::Gamepad_LeftY) return TEXT("Левый стик — вертикаль");
        if (Key == EKeys::Gamepad_RightX) return TEXT("Правый стик — горизонталь");
        if (Key == EKeys::Gamepad_RightY) return TEXT("Правый стик — вертикаль");
        if (Key == EKeys::Gamepad_LeftTriggerAxis) return TEXT("LT — левый курок");
        return TEXT("RT — правый курок");
    }
    UButton* Button(UWidgetTree* Tree, const FString& Label, FName Name = NAME_None)
    {
        auto* B = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
        B->SetBackgroundColor(FLinearColor(0.10f, 0.16f, 0.23f));
        auto* Padding = Tree->ConstructWidget<UBorder>();
        Padding->SetBrushColor(FLinearColor::Transparent);
        Padding->SetPadding(FMargin(14, 9));
        Padding->SetContent(Text(Tree, Label, 16));
        B->AddChild(Padding);
        return B;
    }
}
void UDroneSettingBinding::Changed(float Value)
{
    if (Settings) Settings->SetDraftNumber(Key, Value);
}
void UDroneGamepadAxisBinding::Changed(FString Selected, ESelectInfo::Type SelectionType)
{
    if (!Settings) return;
    auto Value = Settings->GetDraft().Gamepad;
    for (FKey Key : FDroneGamepadSettings::AxisKeys()) if (AxisLabel(Key) == Selected)
    {
        if (Axis == EDroneInputAxis::Throttle) Value.Throttle = Key;
        if (Axis == EDroneInputAxis::Yaw) Value.Yaw = Key;
        if (Axis == EDroneInputAxis::Pitch) Value.Pitch = Key;
        if (Axis == EDroneInputAxis::Roll) Value.Roll = Key;
    }
    Settings->SetDraftGamepad(Value);
}
void UDronePauseMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    SetIsFocusable(true);
    Settings = GetGameInstance()->GetSubsystem<UDroneSettingsSubsystem>();
    if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this);
    auto* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;
    auto* Shade = WidgetTree->ConstructWidget<UBorder>();
    Shade->SetBrushColor(FLinearColor(0.005f, 0.012f, 0.025f, 0.88f));
    auto* ShadeSlot = Canvas->AddChildToCanvas(Shade);
    ShadeSlot->SetAnchors(FAnchors(0, 0, 1, 1)); ShadeSlot->SetOffsets(FMargin(0));
    auto* Size = WidgetTree->ConstructWidget<USizeBox>();
    Size->SetWidthOverride(1000); Size->SetHeightOverride(640);
    auto* Scale = WidgetTree->ConstructWidget<UScaleBox>();
    Scale->SetStretch(EStretch::ScaleToFit); Scale->SetStretchDirection(EStretchDirection::DownOnly);
    auto* PanelSlot = Canvas->AddChildToCanvas(Scale);
    PanelSlot->SetAnchors(FAnchors(0, 0, 1, 1)); PanelSlot->SetOffsets(FMargin(20));
    auto* ScaleSlot = Cast<UScaleBoxSlot>(Scale->AddChild(Size));
    ScaleSlot->SetHorizontalAlignment(HAlign_Center); ScaleSlot->SetVerticalAlignment(VAlign_Center);
    auto* Panel = WidgetTree->ConstructWidget<UBorder>();
    Panel->SetPadding(FMargin(26)); Panel->SetBrushColor(FLinearColor(0.025f, 0.045f, 0.070f, 1)); Size->SetContent(Panel);
    auto* Layout = WidgetTree->ConstructWidget<UVerticalBox>(); Panel->SetContent(Layout);
    Layout->AddChildToVerticalBox(Text(WidgetTree, TEXT("ПАУЗА  /  НАСТРОЙКИ"), 27))->SetPadding(FMargin(0, 0, 0, 5));
    auto* Subtitle = Text(WidgetTree, (Cast<ADroneTestPlayerController>(GetOwningPlayer())->IsWalkingMode() ? TEXT("ПРОГУЛКА     •     TAB — продолжить") : TEXT("METEOR75 PRO     •     Полёт приостановлен     •     TAB — продолжить")), 14);
    Subtitle->SetColorAndOpacity(FSlateColor(FLinearColor(0.40f, 0.72f, 0.82f)));
    Layout->AddChildToVerticalBox(Subtitle)->SetPadding(FMargin(0, 0, 0, 20));
    auto* Tabs = WidgetTree->ConstructWidget<UHorizontalBox>(); Layout->AddChildToVerticalBox(Tabs)->SetPadding(FMargin(0, 0, 0, 18));
    auto* B = Button(WidgetTree, TEXT("Управление"), TEXT("ControlsTab")); B->OnClicked.AddDynamic(this, &UDronePauseMenu::Controls); Tabs->AddChildToHorizontalBox(B)->SetPadding(FMargin(0, 0, 8, 0));
    B = Button(WidgetTree, TEXT("Коптер"), TEXT("AircraftTab")); B->OnClicked.AddDynamic(this, &UDronePauseMenu::Aircraft); Tabs->AddChildToHorizontalBox(B)->SetPadding(FMargin(0, 0, 8, 0));
    B = Button(WidgetTree, TEXT("Ввод"), TEXT("RadioTab")); B->OnClicked.AddDynamic(this, &UDronePauseMenu::Radio); Tabs->AddChildToHorizontalBox(B)->SetPadding(FMargin(0, 0, 8, 0));
    B = Button(WidgetTree, TEXT("Камера"), TEXT("CameraTab")); B->OnClicked.AddDynamic(this, &UDronePauseMenu::Camera); Tabs->AddChildToHorizontalBox(B);
    auto* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
    auto* ScrollSlot = Layout->AddChildToVerticalBox(Scroll); ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    Content = WidgetTree->ConstructWidget<UVerticalBox>(); Scroll->AddChild(Content);
    Status = Text(WidgetTree, TEXT(""), 14); Status->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.8f, 0.9f)));
    Layout->AddChildToVerticalBox(Status)->SetPadding(FMargin(0, 12, 0, 12));
    auto* Footer = WidgetTree->ConstructWidget<UHorizontalBox>(); Layout->AddChildToVerticalBox(Footer);
    B = Button(WidgetTree, TEXT("Продолжить"), TEXT("ResumeButton")); B->OnClicked.AddDynamic(this, &UDronePauseMenu::ResumeFlight); Footer->AddChildToHorizontalBox(B)->SetPadding(FMargin(0, 0, 8, 0));
    B = Button(WidgetTree, TEXT("Применить и продолжить"), TEXT("ApplyButton")); B->SetBackgroundColor(FLinearColor(0.05f, 0.42f, 0.52f)); B->OnClicked.AddDynamic(this, &UDronePauseMenu::ApplyAndResume); Footer->AddChildToHorizontalBox(B)->SetPadding(FMargin(0, 0, 8, 0));
    B = Button(WidgetTree, TEXT("Сброс Meteor75"), TEXT("ResetButton")); B->OnClicked.AddDynamic(this, &UDronePauseMenu::ResetMeteorDraft); Footer->AddChildToHorizontalBox(B)->SetPadding(FMargin(0, 0, 8, 0));
    B = Button(WidgetTree, TEXT("На старт"), TEXT("RestartButton")); B->OnClicked.AddDynamic(this, &UDronePauseMenu::RestartFlight); Footer->AddChildToHorizontalBox(B);
    SelectSection(0);
}
void UDronePauseMenu::SelectSection(int32 Section)
{
    if (!Content || !Settings) return;
    ActiveSection = FMath::Clamp(Section, 0, 3);
    Content->ClearChildren();
    AxisBindings.Empty(); SourcePicker = nullptr; SourceIds.Empty(); Bindings.Empty(); DeviceReadout = nullptr;
    const FString Names[] = {TEXT("Управление"), TEXT("Физика коптера"), TEXT("Устройства управления"), TEXT("Камера")};
    Content->AddChildToVerticalBox(Text(WidgetTree, Names[ActiveSection], 23))->SetPadding(FMargin(0, 0, 0, 8));
    if (ActiveSection == 0)
    {
        auto* PC=Cast<ADroneTestPlayerController>(GetOwningPlayer());
        auto* Switch=Button(WidgetTree,PC && PC->IsWalkingMode() ? TEXT("Вернуться к коптеру") : TEXT("Гулять от третьего лица"),TEXT("WalkModeButton"));
        Switch->OnClicked.AddDynamic(this,&UDronePauseMenu::SwitchPlayMode);
        Content->AddChildToVerticalBox(Switch)->SetPadding(FMargin(0,0,0,8));
        Content->AddChildToVerticalBox(Text(WidgetTree,TEXT("F / Xbox Menu — прогулка или полёт. WASD / левый стик — ходьба; мышь / правый стик — камера; Space / A — прыжок. При прогулке коптер возвращается на старт и замирает. Переключение отменяет несохранённые настройки."),14))->SetPadding(FMargin(0,0,0,12));
        auto* Mode = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), TEXT("AcroCheckBox"));
        Mode->SetIsChecked(Settings->GetDraft().Acro);
        Mode->SetContent(Text(WidgetTree, TEXT("Acro — сохранять наклон. Выключено: Angle — выравнивать коптер."), 16));
        Mode->OnCheckStateChanged.AddDynamic(this, &UDronePauseMenu::FlightModeChanged);
        Content->AddChildToVerticalBox(Mode)->SetPadding(FMargin(0, 0, 0, 14));
        Content->AddChildToVerticalBox(Text(WidgetTree, TEXT("Actual Rates: максимум определяет скорость вращения, Expo смягчает отклик около центра. В Angle крен и тангаж задают наклон до 50°."), 14))->SetPadding(FMargin(0, 0, 0, 12));
    }
    if (ActiveSection == 1)
        Content->AddChildToVerticalBox(Text(WidgetTree, TEXT("Эти изменения применяются с возвратом на старт. Рама, моторы и винты: профиль Meteor75 Pro; параметры меняют только текущий экземпляр коптера."), 14))->SetPadding(FMargin(0, 0, 0, 12));
    if (ActiveSection == 2)
    {
        auto* Input = GetGameInstance()->GetSubsystem<UDroneInputSubsystem>();
        const auto Draft = Settings->GetDraft();
        Content->AddChildToVerticalBox(Text(WidgetTree, TEXT("Источник управления (вступит в силу после «Применить»)"), 16));
        SourcePicker = WidgetTree->ConstructWidget<UComboBoxString>();
        StyleCombo(SourcePicker);
        if (Input) for (const auto& Source : Input->GetControlSources())
        {
            SourceIds.Add(Source.Id); SourcePicker->AddOption(Source.Label.ToString());
            if (Source.Id == Draft.ControlSource) SourcePicker->SetSelectedOption(Source.Label.ToString());
        }
        SourcePicker->OnSelectionChanged.AddDynamic(this, &UDronePauseMenu::SourceChanged);
        Content->AddChildToVerticalBox(SourcePicker)->SetPadding(FMargin(0,4,0,12));
        DeviceReadout = Text(WidgetTree, TEXT("Ожидание данных…"), 16);
        Content->AddChildToVerticalBox(DeviceReadout)->SetPadding(FMargin(0, 0, 0, 20));
        if (Draft.ControlSource == TEXT("Gamepad"))
        {
            Content->AddChildToVerticalBox(Text(WidgetTree, TEXT("Как на FPV-пульте: левый стик вверх/вниз — газ, влево/вправо — поворот. Правый — тангаж/крен. Газ: внизу 0%, в центре 50%, вверху 100%. Пружина Xbox возвращает газ к 50%: для нуля держите стик внизу."), 14))->SetPadding(FMargin(0,0,0,12));
            const EDroneInputAxis Axes[] = {EDroneInputAxis::Throttle, EDroneInputAxis::Yaw, EDroneInputAxis::Pitch, EDroneInputAxis::Roll};
            const TCHAR* Labels[] = {TEXT("Газ"), TEXT("Поворот"), TEXT("Тангаж"), TEXT("Крен")};
            const FKey Keys[] = {Draft.Gamepad.Throttle,Draft.Gamepad.Yaw,Draft.Gamepad.Pitch,Draft.Gamepad.Roll};
            for (int32 Index=0; Index<4; ++Index)
            {
                auto* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
                auto* LabelSlot = Row->AddChildToHorizontalBox(Text(WidgetTree,Labels[Index],16)); LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                auto* Picker = WidgetTree->ConstructWidget<UComboBoxString>();
                StyleCombo(Picker);
                for (FKey Key : FDroneGamepadSettings::AxisKeys()) Picker->AddOption(AxisLabel(Key));
                Picker->SetSelectedOption(AxisLabel(Keys[Index]));
                auto* Binding = NewObject<UDroneGamepadAxisBinding>(this); Binding->Settings=Settings; Binding->Axis=Axes[Index]; AxisBindings.Add(Binding);
                Picker->OnSelectionChanged.AddDynamic(Binding,&UDroneGamepadAxisBinding::Changed);
                Row->AddChildToHorizontalBox(Picker); Content->AddChildToVerticalBox(Row)->SetPadding(FMargin(0,3));
            }
            auto* Reset = Button(WidgetTree,TEXT("Сброс раскладки Xbox"));
            Reset->OnClicked.AddDynamic(this,&UDronePauseMenu::ResetGamepadLayout); Content->AddChildToVerticalBox(Reset);
            auto* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
            Row->AddChildToHorizontalBox(Text(WidgetTree,TEXT("Мёртвая зона стиков и курка"),16))->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            auto* Spin=WidgetTree->ConstructWidget<USpinBox>(); Spin->SetMinValue(0); Spin->SetMaxValue(0.4f); Spin->SetDelta(0.01f); Spin->SetValue(Draft.Gamepad.Deadzone);
            Spin->SetForegroundColor(FSlateColor(FLinearColor(0.03f,0.05f,0.08f)));
            Spin->OnValueChanged.AddDynamic(this,&UDronePauseMenu::GamepadDeadzoneChanged); Row->AddChildToHorizontalBox(Spin); Content->AddChildToVerticalBox(Row);
            auto* Invert=WidgetTree->ConstructWidget<UCheckBox>(); Invert->SetIsChecked(Draft.Gamepad.InvertPitch);
            Invert->SetContent(Text(WidgetTree,TEXT("Инверсия тангажа геймпада"),16));
            Invert->OnCheckStateChanged.AddDynamic(this,&UDronePauseMenu::GamepadPitchChanged); Content->AddChildToVerticalBox(Invert);
        }
        else if (Draft.ControlSource == TEXT("Radio"))
        {
            Content->AddChildToVerticalBox(Text(WidgetTree,TEXT("Pocket Mode 2: левый стик — газ/поворот, правый — тангаж/крен. USB-подключение проверяется каждые 2 секунды. Для Pocket с готовым профилем повторная калибровка не нужна."),14));
            auto* B=Button(WidgetTree,TEXT("Открыть ручную калибровку"),TEXT("CalibrationButton"));
            B->OnClicked.AddDynamic(this,&UDronePauseMenu::OpenCalibration); Content->AddChildToVerticalBox(B);
        }
    }

    for (const auto& D : UDroneSettingsSubsystem::Descriptors()) if (D.Section == ActiveSection)
    {
        auto* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
        auto* Label = Text(WidgetTree, D.Label, 17);
        auto* LabelSlot = Row->AddChildToHorizontalBox(Label); LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); LabelSlot->SetVerticalAlignment(VAlign_Center);
        auto* Box = WidgetTree->ConstructWidget<USizeBox>(); Box->SetWidthOverride(160);
        auto* Spin = WidgetTree->ConstructWidget<USpinBox>(USpinBox::StaticClass(), *FString::Printf(TEXT("Setting_%s"), *D.Key.ToString()));
        Spin->SetMinValue(D.Minimum); Spin->SetMaxValue(D.Maximum); Spin->SetMinSliderValue(D.Minimum); Spin->SetMaxSliderValue(D.Maximum);
        Spin->SetDelta(D.Step); Spin->SetValue(Settings->GetDraftNumber(D.Key));
        Spin->SetForegroundColor(FSlateColor(FLinearColor(0.03f, 0.05f, 0.08f)));
        auto* Binding = NewObject<UDroneSettingBinding>(this); Binding->Settings = Settings; Binding->Key = D.Key; Bindings.Add(Binding);
        Spin->OnValueChanged.AddDynamic(Binding, &UDroneSettingBinding::Changed);
        Box->SetContent(Spin); Row->AddChildToHorizontalBox(Box);
        Content->AddChildToVerticalBox(Row)->SetPadding(FMargin(0, 4, 14, 4));
    }
}
void UDronePauseMenu::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);
    if (Settings && Status)
    {
        const auto* PC=Cast<ADroneTestPlayerController>(GetOwningPlayer());
        const FString ModeError=PC ? PC->GetPlayModeError() : FString();
        const FString Error = !ModeError.IsEmpty() ? ModeError : Settings->GetLastError();
        Status->SetText(FText::FromString(!Error.IsEmpty() ? Error : Settings->NeedsRestart()
            ? TEXT("После применения коптер вернётся на старт. Настройки сохранятся между запусками.")
            : TEXT("Изменения сохранятся только по кнопке «Применить». TAB / «Продолжить» — без применения.")));
    }
    if (DeviceReadout)
    {
        auto* Input = GetGameInstance()->GetSubsystem<UDroneInputSubsystem>();
        const FName Source = Settings->GetDraft().ControlSource;
        FString Devices = Input ? Input->GetControlSourceStatus(Source) : TEXT("Система ввода недоступна");
        if (Input) Devices += FString::Printf(TEXT("\nГаз: %.0f%%   Поворот: %+.2f   Тангаж: %+.2f   Крен: %+.2f"),
            Input->PreviewControlAxis(Source,EDroneInputAxis::Throttle)*100.f,
            Input->PreviewControlAxis(Source,EDroneInputAxis::Yaw),Input->PreviewControlAxis(Source,EDroneInputAxis::Pitch),Input->PreviewControlAxis(Source,EDroneInputAxis::Roll));
        DeviceReadout->SetText(FText::FromString(Devices));
    }
}
FReply UDronePauseMenu::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
    if (Event.GetKey()==EKeys::F || Event.GetKey()==EKeys::Gamepad_Special_Right)
    { SwitchPlayMode(); return FReply::Handled(); }
    if (Event.GetKey() == EKeys::Tab || Event.GetKey() == EKeys::Gamepad_Special_Left)
    {
        ResumeFlight(); return FReply::Handled();
    }
    return Super::NativeOnPreviewKeyDown(Geometry, Event);
}
void UDronePauseMenu::FlightModeChanged(bool Checked) { if (Settings) Settings->SetDraftAcro(Checked); }
void UDronePauseMenu::ApplyAndResume()
{
    auto* PC = Cast<ADroneTestPlayerController>(GetOwningPlayer());
    if (!PC || !Settings) return;
    const bool Restart = Settings->NeedsRestart();
    if (!Settings->CommitDraft()) return;
    if (Restart) PC->RestartFlight();
    else { PC->ApplyFlightSettings(); PC->ClosePauseMenu(); }
}
void UDronePauseMenu::SwitchPlayMode()
{
    if (auto* PC=Cast<ADroneTestPlayerController>(GetOwningPlayer()))
        if (!PC->SetWalkingMode(!PC->IsWalkingMode()) && Status) Status->SetText(FText::FromString(PC->GetPlayModeError()));
}
void UDronePauseMenu::ResetMeteorDraft() { if (Settings) { Settings->ResetDraft(); SelectSection(ActiveSection); } }
void UDronePauseMenu::ResumeFlight() { if (auto* PC = Cast<ADroneTestPlayerController>(GetOwningPlayer())) PC->ClosePauseMenu(); }
void UDronePauseMenu::RestartFlight() { if (auto* PC = Cast<ADroneTestPlayerController>(GetOwningPlayer())) PC->RestartFlight(); }
void UDronePauseMenu::OpenCalibration() { if (auto* PC = Cast<ADroneTestPlayerController>(GetOwningPlayer())) PC->show_calibration_menu(); }
FString UDronePauseMenu::GetStatusText() const
{
    const auto* PC=Cast<ADroneTestPlayerController>(GetOwningPlayer());
    if (PC && !PC->GetPlayModeError().IsEmpty()) return PC->GetPlayModeError();
    if (!Settings) return FString();
    if (!Settings->GetLastError().IsEmpty()) return Settings->GetLastError();
    return Settings->NeedsRestart()
        ? TEXT("После применения коптер вернётся на старт. Настройки сохранятся между запусками.")
        : TEXT("Изменения сохранятся только по кнопке «Применить». TAB / «Продолжить» — без применения.");
}

void UDronePauseMenu::SourceChanged(FString Selected, ESelectInfo::Type SelectionType)
{
    if (!Settings || !SourcePicker) return;
    const int32 Index=SourcePicker->FindOptionIndex(Selected);
    if (SourceIds.IsValidIndex(Index)) { Settings->SetDraftControlSource(SourceIds[Index]); SelectSection(2); }
}
void UDronePauseMenu::GamepadDeadzoneChanged(float Value)
{
    if (Settings) { auto Gamepad=Settings->GetDraft().Gamepad; Gamepad.Deadzone=Value; Settings->SetDraftGamepad(Gamepad); }
}
void UDronePauseMenu::GamepadPitchChanged(bool Value)
{
    if (Settings) { auto Gamepad=Settings->GetDraft().Gamepad; Gamepad.InvertPitch=Value; Settings->SetDraftGamepad(Gamepad); }
}

void UDronePauseMenu::ResetGamepadLayout()
{
    if (Settings) { Settings->SetDraftGamepad(FDroneGamepadSettings()); SelectSection(2); }
}
