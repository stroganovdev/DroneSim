#include "DroneFlightSettings.h"
#include "DroneSimulatorInput/Public/DroneInputSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/UnrealType.h"
#include "DroneSimulatorGame/Gameplay/DroneMovementComponent.h"
#include "DroneSimulatorGame/Assets/DroneFrameAsset.h"
#include "DroneSimulatorGame/Assets/DroneMotorAsset.h"
#include "DroneSimulatorGame/Assets/DroneBatteryAsset.h"
#include "DroneSimulatorGame/Assets/DronePropellerAsset.h"
#include "DroneSimulatorCore/PropulsionModel/PropulsionModel.h"

namespace
{
    constexpr const TCHAR* DroneSettingsSaveSlot = TEXT("DroneFlightSettings");
    FFloatProperty* SettingField(FName Key)
    {
        return FindFProperty<FFloatProperty>(FDroneFlightSettings::StaticStruct(), Key);
    }
    void Number(UObject* Object, FName Key, double Value)
    {
        if (!Object) return;
        if (auto* DoubleField = FindFProperty<FDoubleProperty>(Object->GetClass(), Key)) DoubleField->SetPropertyValue_InContainer(Object, Value);
        else if (auto* FloatField = FindFProperty<FFloatProperty>(Object->GetClass(), Key)) FloatField->SetPropertyValue_InContainer(Object, Value);
    }
    void Vector(UObject* Object, FName Key, const FVector& Value)
    {
        if (auto* P = FindFProperty<FStructProperty>(Object->GetClass(), Key))
            if (P->Struct == TBaseStructure<FVector>::Get()) *P->ContainerPtrToValuePtr<FVector>(Object) = Value;
    }
}
const TArray<FDroneSettingDescriptor>& UDroneSettingsSubsystem::Descriptors()
{
    static const TArray<FDroneSettingDescriptor> Fields = {
        {TEXT("RollRate"), TEXT("Крен — максимум, °/с"), 0, 100, 1500, 10},
        {TEXT("PitchRate"), TEXT("Тангаж — максимум, °/с"), 0, 100, 1500, 10},
        {TEXT("YawRate"), TEXT("Поворот — максимум, °/с"), 0, 100, 1500, 10},
        {TEXT("CenterSensitivity"), TEXT("Чувствительность у центра"), 0, 10, 300, 5},
        {TEXT("Expo"), TEXT("Expo — мягкость у центра"), 0, 0, 1, 0.05f},
        {TEXT("MassGrams"), TEXT("Масса с батареей, г"), 1, 30, 120, 1},
        {TEXT("MotorResponseMs"), TEXT("Время отклика моторов, мс"), 1, 5, 100, 1},
        {TEXT("ThrustScale"), TEXT("Множитель тяги"), 1, 0.5f, 1.5f, 0.05f},
        {TEXT("DragScale"), TEXT("Сопротивление корпуса"), 1, 0.25f, 3, 0.1f},
        {TEXT("Voltage"), TEXT("Напряжение батареи 1S, В"), 1, 3.2f, 4.35f, 0.05f},
        {TEXT("CameraFov"), TEXT("Поле зрения, °"), 3, 60, 140, 5},
        {TEXT("CameraTilt"), TEXT("Поправка наклона FPV, °"), 3, -20, 40, 1},
        {TEXT("ChaseDistance"), TEXT("Дистанция внешней камеры, см"), 3, 20, 300, 10},
    };
    return Fields;
}
void UDroneSettingsSubsystem::Sanitize(FDroneFlightSettings& Settings)
{
    Settings.Gamepad.Sanitize();
    if (Settings.ControlSource.IsNone()) Settings.ControlSource = TEXT("Radio");
    const FDroneFlightSettings Defaults;
    for (const auto& D : Descriptors())
    {
        auto* Field = SettingField(D.Key);
        float Value = Field->GetPropertyValue_InContainer(&Settings);
        if (!FMath::IsFinite(Value)) Value = Field->GetPropertyValue_InContainer(&Defaults);
        Field->SetPropertyValue_InContainer(&Settings, FMath::Clamp(Value, D.Minimum, D.Maximum));
    }
    Settings.CenterSensitivity = FMath::Min(Settings.CenterSensitivity,
        FMath::Min3(Settings.RollRate, Settings.PitchRate, Settings.YawRate));
}
void UDroneSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if (UGameplayStatics::DoesSaveGameExist(DroneSettingsSaveSlot, 0))
    {
        if (auto* Saved = Cast<UDroneFlightSettingsSave>(UGameplayStatics::LoadGameFromSlot(DroneSettingsSaveSlot, 0)))
        {
            FutureVersion = Saved->Version > 1;
            if (!FutureVersion) Current = Saved->Settings;
            else LastError = TEXT("Сохранение создано более новой версией. Оно не будет перезаписано.");
        }
        else LastError = TEXT("Не удалось прочитать настройки. Использован стандартный профиль.");
    }
    Sanitize(Current);
    Draft = Current;
}
float UDroneSettingsSubsystem::GetDraftNumber(FName Key) const
{
    const auto* P = SettingField(Key);
    return P ? P->GetPropertyValue_InContainer(&Draft) : 0.f;
}
void UDroneSettingsSubsystem::SetDraftNumber(FName Key, float Value)
{
    for (const auto& D : Descriptors()) if (D.Key == Key)
    {
        if (!FMath::IsFinite(Value)) return;
        SettingField(Key)->SetPropertyValue_InContainer(&Draft, FMath::Clamp(Value, D.Minimum, D.Maximum));
        return;
    }
}
bool UDroneSettingsSubsystem::NeedsRestart() const
{
    for (const auto& D : Descriptors()) if (D.Section == 1)
        if (!FMath::IsNearlyEqual(SettingField(D.Key)->GetPropertyValue_InContainer(&Current), GetDraftNumber(D.Key))) return true;
    return false;
}
bool UDroneSettingsSubsystem::CommitDraft()
{
    if (FutureVersion) return false;
    auto* Input = GetGameInstance()->GetSubsystem<UDroneInputSubsystem>();
    bool Known = false;
    if (Input) for (const auto& Source : Input->GetControlSources()) Known |= Source.Id == Draft.ControlSource;
    if (!Known) { LastError = TEXT("Выбранный источник управления недоступен."); return false; }
    Sanitize(Draft);
    auto* Saved = Cast<UDroneFlightSettingsSave>(UGameplayStatics::CreateSaveGameObject(UDroneFlightSettingsSave::StaticClass()));
    Saved->Settings = Draft;
    if (!UGameplayStatics::SaveGameToSlot(Saved, DroneSettingsSaveSlot, 0))
    {
        LastError = TEXT("Не удалось сохранить настройки. Изменения не применены.");
        return false;
    }
    Current = Draft;
    LastError.Empty();
    return true;
}
void UDroneSettingsSubsystem::ApplyToPawn(APawn* Pawn)
{
    if (!Pawn) return;
    if (auto* Input = GetGameInstance()->GetSubsystem<UDroneInputSubsystem>())
    {
        Input->ConfigureGamepad(Current.Gamepad);
        if (!Input->SetControlSource(Current.ControlSource)) Input->ClearControlSource();
    }
    auto* Movement = Pawn->FindComponentByClass<UDroneMovementComponent>();
    if (!Movement) return;
    if (PreparedPawn.Get() != Pawn)
    {
        PreparedPawn = Pawn;
        RuntimeFrame = Movement->frame_asset ? DuplicateObject<UDroneFrameAsset>(Movement->frame_asset, Pawn) : nullptr;
        auto* Simple = Cast<UDronePropellerSimplifiedAsset>(Movement->propeller_asset);
        RuntimePropeller = Simple ? DuplicateObject<UDronePropellerSimplifiedAsset>(Simple, Pawn) : nullptr;
        RuntimeBattery = Movement->battery_asset ? DuplicateObject<UDroneBatteryAsset>(Movement->battery_asset, Pawn) : nullptr;
        if (RuntimeFrame)
        {
            BaseInertia = RuntimeFrame->inertia_override_kg_m2;
            BaseArea = RuntimeFrame->area_m2;
            BaseRotationalDrag = RuntimeFrame->rotational_drag_coefficient;
            BaseTotalMass = RuntimeFrame->mass_kg + (RuntimeBattery ? RuntimeBattery->mass_kg : 0.0)
                + (Movement->motor_asset ? 4.0 * Movement->motor_asset->mass_kg : 0.0);
        }
        if (RuntimePropeller) { BaseCt = RuntimePropeller->thrust_coefficient; BaseCq = RuntimePropeller->torque_coefficient; }
    }
    if (RuntimeFrame)
    {
        const double TotalMass = Current.MassGrams * 0.001;
        const double MotorsMass = Movement->motor_asset ? 4.0 * Movement->motor_asset->mass_kg : 0.0;
        RuntimeFrame->mass_kg = FMath::Max(0.001, TotalMass - MotorsMass - (RuntimeBattery ? RuntimeBattery->mass_kg : 0.0));
        RuntimeFrame->inertia_override_kg_m2 = BaseInertia * (TotalMass / FMath::Max(BaseTotalMass, 0.001));
        RuntimeFrame->area_m2 = BaseArea * Current.DragScale;
        RuntimeFrame->rotational_drag_coefficient = BaseRotationalDrag * Current.DragScale;
        Movement->frame_asset = RuntimeFrame;
    }
    if (RuntimePropeller)
    {
        RuntimePropeller->thrust_coefficient = BaseCt * Current.ThrustScale;
        RuntimePropeller->torque_coefficient = BaseCq * Current.ThrustScale;
        Movement->propeller_asset = RuntimePropeller;
    }
    if (RuntimeBattery)
    {
        RuntimeBattery->voltage = Current.Voltage;
        Movement->battery_asset = RuntimeBattery;
    }
    Number(Movement->propulsion_model, TEXT("motor_response_time_seconds"), Current.MotorResponseMs * 0.001);
    FName ChosenMode;
    for (const auto& Pair : Movement->flight_modes)
    {
        UObject* Mode = Pair.Value;
        if (!Mode) continue;
        const FString Class = Mode->GetClass()->GetName();
        if (Class == TEXT("FlightModeAir"))
        {
            Number(Mode, TEXT("roll_rate_deg_per_second"), Current.RollRate);
            Number(Mode, TEXT("pitch_rate_deg_per_second"), Current.PitchRate);
            Number(Mode, TEXT("yaw_rate_deg_per_second"), Current.YawRate);
            Vector(Mode, TEXT("actual_center_sensitivity"), FVector(Current.CenterSensitivity));
            Vector(Mode, TEXT("actual_expo"), FVector(Current.Expo));
            if (auto* P = FindFProperty<FBoolProperty>(Mode->GetClass(), TEXT("use_actual_rates"))) P->SetPropertyValue_InContainer(Mode, true);
            if (Current.Acro) ChosenMode = Pair.Key;
        }
        else if (Class == TEXT("FlightModeAngle"))
        {
            Number(Mode, TEXT("yaw_rate_deg_per_second"), Current.YawRate);
            if (!Current.Acro) ChosenMode = Pair.Key;
        }
    }
    if (!ChosenMode.IsNone()) Movement->set_active_flight_mode(ChosenMode);
    Movement->refresh_drone_configuration();
    TArray<UCameraComponent*> Cameras;
    Pawn->GetComponents(Cameras);
    for (auto* Camera : Cameras)
    {
        Camera->SetFieldOfView(Current.CameraFov);
        if (Camera->GetName().Contains(TEXT("FPV"))) Camera->SetRelativeRotation(FRotator(Current.CameraTilt, 0, 0));
    }
    TArray<USpringArmComponent*> Arms;
    Pawn->GetComponents(Arms);
    for (auto* Arm : Arms) if (Arm->GetName().Contains(TEXT("3P"))) Arm->TargetArmLength = Current.ChaseDistance;
}
