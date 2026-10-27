#pragma once
#include "CoreMinimal.h"
#include "DroneInputTypes.h"

// Register a provider with the input subsystem to add another selectable control source.
// Values are signed [-1,1], except throttle, whose space is requested by the flight mode.
class DRONESIMULATORINPUT_API IDroneControlSource
{
public:
    virtual ~IDroneControlSource() = default;
    virtual FName GetId() const = 0;
    virtual FText GetLabel() const = 0;
    virtual bool IsAvailable() const = 0;
    virtual float ReadAxis(EDroneInputAxis Axis, EThrottleCalibrationSpace Space) const = 0;
    virtual void Tick() {}
    virtual void Reset() {}
};
