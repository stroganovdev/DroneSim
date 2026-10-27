#include "DroneInputSubsystem.h"
#include "DroneControlSources.h"
#include "DroneInputCalibrator.h"
#include "DroneInputProcessor.h"
#include "DroneInputProfileManager.h"
#include "DroneSimulatorInput.h"
#include "Framework/Application/SlateApplication.h"
#if PLATFORM_WINDOWS
#include "Windows/HIDInputManager.h"
#elif PLATFORM_MAC
#include "Mac/HIDInputManager.h"
#endif

namespace
{
	static constexpr float SIGNED_AXIS_DEADZONE_LOW_PRECISION = 0.05f;
	static constexpr float SIGNED_AXIS_DEADZONE_HIGH_PRECISION = 0.01f;

	static float get_signed_axis_deadzone(EDroneInputPrecisionMode precision_mode)
	{
		return precision_mode == EDroneInputPrecisionMode::HighPrecision
			? SIGNED_AXIS_DEADZONE_HIGH_PRECISION
			: SIGNED_AXIS_DEADZONE_LOW_PRECISION;
	}
} // namespace

void UDroneInputSubsystem::Initialize(FSubsystemCollectionBase &Collection) {
	Super::Initialize(Collection);

	this->profile_manager = MakeShared<FDroneInputProfileManager>();
	this->profile_manager->load_profiles();
	this->calibrator =
			MakeShared<FDroneInputCalibrator>(this, this->profile_manager);

	this->input_processor = MakeShared<FDroneInputProcessor>(this);
	if (FSlateApplication::IsInitialized()) {
		FSlateApplication::Get().RegisterInputPreProcessor(this->input_processor);
	}

    RegisterControlSource(MakeShared<FDroneRadioSource>(this));
    GamepadSource = MakeShared<FDroneGamepadSource>();
    RegisterControlSource(GamepadSource.ToSharedRef());
	FHIDInputManager::get().initialize();
	UE_LOG(LogDroneSimulatorInput, Log, TEXT("UDroneInputManager::Initialize - Input System Initialized"));
}

void UDroneInputSubsystem::Deinitialize() {
	FHIDInputManager::get().shutdown();

	if (FSlateApplication::IsInitialized() && this->input_processor.IsValid()) {
		FSlateApplication::Get().UnregisterInputPreProcessor(this->input_processor);
	}
	ControlSources.Empty();
    GamepadSource.Reset();
	this->input_processor.Reset();
	this->calibrator.Reset();
	this->profile_manager.Reset();

	Super::Deinitialize();
	UE_LOG(LogDroneSimulatorInput, Log,
				 TEXT("UDroneInputManager::Deinitialize - Input System Deinitialized"));
}

void UDroneInputSubsystem::Tick(float DeltaTime) {
	// Also clear raw device data to prevent stuck values if device stops sending
	// updates
	for (auto &device_pair : devices) {
		device_pair.Value.raw_axes.Empty();
		device_pair.Value.raw_int_axes.Empty();
	}

    if (calibrator.IsValid()) FHIDInputManager::get().poll_devices(this);
    for (const auto& Source : ControlSources) Source->Tick();
    const auto* Source = FindControlSource(ActiveControlSource);
    const bool Available = Source && Source->IsAvailable();
    if (!Available || !SourceWasAvailable) WaitingForNeutral = true;
    SourceWasAvailable = Available;
    if (Available && WaitingForNeutral)
    {
        const bool Neutral = Source->ReadAxis(EDroneInputAxis::Throttle, EThrottleCalibrationSpace::PositiveOnly) <= 0.05f
            && FMath::Abs(Source->ReadAxis(EDroneInputAxis::Yaw, EThrottleCalibrationSpace::PositiveOnly)) <= 0.05f
            && FMath::Abs(Source->ReadAxis(EDroneInputAxis::Pitch, EThrottleCalibrationSpace::PositiveOnly)) <= 0.05f
            && FMath::Abs(Source->ReadAxis(EDroneInputAxis::Roll, EThrottleCalibrationSpace::PositiveOnly)) <= 0.05f;
        if (Neutral) WaitingForNeutral = false;
    }
}

