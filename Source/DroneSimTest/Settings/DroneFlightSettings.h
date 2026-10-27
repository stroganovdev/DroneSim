#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DroneSimulatorInput/Public/DroneInputTypes.h"
#include "DroneFlightSettings.generated.h"

class APawn;
class UDroneMovementComponent;
class UDroneFrameAsset;
class UDronePropellerSimplifiedAsset;
class UDroneBatteryAsset;

USTRUCT(BlueprintType)
struct FDroneFlightSettings
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FName ControlSource = TEXT("Radio");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FDroneGamepadSettings Gamepad;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) bool Acro = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float RollRate = 670.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float PitchRate = 670.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float YawRate = 670.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float CenterSensitivity = 70.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float Expo = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float MassGrams = 45.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float MotorResponseMs = 25.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float ThrustScale = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float DragScale = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float Voltage = 3.8f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float CameraFov = 100.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float CameraTilt = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) float ChaseDistance = 60.f;
};

// Add a field above and a descriptor here to expose another numeric setting in the menu.
struct FDroneSettingDescriptor
{
    FName Key;
    FString Label;
    uint8 Section;
    float Minimum, Maximum, Step;
};

UCLASS()
class UDroneFlightSettingsSave : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame) int32 Version = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FDroneFlightSettings Settings;
};

UCLASS()
class DRONESIMTEST_API UDroneSettingsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    static const TArray<FDroneSettingDescriptor>& Descriptors();
    UFUNCTION(BlueprintPure) FDroneFlightSettings GetSettings() const { return Current; }
    UFUNCTION(BlueprintPure) FDroneFlightSettings GetDraft() const { return Draft; }
    UFUNCTION(BlueprintCallable) void BeginEditing() { Draft = Current; }
    UFUNCTION(BlueprintCallable) void ResetDraft() { const FName Source = Draft.ControlSource; const auto Gamepad = Draft.Gamepad; Draft = FDroneFlightSettings(); Draft.ControlSource = Source; Draft.Gamepad = Gamepad; }
    UFUNCTION(BlueprintCallable) bool CommitDraft();
    UFUNCTION(BlueprintCallable) void SetDraftControlSource(FName Id) { Draft.ControlSource = Id; }
    UFUNCTION(BlueprintCallable) void SetDraftGamepad(FDroneGamepadSettings Value) { Value.Sanitize(); Draft.Gamepad = Value; }
    UFUNCTION(BlueprintCallable) void SetDraftAcro(bool Value) { Draft.Acro = Value; }
    UFUNCTION(BlueprintCallable) void SetDraftNumber(FName Key, float Value);
    UFUNCTION(BlueprintPure) float GetDraftNumber(FName Key) const;
    UFUNCTION(BlueprintPure) bool NeedsRestart() const;
    UFUNCTION(BlueprintCallable) void ApplyToPawn(APawn* Pawn);
    UFUNCTION(BlueprintPure) FString GetLastError() const { return LastError; }
    static void Sanitize(FDroneFlightSettings& Settings);
private:
    UPROPERTY() FDroneFlightSettings Current;
    UPROPERTY() FDroneFlightSettings Draft;
    UPROPERTY(Transient) TObjectPtr<UDroneFrameAsset> RuntimeFrame;
    UPROPERTY(Transient) TObjectPtr<UDronePropellerSimplifiedAsset> RuntimePropeller;
    UPROPERTY(Transient) TObjectPtr<UDroneBatteryAsset> RuntimeBattery;
    TWeakObjectPtr<APawn> PreparedPawn;
    FVector BaseInertia, BaseArea, BaseRotationalDrag;
    double BaseTotalMass = 0.045, BaseCt = 0.0673303855, BaseCq = 0.005;
    FString LastError;
    bool FutureVersion = false;
};
