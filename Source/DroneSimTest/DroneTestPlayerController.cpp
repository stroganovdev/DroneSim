#include "DroneTestPlayerController.h"
#include "UI/DronePauseMenu.h"
#include "Settings/DroneFlightSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"

#include "DroneSimulatorGame/Gameplay/DronePawn.h"
#include "DroneSimulatorGame/Gameplay/DroneMovementComponent.h"
#include "DroneSimulatorInput/Public/DroneInputSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"

ADroneTestPlayerController::ADroneTestPlayerController()
{
    static ConstructorHelpers::FClassFinder<APawn> CharacterAsset(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    WalkingCharacterClass=CharacterAsset.Class;
    static ConstructorHelpers::FObjectFinder<UInputMappingContext> Walk(TEXT("/Game/Input/IMC_Default"));
    static ConstructorHelpers::FObjectFinder<UInputMappingContext> Mouse(TEXT("/Game/Input/IMC_MouseLook"));
    static ConstructorHelpers::FObjectFinder<UInputMappingContext> Flight(TEXT("/DroneSimulator/Blueprints/IMC_MappingContext"));
    WalkMapping=Walk.Object; MouseMapping=Mouse.Object; FlightMapping=Flight.Object;
    static ConstructorHelpers::FClassFinder<UUserWidget> HUDAsset(TEXT("/DroneSimulator/UI/WB_HUD"));
    FlightHUDClass=HUDAsset.Class;
}

void ADroneTestPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Settings = GetGameInstance()->GetSubsystem<UDroneSettingsSubsystem>()) Settings->ApplyToPawn(GetPawn());
}
void ADroneTestPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (Cast<ADronePawn>(InPawn))
    {
        if (FlightPawn != InPawn) { FlightPawn=InPawn; FlightStart=InPawn->GetActorTransform(); }
        ApplyFlightSettings();
    }
}
void ADroneTestPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    auto& Binding = InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ADroneTestPlayerController::TogglePauseMenu);
    Binding.bExecuteWhenPaused = true;
    auto& GamepadPause = InputComponent->BindKey(EKeys::Gamepad_Special_Left, IE_Pressed, this, &ADroneTestPlayerController::TogglePauseMenu);
    GamepadPause.bExecuteWhenPaused = true;
    InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ADroneTestPlayerController::ToggleWalkingMode);
    InputComponent->BindKey(EKeys::Gamepad_Special_Right, IE_Pressed, this, &ADroneTestPlayerController::ToggleWalkingMode);
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
    if (IsWalkingMode())
    {
        if (!IsValid(WalkingPawn)) { SetWalkingMode(false); return; }
        ApplyFlightSettings();
        if (auto* Walk=Cast<ACharacter>(WalkingPawn)) Walk->GetCharacterMovement()->StopMovementImmediately();
        WalkingPawn->SetActorTransform(WalkStart,false,nullptr,ETeleportType::TeleportPhysics);
        return;
    }
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