static float apply_unsigned_axis_invert(float calibrated_value,
																				bool is_inverted) {
	return is_inverted ? 1.f - calibrated_value : calibrated_value;
}

static float apply_signed_axis_invert(float calibrated_value,
																			bool is_inverted) {
	return is_inverted ? -calibrated_value : calibrated_value;
}

static float
compute_unsigned_throttle_value(float raw_value,
																const FAxisCalibrationData *calibration) {
	if (calibration == nullptr) {
		return 0.f;
	}

	const float range = calibration->max_value - calibration->zero_value;
	if (FMath::IsNearlyZero(range)) {
		return 0.f;
	}

	const float processed_value = (raw_value - calibration->zero_value) / range;
	return FMath::Clamp(processed_value, 0.f, 1.f);
}

static float
compute_signed_axis_value(float raw_value, const FAxisCalibrationData *calibration, float signed_axis_deadzone) {
	if (calibration == nullptr) {
		return 0.f;
	}

	float processed_value = raw_value;
	const float range = calibration->max_value - calibration->min_value;

	if (!FMath::IsNearlyZero(range)) {
		processed_value = (raw_value - calibration->min_value) / range;
		processed_value = FMath::Clamp(processed_value, 0.f, 1.f);
	}

	float center_norm = 0.5f;
	if (!FMath::IsNearlyZero(range)) {
		const float clamped_center =
				FMath::Clamp(calibration->zero_value, calibration->min_value,
										 calibration->max_value);
		center_norm = FMath::Clamp(
				(clamped_center - calibration->min_value) / range, 0.f, 1.f);
	}

	const float lower_span = FMath::Max(center_norm, KINDA_SMALL_NUMBER);
	const float upper_span = FMath::Max(1.0f - center_norm, KINDA_SMALL_NUMBER);

	float axis_value = 0.f;
	if (processed_value < center_norm) {
		axis_value = (processed_value - center_norm) / lower_span;
	} else {
		axis_value = (processed_value - center_norm) / upper_span;
	}

	if (FMath::Abs(axis_value) < signed_axis_deadzone) {
		axis_value = 0.f;
	}

	return FMath::Clamp(axis_value, -1.0f, 1.0f);
}

static float compute_signed_throttle_value(float raw_value, const FAxisCalibrationData *calibration,
	float signed_axis_deadzone)
{
	if (calibration == nullptr) {
		return 0.f;
	}

	const float range = calibration->max_value - calibration->min_value;

	// If using a controller with a zero value that's the min value as well, we're
	// in a drone style controller, with no auto centering
	if (!FMath::IsNearlyZero(range) && FMath::Abs(calibration->zero_value - calibration->min_value) / range < 0.25f)
	{
		const auto unclamped = (raw_value - calibration->min_value) / range * 2.f - 1.f;

		float val = FMath::Clamp(unclamped, -1.f, 1.f);
		if (FMath::Abs(val) < signed_axis_deadzone) {
			val = 0.f;
		}

		return val;
	}

	return compute_signed_axis_value(raw_value, calibration, signed_axis_deadzone);
}

static float calibrate_axis_value(EDroneInputAxis axis, float raw_value, const FAxisCalibrationData *calibration,
	bool is_inverted, EThrottleCalibrationSpace throttle_calibration_space, float signed_axis_deadzone)
{
	if (axis == EDroneInputAxis::Throttle)
	{
		if (throttle_calibration_space == EThrottleCalibrationSpace::PositiveNegative) {
			const float signed_value = compute_signed_throttle_value(raw_value, calibration, signed_axis_deadzone);
			return apply_signed_axis_invert(signed_value, is_inverted);
		}

		const float unsigned_value = compute_unsigned_throttle_value(raw_value, calibration);
		return apply_unsigned_axis_invert(unsigned_value, is_inverted);
	}

	const float signed_value = compute_signed_axis_value(raw_value, calibration, signed_axis_deadzone);
	return apply_signed_axis_invert(signed_value, is_inverted);
}

