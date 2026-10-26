#pragma once

#include "DroneSimulatorCore/FlightMode/FlightMode.h"

#include "FlightModeAir.generated.h"

UCLASS(BlueprintType)
class UFlightModeAir : public UFlightModeBase
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Roll rate (degrees per second)"))
    double roll_rate_deg_per_second = 500.0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Pitch rate (degrees per second)"))
    double pitch_rate_deg_per_second = 500.0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Yaw rate (degrees per second)"))
    double yaw_rate_deg_per_second = 500.0;

    // Disabled by default to retain the response of existing Air-mode assets.
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates|Actual")
    bool use_actual_rates = false;

    // X = roll, Y = pitch, Z = yaw. Betaflight Actual center sensitivity, in deg/s.
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates|Actual", meta=(ClampMin="0.0", ClampMax="2000.0"))
    FVector actual_center_sensitivity = FVector(70.0, 70.0, 70.0);

    // X = roll, Y = pitch, Z = yaw. Normalized expo (0..1).
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates|Actual", meta=(ClampMin="0.0", ClampMax="1.0"))
    FVector actual_expo = FVector::ZeroVector;

    virtual ECameraTiltMode get_camera_tilt_mode() const override;

#ifdef WITH_DRONE_INPUT
    virtual EThrottleCalibrationSpace get_throttle_calibration_space() const override;
#endif

    virtual FDroneSetpoint compute_setpoint(const FDronePlayerInput& player_input, const FFlightModeState& flight_state) override;

};
