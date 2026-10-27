#pragma once
#include "DroneControlSource.h"
#include "DroneInputSubsystem.h"

class FDroneRadioSource final : public IDroneControlSource
{
public:
    explicit FDroneRadioSource(UDroneInputSubsystem* InInput) : Input(InInput) {}
    FName GetId() const override { return TEXT("Radio"); }
    FText GetLabel() const override { return FText::FromString(TEXT("FPV-пульт / HID")); }
    bool IsAvailable() const override { return Input.IsValid() && Input->GetRadioDeviceId() != INDEX_NONE; }
    float ReadAxis(EDroneInputAxis Axis, EThrottleCalibrationSpace Space) const override
    { return Input.IsValid() ? Input->ReadRadioAxis(Axis, Space) : 0.f; }
private:
    TWeakObjectPtr<UDroneInputSubsystem> Input;
};

class FDroneGamepadSource final : public IDroneControlSource
{
public:
    FName GetId() const override { return TEXT("Gamepad"); }
    FText GetLabel() const override { return FText::FromString(TEXT("Xbox / геймпад")); }
    bool IsAvailable() const override;
    float ReadAxis(EDroneInputAxis Axis, EThrottleCalibrationSpace Space) const override;
    void Tick() override;
    void Reset() override { Devices.Empty(); }
    void Analog(int32 Device, FKey Key, float Value);
    FDroneGamepadSettings Settings;
private:
    TMap<int32, TMap<FKey, float>> Devices;
};