void UDroneInputSubsystem::handle_raw_analog_input(int32 device_id, FKey key,
																									 float analog_value) {
    if (key.IsGamepadKey())
    {
        if (GamepadSource) GamepadSource->Analog(device_id, key, analog_value);
        return; // Standard gamepads never participate in HID calibration.
    }
	if (!calibrator.IsValid()) return;

	// 1. Update Device Raw Data
	register_device(device_id);
	FDroneInputDevice &device = devices.FindOrAdd(device_id);
	device.raw_axes.Add(key.GetFName(), analog_value);

	// Calibration Logic
	bool handled_by_calibration =
			calibrator->handle_raw_input(device_id, key, analog_value);

	// If calibration is active, we don't want to process the input for game
	// control
	if (handled_by_calibration ||
			calibrator->get_calibration_phase() != EDroneCalibrationPhase::None) {
		return;
	}
}

void UDroneInputSubsystem::update_raw_int_axis_value(int32 device_id,
																										 FName axis_name,
																										 int32 raw_value) {
	if (FDroneInputDevice *device = devices.Find(device_id)) {
		device->raw_int_axes.Add(axis_name, raw_value);
	}
}

void UDroneInputSubsystem::handle_key_down(int32 device_id, FKey key) {
	register_device(device_id);
}

void UDroneInputSubsystem::handle_key_up(int32 device_id, FKey key) {
	register_device(device_id);
}

void UDroneInputSubsystem::handle_device_disconnected(int32 device_id) {
	if (calibrator->get_calibrating_device_id() == device_id) {
		reset_calibration_state_for_device(device_id);
	}

	if (devices.Contains(device_id)) {
		FDroneInputDevice device = devices[device_id];
		devices.Remove(device_id);
	} else {
		for (const auto &pair : devices) {
			UE_LOG(LogDroneSimulatorInput, Warning, TEXT(" - %d"), pair.Key);
		}
	}
}

void UDroneInputSubsystem::reset_calibration_state_for_device(int32 device_id) {
	calibrator->on_device_disconnected(device_id);

	device_mappings.Remove(device_id);
	calibration_data.Remove(device_id);
}

void UDroneInputSubsystem::register_device(int32 device_id,
																					 const FString &device_name,
																					 const FString &device_uid) {
	// TODO: remove devices

	const bool was_present = devices.Contains(device_id);

	FDroneInputDevice &device = devices.FindOrAdd(device_id);
	device.device_id = device_id;

	if (!device_name.IsEmpty()) {
		device.device_name = device_name;
	} else if (device.device_name.IsEmpty()) {
		device.device_name = FString::Printf(TEXT("Device %d"), device_id);
	}

	if (!device_uid.IsEmpty()) {
		device.device_uid = device_uid;
	}

	if (!was_present) {
		on_input_device_detected_native.Broadcast(device);
		UE_LOG(LogDroneSimulatorInput, Log,
					 TEXT("UDroneInputManager::RegisterDevice - New Device Registered: "
								"Id=%d, Name=%s"),
					 device_id, *device.device_name);
		if (profile_manager.IsValid()) {
			profile_manager->apply_saved_profile_if_any(
					device, device_id, device_mappings, calibration_data);
		}
	}
}

TArray<FDroneInputDevice> UDroneInputSubsystem::get_all_devices() const {
	TArray<FDroneInputDevice> result;
	devices.GenerateValueArray(result);
	return result;
}

FDroneInputDevice UDroneInputSubsystem::get_device(int32 device_id) const {
	if (const FDroneInputDevice *device = devices.Find(device_id)) {
		return *device;
	}
	return FDroneInputDevice();
}

void UDroneInputSubsystem::set_axis_mapping(int32 device_id, FName axis_name,
																						EDroneInputAxis game_axis) {
	FInputAxisMapping &mapping =
			device_mappings.FindOrAdd(device_id).FindOrAdd(axis_name);
	mapping.input_device_axis_name = axis_name;
	mapping.game_axis = game_axis;
	UE_LOG(LogDroneSimulatorInput, Log,
				 TEXT("UDroneInputManager::SetAxisMapping - Mapped DeviceId=%d, "
							"Axis=%s to GameAxis=%d"),
				 device_id, *axis_name.ToString(), (int32)game_axis);
	if (profile_manager.IsValid()) {
		if (const FDroneInputDevice *device = devices.Find(device_id)) {
			const TMap<FName, FInputAxisMapping> *mappings =
					device_mappings.Find(device_id);
			const TMap<FName, FAxisCalibrationData> *calibrations =
					calibration_data.Find(device_id);
			profile_manager->update_saved_profile_for_device(*device, mappings,
																											 calibrations);
			profile_manager->save_profiles();
		}
	}
}

