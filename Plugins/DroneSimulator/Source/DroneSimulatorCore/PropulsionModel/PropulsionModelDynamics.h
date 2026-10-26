#pragma once

#include "DroneSimulatorCore/PropulsionModel/PropulsionModel.h"
#include "DroneSimulatorCore/Controller/Throttle.h"

#include "PropulsionModelDynamics.generated.h"

class URotorModelBase;
class UDroneController;

UCLASS()
class UPropulsionModelDynamics : public UPropulsionModel
{
    GENERATED_BODY()

public:

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Controller"))
    UDroneController* drone_controller;

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Rotor model"))
    URotorModelBase* rotor_model;

    // First-order shaft-speed response. Zero retains instantaneous legacy motors.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(ClampMin="0.0"))
    double motor_response_time_seconds = 0.0;

    UPROPERTY(Transient)
    FPropellerSetThrottle motor_throttle_state;

    virtual TOptional<FDynamicsPropellerSetInfo> tick_propulsion(double delta_time, FSubstepBody* substep_body,
        const FDroneSetpoint& drone_setpoint, const FPropulsionDroneSetup& drone_setup,
        const USimulationWorld* simulation_world) override;
};