bool ADroneTestPlayerController::IsWalkingMode() const
{
    return WalkingModeActive;
}
void ADroneTestPlayerController::ApplyFlightSettings()
{
    if (IsValid(FlightPawn))
        if (auto* Settings = GetGameInstance()->GetSubsystem<UDroneSettingsSubsystem>()) Settings->ApplyToPawn(FlightPawn);
    // ApplyToPawn configures the selected flight input. Keep it disabled while walking.
    if (IsWalkingMode())
        if (auto* Input = GetGameInstance()->GetSubsystem<UDroneInputSubsystem>()) Input->ClearControlSource();
}
void ADroneTestPlayerController::SetModeMappings(bool Walking)
{
    auto* Local = GetLocalPlayer();
    auto* Input = Local ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Local) : nullptr;
    if (!Input) return;
    FModifyContextOptions Options;
    Options.bIgnoreAllPressedKeysUntilRelease = true;
    Options.bForceImmediately = true;
    if (Walking)
    {
        if (FlightContextWasActive) Input->RemoveMappingContext(FlightMapping, Options);
        Input->AddMappingContext(WalkMapping, 1, Options);
        Input->AddMappingContext(MouseMapping, 1, Options);
    }
    else
    {
        Input->RemoveMappingContext(WalkMapping, Options);
        Input->RemoveMappingContext(MouseMapping, Options);
        if (FlightContextWasActive) Input->AddMappingContext(FlightMapping, FlightContextPriority, Options);
    }
}
bool ADroneTestPlayerController::FindWalkSpawn(FVector& Location, FRotator& Rotation) const
{
    const auto* WalkDefaults = WalkingCharacterClass ? Cast<ACharacter>(WalkingCharacterClass->GetDefaultObject()) : nullptr;
    if (!WalkDefaults) return false;
    const auto* Capsule = WalkDefaults->GetCapsuleComponent();
    const float Radius = Capsule->GetScaledCapsuleRadius();
    const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
    FCollisionQueryParams Query(SCENE_QUERY_STAT(DroneWalkSpawn), false);
    if (IsValid(FlightPawn)) Query.AddIgnoredActor(FlightPawn);
    if (IsValid(WalkingPawn)) Query.AddIgnoredActor(WalkingPawn);
    const FVector Offsets[] = {FVector(-200,0,0),FVector(0,-200,0),FVector(0,200,0),FVector(200,0,0),FVector(-400,0,0)};
    for (const FVector& Offset : Offsets)
    {
        const FVector Anchor = FlightStart.GetLocation() + Offset;
        FHitResult Ground;
        if (!GetWorld()->LineTraceSingleByChannel(Ground, Anchor + FVector(0,0,500), Anchor-FVector(0,0,10000), ECC_Visibility, Query)) continue;
        if (Ground.ImpactNormal.Z < 0.7f) continue;
        const FVector Candidate = Ground.ImpactPoint + FVector(0,0,HalfHeight+3.f);
        if (GetWorld()->OverlapBlockingTestByProfile(Candidate,FQuat::Identity,TEXT("Pawn"),FCollisionShape::MakeCapsule(Radius,HalfHeight),Query)) continue;
        Location = Candidate;
        Rotation = FRotator(0,FlightStart.Rotator().Yaw,0);
        return true;
    }
    return false;
}
void ADroneTestPlayerController::ToggleWalkingMode() { SetWalkingMode(!IsWalkingMode()); }
bool ADroneTestPlayerController::SetWalkingMode(bool Walking)
{
    if (!IsLocalController() || ChangingPlayMode) return false;
    if (Walking == IsWalkingMode()) return true;
    PlayModeError.Empty();
    if (CalibrationFromMenu) { PlayModeError=TEXT("Сначала закройте калибровку."); return false; }
    if (UGameplayStatics::IsGamePaused(this) && !Menu) { PlayModeError=TEXT("Сначала снимите внешнюю паузу."); return false; }
    if (!IsValid(FlightPawn))
    { PlayModeError=TEXT("Коптер или персонаж недоступен. Вернитесь на старт."); return false; }
    auto* Body=Cast<UPrimitiveComponent>(FlightPawn->GetRootComponent());
    auto* Movement=FlightPawn->FindComponentByClass<UDroneMovementComponent>();
    if (!Body || !Movement) { PlayModeError=TEXT("Не найдена физическая основа коптера."); return false; }
    if (Walking)
    {
        if (!WalkingCharacterClass || !WalkMapping || !MouseMapping)
        { PlayModeError=TEXT("Не загружены ассеты Third Person."); return false; }
        if (!IsValid(WalkingPawn))
        {
            FVector Location; FRotator Rotation;
            if (!FindWalkSpawn(Location,Rotation))
            { PlayModeError=TEXT("Рядом со стартом нет свободного места для персонажа."); return false; }
            FActorSpawnParameters Params; Params.Owner=this; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
            WalkingPawn=GetWorld()->SpawnActor<APawn>(WalkingCharacterClass,Location,Rotation,Params);
            if (!WalkingPawn) { PlayModeError=TEXT("Не удалось создать персонажа."); return false; }
            WalkStart=WalkingPawn->GetActorTransform();
        }
        WalkingPawn->SetActorHiddenInGame(false);
        WalkingPawn->SetActorEnableCollision(true);
        if (auto* Walk=Cast<ACharacter>(WalkingPawn)) Walk->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }
    if (Walking)
    {
        auto* Local=GetLocalPlayer();
        auto* Contexts=Local ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Local) : nullptr;
        FlightContextWasActive=Contexts && FlightMapping && Contexts->HasMappingContext(FlightMapping,FlightContextPriority);
    }
    TGuardValue<bool> Guard(ChangingPlayMode,true);
    APawn* Target=Walking ? WalkingPawn.Get() : FlightPawn.Get();
    APawn* Previous=GetPawn();
    ClosePauseMenu(); // mode changes discard unsaved menu drafts
    Possess(Target);
    if (GetPawn()!=Target)
    {
        if (Previous) Possess(Previous);
        if (Walking) { WalkingPawn->SetActorHiddenInGame(true); WalkingPawn->SetActorEnableCollision(false); }
        PlayModeError=TEXT("Не удалось переключить управление.");
        return false;
    }
    WalkingModeActive=Walking;
    if (Walking)
    {
        FlightWasSimulating=Body->IsSimulatingPhysics();
        FlightMovementWasTicking=Movement->IsComponentTickEnabled();
        Movement->SetComponentTickEnabled(false);
        if (FlightWasSimulating) { Body->SetPhysicsLinearVelocity(FVector::ZeroVector); Body->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector); }
        Body->SetSimulatePhysics(false);
        FlightPawn->SetActorTransform(FlightStart,false,nullptr,ETeleportType::TeleportPhysics);
        if (auto* Input=GetGameInstance()->GetSubsystem<UDroneInputSubsystem>()) Input->ClearControlSource();
    }
    else
    {
        if (auto* Walk=Cast<ACharacter>(WalkingPawn)) { Walk->GetCharacterMovement()->StopMovementImmediately(); Walk->GetCharacterMovement()->DisableMovement(); }
        if (IsValid(WalkingPawn)) { WalkingPawn->SetActorHiddenInGame(true); WalkingPawn->SetActorEnableCollision(false); }
        Body->SetSimulatePhysics(FlightWasSimulating);
        Movement->refresh_drone_configuration();
        Movement->SetComponentTickEnabled(FlightMovementWasTicking);
        ApplyFlightSettings(); // ClearControlSource on entry guarantees a new neutral gate.
    }
    if (Walking)
    {
        TArray<UUserWidget*> Widgets;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this,Widgets,FlightHUDClass,false);
        HiddenFlightHUD.Empty();
        for (UUserWidget* Widget : Widgets)
            if (Widget && Widget->GetOwningPlayer()==this)
            { HiddenFlightHUD.Add(Widget,Widget->GetVisibility()); Widget->SetVisibility(ESlateVisibility::Collapsed); }
    }
    else
    {
        for (const auto& Entry : HiddenFlightHUD)
            if (IsValid(Entry.Key) && Entry.Key->IsInViewport()) Entry.Key->SetVisibility(Entry.Value);
        HiddenFlightHUD.Empty();
    }
    SetModeMappings(Walking);
    FlushPressedKeys();
    SetControlRotation(Walking ? WalkingPawn->GetActorRotation() : FlightStart.Rotator());
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor=false;
    return true;
}