float UDroneInputSubsystem::ReadRadioAxis(
		EDroneInputAxis axis,
		EThrottleCalibrationSpace throttle_calibration_space) const {
	if (!calibrator.IsValid() ||
			calibrator->get_calibration_phase() != EDroneCalibrationPhase::None) {
		return 0.0f;
	}

    const int32 SelectedRadio = GetRadioDeviceId();
	for (const auto &device_mapping_pair : device_mappings) {
		const int32 device_id = device_mapping_pair.Key;
        if (device_id != SelectedRadio) continue;
		const TMap<FName, FInputAxisMapping> &mappings = device_mapping_pair.Value;

		for (const auto &mapping_pair : mappings) {
			const FInputAxisMapping &mapping = mapping_pair.Value;
			if (mapping.game_axis != axis) {
				continue;
			}

			if (const FDroneInputDevice *device = devices.Find(device_id)) {
				if (const float *raw_value =
								device->raw_axes.Find(mapping.input_device_axis_name)) {
					const FAxisCalibrationData *calibration = nullptr;
					if (const TMap<FName, FAxisCalibrationData> *device_calibration =
									calibration_data.Find(device_id)) {
						calibration =
								device_calibration->Find(mapping.input_device_axis_name);
					}

					return calibrate_axis_value(axis, *raw_value, calibration, mapping.is_inverted,
						throttle_calibration_space, get_signed_axis_deadzone(precision_mode));
				}
			}
		}
	}

	return 0.f;
}

float UDroneInputSubsystem::get_raw_axis_value(int32 device_id, FName axis_name) const
{
	if (const FDroneInputDevice *device = devices.Find(device_id))
	{
		if (const float *val = device->raw_axes.Find(axis_name))
		{
			return *val;
		}
	}
	return 0.0f;
}

bool UDroneInputSubsystem::is_device_calibrated(int32 device_id) const {
	return calibration_data.Contains(device_id);
}

void UDroneInputSubsystem::remove_device_calibration(int32 device_id) {
	reset_calibration_state_for_device(device_id);

	if (profile_manager.IsValid()) {
		if (const FDroneInputDevice *device = devices.Find(device_id)) {
			profile_manager->remove_profile_for_device(*device);
		}
	}

	UE_LOG(LogDroneSimulatorInput, Log,
				 TEXT("UDroneInputSubsystem::remove_device_calibration - Removed "
							"calibration for DeviceId=%d"),
				 device_id);
}

bool UDroneInputSubsystem::is_axis_inverted(int32 device_id,
																						EDroneInputAxis axis) const {
	if (const TMap<FName, FInputAxisMapping> *mappings =
					device_mappings.Find(device_id)) {
		for (const auto &mapping_pair : *mappings) {
			if (mapping_pair.Value.game_axis == axis) {
				return mapping_pair.Value.is_inverted;
			}
		}
	}
	return false;
}

void UDroneInputSubsystem::set_axis_inverted(int32 device_id,
																						 EDroneInputAxis axis,
																						 bool inverted) {
	if (TMap<FName, FInputAxisMapping> *mappings =
					device_mappings.Find(device_id)) {
		for (auto &mapping_pair : *mappings) {
			if (mapping_pair.Value.game_axis == axis) {
				mapping_pair.Value.is_inverted = inverted;

				// Save profile
				if (profile_manager.IsValid()) {
					if (const FDroneInputDevice *device = devices.Find(device_id)) {
						const TMap<FName, FAxisCalibrationData> *calibrations =
								calibration_data.Find(device_id);
						profile_manager->update_saved_profile_for_device(*device, mappings,
																														 calibrations);
						profile_manager->save_profiles();
					}
				}
				return;
			}
		}
	}
}

