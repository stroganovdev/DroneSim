#include "DroneControlSources.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericApplication.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"

TArray<FKey> FDroneGamepadSettings::AxisKeys()
{
    return {EKeys::Gamepad_LeftX, EKeys::Gamepad_LeftY, EKeys::Gamepad_RightX,
        EKeys::Gamepad_RightY, EKeys::Gamepad_LeftTriggerAxis, EKeys::Gamepad_RightTriggerAxis};
}
void FDroneGamepadSettings::Sanitize()
{
    const FDroneGamepadSettings Defaults;
    const auto Allowed = AxisKeys();
    if (!Allowed.Contains(Throttle)) Throttle = Defaults.Throttle;
    if (!Allowed.Contains(Yaw)) Yaw = Defaults.Yaw;
    if (!Allowed.Contains(Pitch)) Pitch = Defaults.Pitch;
    if (!Allowed.Contains(Roll)) Roll = Defaults.Roll;
    Deadzone = FMath::IsFinite(Deadzone) ? FMath::Clamp(Deadzone, 0.f, 0.4f) : Defaults.Deadzone;
}
float FDroneGamepadSettings::Normalize(float Value, bool ThrottleAxis) const
{
    if (!FMath::IsFinite(Value)) return 0.f;
    const float D = FMath::Clamp(Deadzone, 0.f, 0.4f);
    Value = FMath::Clamp(Value, ThrottleAxis ? 0.f : -1.f, 1.f);
    return FMath::Sign(Value) * FMath::Max(0.f, FMath::Abs(Value) - D) / (1.f - D);
}
bool FDroneGamepadSettings::operator==(const FDroneGamepadSettings& Other) const
{
    return Throttle == Other.Throttle && Yaw == Other.Yaw && Pitch == Other.Pitch && Roll == Other.Roll
        && Deadzone == Other.Deadzone && InvertPitch == Other.InvertPitch;
}
namespace
{
    bool GamepadAttached()
    {
        return FSlateApplication::IsInitialized() && FSlateApplication::Get().GetPlatformApplication().IsValid()
            && FSlateApplication::Get().GetPlatformApplication()->IsGamepadAttached();
    }
}
bool FDroneGamepadSource::IsAvailable() const { return GamepadAttached() && !Devices.IsEmpty(); }
void FDroneGamepadSource::Analog(int32 Device, FKey Key, float Value)
{
    if (Key.IsGamepadKey() && Key.IsAnalog()) Devices.FindOrAdd(Device).Add(Key, FMath::IsFinite(Value) ? Value : 0.f);
}
void FDroneGamepadSource::Tick()
{
    if (!GamepadAttached() || !FSlateApplication::Get().IsActive()) { Reset(); return; }
    for (auto It = Devices.CreateIterator(); It; ++It)
        if (IPlatformInputDeviceMapper::Get().GetInputDeviceConnectionState(FInputDeviceId::CreateFromInternalId(It.Key())) == EInputDeviceConnectionState::Disconnected)
            It.RemoveCurrent();
}
float FDroneGamepadSource::ReadAxis(EDroneInputAxis Axis, EThrottleCalibrationSpace Space) const
{
    const float Zero = Axis == EDroneInputAxis::Throttle && Space == EThrottleCalibrationSpace::PositiveNegative ? -1.f : 0.f;
    if (!IsAvailable()) return Zero;
    int32 Selected = MAX_int32;
    for (const auto& Device : Devices) Selected = FMath::Min(Selected, Device.Key);
    const auto* Values = Devices.Find(Selected);
    const FKey Key = Axis == EDroneInputAxis::Throttle ? Settings.Throttle : Axis == EDroneInputAxis::Yaw ? Settings.Yaw
        : Axis == EDroneInputAxis::Pitch ? Settings.Pitch : Settings.Roll;
    float Value = Settings.Normalize(Values ? Values->FindRef(Key) : 0.f, Axis == EDroneInputAxis::Throttle);
    if (Axis == EDroneInputAxis::Pitch && Settings.InvertPitch) Value = -Value;
    return Axis == EDroneInputAxis::Throttle && Space == EThrottleCalibrationSpace::PositiveNegative ? Value * 2.f - 1.f : Value;
}
