#pragma once
#include "CoreMinimal.h"
#include "DroneSimulatorGame/Gameplay/DronePlayerController.h"
#include "DroneTestPlayerController.generated.h"
class UDronePauseMenu;

UCLASS()
class DRONESIMTEST_API ADroneTestPlayerController : public ADronePlayerController
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
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
    UPROPERTY(Transient) TObjectPtr<UDronePauseMenu> Menu;
    bool WasPaused = false;
    bool CalibrationFromMenu = false;
    void FocusPauseMenu();
};