void UDroneInputSubsystem::set_precision_mode(EDroneInputPrecisionMode in_precision_mode)
{
	this->precision_mode = in_precision_mode;
}

EDroneInputPrecisionMode UDroneInputSubsystem::get_precision_mode() const
{
	return this->precision_mode;
}

const IDroneControlSource* UDroneInputSubsystem::FindControlSource(FName Id) const
{
    for (const auto& Source : ControlSources) if (Source->GetId() == Id) return Source.Get();
    return nullptr;
}
bool UDroneInputSubsystem::RegisterControlSource(TSharedRef<IDroneControlSource> Source)
{
    if (Source->GetId().IsNone() || FindControlSource(Source->GetId())) return false;
    ControlSources.Add(Source); return true;
}
TArray<FDroneControlSourceInfo> UDroneInputSubsystem::GetControlSources() const
{
    TArray<FDroneControlSourceInfo> Result;
    for (const auto& Source : ControlSources)
    {
        FDroneControlSourceInfo Info; Info.Id = Source->GetId(); Info.Label = Source->GetLabel(); Result.Add(Info);
    }
    return Result;
}
bool UDroneInputSubsystem::SetControlSource(FName Id)
{
    const auto* Source = FindControlSource(Id);
    if (!Source) return false;
    if (ActiveControlSource != Id)
    {
        ActiveControlSource = Id; WaitingForNeutral = true; SourceWasAvailable = false;
        for (const auto& Item : ControlSources) if (Item->GetId() == Id) Item->Reset();
    }
    return true;
}
void UDroneInputSubsystem::ConfigureGamepad(FDroneGamepadSettings Settings)
{
    Settings.Sanitize();
    if (GamepadSource && !(GamepadSource->Settings == Settings))
    {
        GamepadSource->Settings = Settings; GamepadSource->Reset();
        if (ActiveControlSource == TEXT("Gamepad")) WaitingForNeutral = true;
    }
}
int32 UDroneInputSubsystem::GetRadioDeviceId() const
{
    int32 Selected = MAX_int32;
    for (const auto& Pair : devices)
        if (!Pair.Value.device_uid.IsEmpty() && is_device_calibrated(Pair.Key)) Selected = FMath::Min(Selected, Pair.Key);
    return Selected == MAX_int32 ? INDEX_NONE : Selected;
}
float UDroneInputSubsystem::get_calibrated_axis_value(EDroneInputAxis Axis, EThrottleCalibrationSpace Space) const
{
    const auto* Source = FindControlSource(ActiveControlSource);
    if (!Source || !Source->IsAvailable() || WaitingForNeutral)
        return Axis == EDroneInputAxis::Throttle && Space == EThrottleCalibrationSpace::PositiveNegative ? -1.f : 0.f;
    const float Value = Source->ReadAxis(Axis, Space);
    if (!FMath::IsFinite(Value)) return Axis == EDroneInputAxis::Throttle && Space == EThrottleCalibrationSpace::PositiveNegative ? -1.f : 0.f;
    return FMath::Clamp(Value, Axis == EDroneInputAxis::Throttle && Space == EThrottleCalibrationSpace::PositiveOnly ? 0.f : -1.f, 1.f);
}
float UDroneInputSubsystem::PreviewControlAxis(FName Id, EDroneInputAxis Axis) const
{
    const auto* Source = FindControlSource(Id);
    return Source && Source->IsAvailable() ? Source->ReadAxis(Axis, EThrottleCalibrationSpace::PositiveOnly) : 0.f;
}
FString UDroneInputSubsystem::GetControlSourceStatus(FName Id) const
{
    const auto* Source = FindControlSource(Id);
    if (!Source) return TEXT("Источник управления недоступен.");
    FString Result = Source->GetLabel().ToString();
    if (!Source->IsAvailable()) return Result + TEXT(" — нет данных. Подключите устройство; для Xbox пошевелите стик.");
    Result += TEXT(" — подключён");
    if (Id == TEXT("Radio")) Result += TEXT(" • ") + get_device(GetRadioDeviceId()).device_name;
    if (Id == ActiveControlSource && WaitingForNeutral) Result += TEXT(". Газ в ноль, стики в центр для включения управления.");
    return Result;
}
