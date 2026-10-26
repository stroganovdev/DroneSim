#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DronePauseMenu.generated.h"
class UDroneSettingsSubsystem;
class UVerticalBox;
class UTextBlock;
class USpinBox;
class UCheckBox;

UCLASS()
class UDroneSettingBinding : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() TObjectPtr<UDroneSettingsSubsystem> Settings;
    FName Key;
    UFUNCTION() void Changed(float Value);
};

UCLASS()
class DRONESIMTEST_API UDronePauseMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
    UFUNCTION(BlueprintCallable) void SelectSection(int32 Section);
    UFUNCTION(BlueprintCallable) void ApplyAndResume();
    UFUNCTION(BlueprintCallable) void ResetMeteorDraft();
    UFUNCTION(BlueprintCallable) void ResumeFlight();
    UFUNCTION(BlueprintCallable) void RestartFlight();
    UFUNCTION(BlueprintCallable) void OpenCalibration();
    UFUNCTION(BlueprintPure) FString GetStatusText() const;
private:
    UPROPERTY() TObjectPtr<UDroneSettingsSubsystem> Settings;
    UPROPERTY() TObjectPtr<UVerticalBox> Content;
    UPROPERTY() TObjectPtr<UTextBlock> Status;
    UPROPERTY() TObjectPtr<UTextBlock> DeviceReadout;
    UPROPERTY() TArray<TObjectPtr<UDroneSettingBinding>> Bindings;
    int32 ActiveSection = 0;
    UFUNCTION() void Controls() { SelectSection(0); }
    UFUNCTION() void Aircraft() { SelectSection(1); }
    UFUNCTION() void Radio() { SelectSection(2); }
    UFUNCTION() void Camera() { SelectSection(3); }
    UFUNCTION() void FlightModeChanged(bool Checked);
};
