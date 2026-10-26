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
    Layout->AddChildToVerticalBox(Text(WidgetTree, TEXT("ПАУЗА  /  НАСТРОЙКИ ПОЛЁТА"), 27))->SetPadding(FMargin(0, 0, 0, 5));
    auto* Subtitle = Text(WidgetTree, TEXT("METEOR75 PRO     •     Полёт приостановлен     •     TAB — продолжить"), 14);
    Subtitle->SetColorAndOpacity(FSlateColor(FLinearColor(0.40f, 0.72f, 0.82f)));
    Layout->AddChildToVerticalBox(Subtitle)->SetPadding(FMargin(0, 0, 0, 20));
    auto* Tabs = WidgetTree->ConstructWidget<UHorizontalBox>(); Layout->AddChildToVerticalBox(Tabs)->SetPadding(FMargin(0, 0, 0, 18));
    auto* B = Button(WidgetTree, TEXT("Управление"), TEXT("ControlsTab")); B->OnClicked.AddDynamic(this, &UDronePauseMenu::Controls); Tabs->AddChildToHorizontalBox(B)->SetPadding(FMargin(0, 0, 8, 0));
    B = Button(WidgetTree, TEXT("Коптер"), TEXT("AircraftTab")); B->OnClicked.AddDynamic(this, &UDronePauseMenu::Aircraft); Tabs->AddChildToHorizontalBox(B)->SetPadding(FMargin(0, 0, 8, 0));
    B = Button(WidgetTree, TEXT("Пульт"), TEXT("RadioTab")); B->OnClicked.AddDynamic(this, &UDronePauseMenu::Radio); Tabs->AddChildToHorizontalBox(B)->SetPadding(FMargin(0, 0, 8, 0));
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
    Content->ClearChildren(); Bindings.Empty(); DeviceReadout = nullptr;
    const FString Names[] = {TEXT("Управление"), TEXT("Физика коптера"), TEXT("RadioMaster и другие HID-пульты"), TEXT("Камера")};
    Content->AddChildToVerticalBox(Text(WidgetTree, Names[ActiveSection], 23))->SetPadding(FMargin(0, 0, 0, 8));
    if (ActiveSection == 0)
    {
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
        DeviceReadout = Text(WidgetTree, TEXT("Ожидание HID-данных…"), 18);
        Content->AddChildToVerticalBox(DeviceReadout)->SetPadding(FMargin(0, 0, 0, 20));
        Content->AddChildToVerticalBox(Text(WidgetTree, TEXT("Mode 2: левый стик — газ и поворот; правый — тангаж и крен. Показания обновляются и на паузе. USB-подключение проверяется плагином каждые 2 секунды."), 16))->SetPadding(FMargin(0, 0, 0, 16));
        auto* B = Button(WidgetTree, TEXT("Открыть ручную калибровку"), TEXT("CalibrationButton"));
        B->OnClicked.AddDynamic(this, &UDronePauseMenu::OpenCalibration); Content->AddChildToVerticalBox(B);
        Content->AddChildToVerticalBox(Text(WidgetTree, TEXT("Для готового профиля Pocket повторная калибровка не нужна. После закрытия калибровки вы вернётесь в меню паузы."), 14))->SetPadding(FMargin(0, 10, 0, 0));
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
        const FString Error = Settings->GetLastError();
        Status->SetText(FText::FromString(!Error.IsEmpty() ? Error : Settings->NeedsRestart()
            ? TEXT("После применения коптер вернётся на старт. Настройки сохранятся между запусками.")
            : TEXT("Изменения сохранятся только по кнопке «Применить». TAB / «Продолжить» — без применения.")));
    }
    if (DeviceReadout)
    {
        auto* Input = GetGameInstance()->GetSubsystem<UDroneInputSubsystem>();
        FString Devices;
        if (Input) for (const auto& Device : Input->get_all_devices()) if (!Device.device_uid.IsEmpty())
        {
            Devices += FString::Printf(TEXT("%s  •  %s\n"), *Device.device_name, Input->is_device_calibrated(Device.device_id) ? TEXT("Профиль загружен") : TEXT("Без калибровки"));
        }
        if (Devices.IsEmpty()) Devices = TEXT("Пульт не обнаружен. Включите Pocket → верхний USB → USB Joystick.\n");
        if (Input) Devices += FString::Printf(TEXT("\nГаз: %.0f%%     Поворот: %+.2f     Тангаж: %+.2f     Крен: %+.2f"),
            Input->get_calibrated_axis_value(EDroneInputAxis::Throttle) * 100.f,
            Input->get_calibrated_axis_value(EDroneInputAxis::Yaw), Input->get_calibrated_axis_value(EDroneInputAxis::Pitch), Input->get_calibrated_axis_value(EDroneInputAxis::Roll));
        DeviceReadout->SetText(FText::FromString(Devices));
    }
}
FReply UDronePauseMenu::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
    if (Event.GetKey() == EKeys::Tab)
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
    else { Settings->ApplyToPawn(PC->GetPawn()); PC->ClosePauseMenu(); }
}
void UDronePauseMenu::ResetMeteorDraft() { if (Settings) { Settings->ResetDraft(); SelectSection(ActiveSection); } }
void UDronePauseMenu::ResumeFlight() { if (auto* PC = Cast<ADroneTestPlayerController>(GetOwningPlayer())) PC->ClosePauseMenu(); }
void UDronePauseMenu::RestartFlight() { if (auto* PC = Cast<ADroneTestPlayerController>(GetOwningPlayer())) PC->RestartFlight(); }
void UDronePauseMenu::OpenCalibration() { if (auto* PC = Cast<ADroneTestPlayerController>(GetOwningPlayer())) PC->show_calibration_menu(); }
FString UDronePauseMenu::GetStatusText() const { return Status ? Status->GetText().ToString() : FString(); }
