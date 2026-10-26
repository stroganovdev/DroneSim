#if WITH_DEV_AUTOMATION_TESTS
#include "DroneSimulatorCore/FlightMode/FlightModeAir.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDroneActualRatesTest, "DroneSimulator.Input.Meteor75ActualRates",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDroneActualRatesTest::RunTest(const FString& Parameters)
{
    auto* mode = NewObject<UFlightModeAir>();
    FDronePlayerInput input;
    input.throttle = 0.4;
    input.roll = input.pitch = input.yaw = 0.5;
    const FFlightModeState state;
    const auto legacy = mode->compute_setpoint(input, state);
    TestTrue(TEXT("Existing Air assets keep their linear curve"),
        FMath::IsNearlyEqual(FMath::RadiansToDegrees(legacy.angular_velocity_radians.Z), 250.0, 0.0001));
    mode->use_actual_rates = true;
    mode->roll_rate_deg_per_second = mode->pitch_rate_deg_per_second = mode->yaw_rate_deg_per_second = 670.0;
    const double sticks[] = {-1.0, -0.5, -0.25, 0.0, 0.25, 0.5, 1.0};
    const double expected[] = {-670.0, -185.0, -55.0, 0.0, 55.0, 185.0, 670.0};
    for (int i = 0; i < UE_ARRAY_COUNT(sticks); ++i)
    {
        input.roll = input.pitch = input.yaw = sticks[i];
        const auto result = mode->compute_setpoint(input, state);
        const auto degrees = result.angular_velocity_radians * (180.0 / PI);
        TestTrue(FString::Printf(TEXT("Roll at %g"), sticks[i]), FMath::IsNearlyEqual(degrees.X, -expected[i], 0.0001));
        TestTrue(FString::Printf(TEXT("Pitch at %g"), sticks[i]), FMath::IsNearlyEqual(degrees.Y, -expected[i], 0.0001));
        TestTrue(FString::Printf(TEXT("Yaw at %g"), sticks[i]), FMath::IsNearlyEqual(degrees.Z, expected[i], 0.0001));
        TestEqual(TEXT("Rates do not alter throttle"), result.throttle, 0.4);
    }
    mode->actual_expo = FVector(1.0);
    input.roll = input.pitch = input.yaw = 0.5;
    const auto expo = mode->compute_setpoint(input, state);
    TestTrue(TEXT("Actual expo curve at half stick"),
        FMath::IsNearlyEqual(FMath::RadiansToDegrees(expo.angular_velocity_radians.Z), 44.375, 0.0001));
    return true;
}
#endif
