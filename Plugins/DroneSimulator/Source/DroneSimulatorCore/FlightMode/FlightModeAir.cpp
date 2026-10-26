#include "DroneSimulatorCore/FlightMode/FlightModeAir.h"


ECameraTiltMode UFlightModeAir::get_camera_tilt_mode() const
{
    return ECameraTiltMode::Tilted;
}

EThrottleCalibrationSpace UFlightModeAir::get_throttle_calibration_space() const
{
    return EThrottleCalibrationSpace::PositiveOnly;
}

namespace
{
    // Same input/output curve as Betaflight 4.5.3 applyActualRates:
    // https://github.com/betaflight/betaflight/blob/4.5.3/src/main/fc/rc.c
    // Maximum rates use the existing per-axis rate properties below.
    double actual_rate(double stick, double center, double maximum, double expo)
    {
        stick = FMath::Clamp(stick, -1.0, 1.0);
        center = FMath::Clamp(center, 0.0, 2000.0);
        expo = FMath::Clamp(expo, 0.0, 1.0);
        const double stick_squared = stick * stick;
        const double curve = FMath::Abs(stick) *
            (stick * stick_squared * stick_squared * expo + stick * (1.0 - expo));
        return FMath::Clamp(stick * center + FMath::Max(0.0, maximum - center) * curve, -1998.0, 1998.0);
    }
}

FDroneSetpoint UFlightModeAir::compute_setpoint(const FDronePlayerInput& player_input, const FFlightModeState& flight_state)
{
    if (use_actual_rates)
    {
        const FVector rates(
            -actual_rate(player_input.roll, actual_center_sensitivity.X, roll_rate_deg_per_second, actual_expo.X),
            -actual_rate(player_input.pitch, actual_center_sensitivity.Y, pitch_rate_deg_per_second, actual_expo.Y),
            actual_rate(player_input.yaw, actual_center_sensitivity.Z, yaw_rate_deg_per_second, actual_expo.Z));
        return FDroneSetpoint(player_input.throttle, rates * (PI / 180.0));
    }

    const auto desired_angular_velocity = FVector(
        player_input.roll * -FMath::DegreesToRadians(this->roll_rate_deg_per_second), // Roll stick right = Input.Roll >0 = drone rotates clockwise = rotator roll >0
        player_input.pitch * -FMath::DegreesToRadians(this->pitch_rate_deg_per_second), // Pitch stick up = Input.Pitch >0 = look down = rotator pitch <0
        player_input.yaw * FMath::DegreesToRadians(this->yaw_rate_deg_per_second) // Yaw stick right = Input.Yaw >0 = look right = rotator yaw >0
    );

    return FDroneSetpoint(player_input.throttle, desired_angular_velocity);
}
