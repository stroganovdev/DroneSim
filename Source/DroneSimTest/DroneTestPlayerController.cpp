#include "DroneTestPlayerController.h"
#include "UI/DronePauseMenu.h"
#include "Settings/DroneFlightSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"

void ADroneTestPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Settings = GetGameInstance()->GetSubsystem<UDroneSettingsSubsystem>()) Settings->ApplyToPawn(GetPawn());
}
void ADroneTestPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (auto* Settings = GetGameInstance()->GetSubsystem<UDroneSettingsSubsystem>()) Settings->ApplyToPawn(InPawn);
}
void ADroneTestPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    auto& Binding = InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ADroneTestPlayerController::TogglePauseMenu);
    Binding.bExecuteWhenPaused = true;
}
void ADroneTestPlayerController::TogglePauseMenu()
{
    if (CalibrationFromMenu) { hide_calibration_menu(); return; }
    if (Menu) ClosePauseMenu(); else OpenPauseMenu();
}
void ADroneTestPlayerController::FocusPauseMenu()
{
    if (!Menu) return;
    Menu->SetVisibility(ESlateVisibility::Visible);
    FInputModeGameAndUI Mode;
    Mode.SetWidgetToFocus(Menu->TakeWidget());
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    Mode.SetHideCursorDuringCapture(false);
    SetInputMode(Mode);
    bShowMouseCursor = true;
    Menu->SetKeyboardFocus();
}
void ADroneTestPlayerController::OpenPauseMenu()
{
    if (Menu || !IsLocalController()) return;
    WasPaused = UGameplayStatics::IsGamePaused(this);
    if (!WasPaused && !SetPause(true)) return;
    auto* Settings = GetGameInstance()->GetSubsystem<UDroneSettingsSubsystem>();
    if (!Settings) { if (!WasPaused) SetPause(false); return; }
    Settings->BeginEditing();
    Menu = CreateWidget<UDronePauseMenu>(this, UDronePauseMenu::StaticClass());
    Menu->AddToViewport(100);
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);
    FocusPauseMenu();
}
void ADroneTestPlayerController::ClosePauseMenu()
{
    if (!Menu) return;
    if (CalibrationFromMenu)
    {
        CalibrationFromMenu = false;
        Super::hide_calibration_menu();
    }
    Menu->RemoveFromParent();
    Menu = nullptr;
    SetIgnoreMoveInput(false);
    SetIgnoreLookInput(false);
    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());
    if (!WasPaused) SetPause(false);
}
void ADroneTestPlayerController::RestartFlight()
{
    ClosePauseMenu();
    if (auto* GameMode = GetWorld()->GetAuthGameMode())
    {
        APawn* Old = GetPawn();
        UnPossess();
        if (Old) Old->Destroy();
        GameMode->RestartPlayer(this);
    }
}
void ADroneTestPlayerController::show_calibration_menu()
{
    if (Menu)
    {
        CalibrationFromMenu = true;
        Menu->SetVisibility(ESlateVisibility::Collapsed);
    }
    Super::show_calibration_menu();
}
void ADroneTestPlayerController::hide_calibration_menu()
{
    Super::hide_calibration_menu();
    if (CalibrationFromMenu)
    {
        CalibrationFromMenu = false;
        FocusPauseMenu();
    }
}
void ADroneTestPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
    ClosePauseMenu();
    Super::hide_calibration_menu();
    Super::EndPlay(Reason);
}
