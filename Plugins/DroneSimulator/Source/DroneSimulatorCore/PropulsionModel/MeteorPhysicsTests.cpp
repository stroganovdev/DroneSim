#if WITH_DEV_AUTOMATION_TESTS
#include "DroneSimulatorCore/PropulsionModel/PropulsionModelDynamics.h"
#include "DroneSimulatorCore/Controller/PidDroneController.h"
#include "DroneSimulatorCore/Controller/Setpoint.h"
#include "DroneSimulatorCore/RotorModel/RotorModelSimplified.h"
#include "DroneSimulatorCore/Simulation/SubstepBody.h"
#include "DroneSimulatorCore/Simulation/SimulationWorld.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMeteorPhysicsTest, "DroneSimulator.Physics.Meteor75Pro",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMeteorPhysicsTest::RunTest(const FString& Parameters)
{
    FDroneFrame frame;
    frame.props_extent_front = FVector(2.8567114, 2.8567114, 0.0);
    frame.props_extent_back = FVector(-2.8567114, 2.8567114, 0.0);
    FDroneMotor motor; motor.kv = 22000.0 * (2.0 * PI / 60.0); motor.mass = 0.00279;
    FDroneBattery battery; battery.voltage = 3.8; battery.mass = 0.014;
    FDronePropellerSimplified prop;
    prop.blade_diameter = 0.045; prop.thrust_coefficient = 0.06733038547288748; prop.torque_coefficient = 0.005;
    TDronePropeller variant(TInPlaceType<FDronePropellerSimplified>{}, prop);
    const FPropulsionDroneSetup setup(&frame, &motor, &battery, &variant);
    auto* world = NewObject<USimulationWorld>();
    const FVector inertia(0.000021, 0.000025, 0.000042);
    auto* rotor = NewObject<URotorModelSimplified>();
    FSubstepBody body(FVector::ZeroVector, FQuat::Identity, 0.045, inertia, FVector::ZeroVector, FVector::ZeroVector);
    auto full = rotor->simulate_propeller_rotor(&body, 1.0, &variant, &motor, &battery, FVector::ZeroVector, true, world);
    TestTrue(TEXT("Estimated full thrust per 45 mm rotor is 0.65..0.67 N"), full.value.thrust > 0.65 && full.value.thrust < 0.67);
    auto half = rotor->simulate_propeller_rotor(&body, 0.5, &variant, &motor, &battery, FVector::ZeroVector, true, world);
    TestTrue(TEXT("Half motor speed gives quarter static thrust"), FMath::IsNearlyEqual(half.value.thrust * 4.0, full.value.thrust, 1.e-6));
    auto make_model = []()
    {
        auto* model = NewObject<UPropulsionModelDynamics>();
        auto* pid = NewObject<UPidDroneController>(model);
        pid->roll_pid = FPidConfig{0.009, 0.02, 0.0};
        pid->pitch_pid = FPidConfig{0.0107, 0.02, 0.0};
        pid->yaw_pid = FPidConfig{0.07, 0.05, 0.0};
        model->drone_controller = pid;
        model->rotor_model = NewObject<URotorModelSimplified>(model);
        model->motor_response_time_seconds = 0.025;
        return model;
    };
    auto* model = make_model();
    FSubstepBody climb(FVector::ZeroVector, FQuat::Identity, 0.045, inertia, FVector::ZeroVector, FVector::ZeroVector);
    double height = 0.0;
    for (int i = 0; i < 400; ++i)
    {
        model->tick_propulsion(0.0025, &climb, FDroneSetpoint(0.6, FVector::ZeroVector), setup, world);
        climb.add_force(FVector(0, 0, -0.045 * 9.81));
        climb.consume_forces_and_torques(0.0025);
        height += climb.linear_velocity_world.Z * 0.0025;
    }
    TestTrue(TEXT("60 percent throttle has prompt upward acceleration"), climb.linear_velocity_world.Z > 8.0 && climb.linear_velocity_world.Z < 13.0);
    TestTrue(TEXT("First second of climb is several meters, without artificial speed cap"), height > 3.0 && height < 7.0);
    TestTrue(TEXT("Symmetric climb does not introduce rotation"), climb.angular_velocity_radians_world.IsNearlyZero(0.001));
    const double hover = FMath::Sqrt(0.045 * 9.81 / (4.0 * full.value.thrust));
    TestTrue(TEXT("Hover throttle within plausible whoop range"), hover > 0.35 && hover < 0.45);
    for (int axis = 0; axis < 3; ++axis)
    {
        auto* controlled = make_model();
        FSubstepBody turning(FVector::ZeroVector, FQuat::Identity, 0.045, inertia, FVector::ZeroVector, FVector::ZeroVector);
        FVector target = FVector::ZeroVector; target[axis] = 3.0;
        for (int i = 0; i < 400; ++i)
        {
            controlled->tick_propulsion(0.0025, &turning, FDroneSetpoint(hover, target), setup, world);
            turning.consume_forces_and_torques(0.0025);
        }
        TestTrue(FString::Printf(TEXT("Axis %d rate feedback converges with correct sign"), axis),
            FMath::IsFinite(turning.angular_velocity_radians_world[axis]) && FMath::Abs(turning.angular_velocity_radians_world[axis] - 3.0) < 0.5);
    }
    return true;
}
#endif
