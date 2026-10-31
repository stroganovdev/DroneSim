#pragma once
#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "DroneSimulatorGame/Gameplay/DronePlayerController.h"
#include "DroneTestPlayerController.generated.h"
class UDronePauseMenu;

UCLASS()
class DRONESIMTEST_API ADroneTestPlayerController : public ADronePlayerController
{
    GENERATED_BODY()
public:
    ADroneTestPlayerController();
    virtual void BeginPlay() override;
    UFUNCTION(BlueprintCallable, Category="Play Mode") bool SetWalkingMode(bool Walking);
    UFUNCTION(BlueprintCallable, Category="Play Mode") void ToggleWalkingMode();
    UFUNCTION(BlueprintPure, Category="Play Mode") bool IsWalkingMode() const;
    UFUNCTION(BlueprintPure, Category="Play Mode") FString GetPlayModeError() const { return PlayModeError; }
    UFUNCTION(BlueprintCallable) void ApplyFlightSettings();
    UPROPERTY(EditAnywhere, Category="Play Mode") TSubclassOf<APawn> WalkingCharacterClass;

    virtual void OnPossess(APawn* InPawn) override;
    virtual void SetupInputComponent() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UFUNCTION(BlueprintCallable) void TogglePauseMenu();
    UFUNCTION(BlueprintCallable) void OpenPauseMenu();
    UFUNCTION(BlueprintCallable) void ClosePauseMenu();
    UFUNCTION(BlueprintCallable) void RestartFlight();
    UFUNCTION(BlueprintPure) bool IsPauseMenuOpen() const { return Menu != nullptr; }
    UFUNCTION(BlueprintPure) UDronePauseMenu* GetPauseMenu() const { return Menu.Get(); }
    virtual void show_calibration_menu() override;
    virtual void hide_calibration_menu() override;
private:
    UPROPERTY(Transient) TObjectPtr<APawn> FlightPawn;
    UPROPERTY(Transient) TObjectPtr<APawn> WalkingPawn;
    UPROPERTY(Transient) TObjectPtr<class UInputMappingContext> WalkMapping;
    UPROPERTY(Transient) TObjectPtr<class UInputMappingContext> MouseMapping;
    UPROPERTY(Transient) TObjectPtr<class UInputMappingContext> FlightMapping;
    UPROPERTY(Transient) TSubclassOf<class UUserWidget> FlightHUDClass;
    UPROPERTY(Transient) TMap<TObjectPtr<UUserWidget>, ESlateVisibility> HiddenFlightHUD;
    FTransform FlightStart;
    FTransform WalkStart;
    bool FlightWasSimulating = false;
    bool FlightMovementWasTicking = false;
    bool FlightContextWasActive = false;
    int32 FlightContextPriority = 0;
    bool ChangingPlayMode = false;
    bool WalkingModeActive = false;
    FString PlayModeError;
    bool FindWalkSpawn(FVector& Location, FRotator& Rotation) const;
    void SetModeMappings(bool Walking);
    UPROPERTY(Transient) TObjectPtr<UDronePauseMenu> Menu;
    bool WasPaused = false;
    bool CalibrationFromMenu = false;
    void FocusPauseMenu();
};
