#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "DroneSimulatorInput/Public/DroneControlSource.h"
#include "DroneSimulatorInput/Public/DroneInputSubsystem.h"
#include <limits>
namespace
{
    class FTestSource final : public IDroneControlSource
    {
    public:
        explicit FTestSource(FName InId) : Id(InId) {}
        FName Id;
        bool Connected = true;
        float Throttle = 0.f, Yaw = 0.f;
        FName GetId() const override { return Id; }
        FText GetLabel() const override { return FText::FromName(Id); }
        bool IsAvailable() const override { return Connected; }
        float ReadAxis(EDroneInputAxis Axis, EThrottleCalibrationSpace Space) const override
        {
            if (Axis == EDroneInputAxis::Throttle) return Space == EThrottleCalibrationSpace::PositiveOnly ? Throttle : Throttle*2.f-1.f;
            return Axis == EDroneInputAxis::Yaw ? Yaw : 0.f;
        }
    };
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDroneSourceIsolationTest,"DroneSimTest.Input.SourceIsolation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDroneSourceIsolationTest::RunTest(const FString& Parameters)
{
    auto* Input = NewObject<UDroneInputSubsystem>(NewObject<UGameInstance>());
    auto Radio = MakeShared<FTestSource>(TEXT("TestRadio"));
    auto Pad = MakeShared<FTestSource>(TEXT("TestPad"));
    TestTrue(TEXT("Register extension"), Input->RegisterControlSource(Radio));
    TestTrue(TEXT("Register second extension"), Input->RegisterControlSource(Pad));
    TestFalse(TEXT("Duplicate ID rejected"), Input->RegisterControlSource(Pad));
    TestFalse(TEXT("Unknown selection rejected"), Input->SetControlSource(TEXT("Missing")));
    Radio->Throttle=.8f; Input->SetControlSource(Radio->Id); Input->Tick(.016f);
    TestEqual(TEXT("Held throttle is blocked on switch"),Input->get_calibrated_axis_value(EDroneInputAxis::Throttle),0.f);
    TestEqual(TEXT("Signed idle throttle is minimum"),Input->get_calibrated_axis_value(EDroneInputAxis::Throttle,EThrottleCalibrationSpace::PositiveNegative),-1.f);
    Radio->Throttle=0; Input->Tick(.016f); Radio->Yaw=.8f;
    TestEqual(TEXT("Selected radio works"),Input->get_calibrated_axis_value(EDroneInputAxis::Yaw),.8f);
    Pad->Yaw=-.6f; Input->SetControlSource(Pad->Id); Input->Tick(.016f);
    TestEqual(TEXT("Non-neutral pad blocked"),Input->get_calibrated_axis_value(EDroneInputAxis::Yaw),0.f);
    Pad->Yaw=0; Input->Tick(.016f);
    TestEqual(TEXT("Inactive radio does not leak"),Input->get_calibrated_axis_value(EDroneInputAxis::Yaw),0.f);
    Pad->Yaw=-.6f;
    TestEqual(TEXT("Only pad produces yaw"),Input->get_calibrated_axis_value(EDroneInputAxis::Yaw),-.6f);
    Pad->Connected=false; Input->Tick(.016f);
    TestEqual(TEXT("Disconnect returns zero, no fallback"),Input->get_calibrated_axis_value(EDroneInputAxis::Yaw),0.f);
    Pad->Connected=true; Pad->Throttle=.9f; Input->Tick(.016f);
    TestTrue(TEXT("Reconnect requires neutral"),Input->IsWaitingForNeutral());
    Pad->Throttle=0; Pad->Yaw=0; Input->Tick(.016f);
    TestFalse(TEXT("Neutral reconnect resumes"),Input->IsWaitingForNeutral());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDroneGamepadNormalizationTest,"DroneSimTest.Input.GamepadNormalization",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDroneGamepadNormalizationTest::RunTest(const FString& Parameters)
{
    FDroneGamepadSettings Settings;
    TestEqual(TEXT("Default throttle uses left vertical stick"),Settings.Throttle,EKeys::Gamepad_LeftY);
    TestEqual(TEXT("Stick down is zero"),Settings.Normalize(-1,true),0.f);
    TestEqual(TEXT("Stick centre is half throttle"),Settings.Normalize(0,true),.5f);
    TestEqual(TEXT("Stick up is full throttle"),Settings.Normalize(1,true),1.f);
    TestEqual(TEXT("Lower half of travel is usable"),Settings.Normalize(-.5f,true),.25f);
    Settings.Throttle=EKeys::Gamepad_RightTriggerAxis;
    TestEqual(TEXT("Released trigger is zero"),Settings.Normalize(0,true),0.f);
    TestEqual(TEXT("Full trigger is full throttle"),Settings.Normalize(1,true),1.f);
    TestEqual(TEXT("Negative stick throttle never produces thrust"),Settings.Normalize(-1,true),0.f);
    TestEqual(TEXT("Small drift removed"),Settings.Normalize(.08f,false),0.f);
    TestEqual(TEXT("Full negative rotation preserved"),Settings.Normalize(-1,false),-1.f);
    TestEqual(TEXT("NaN is zero"),Settings.Normalize(std::numeric_limits<float>::quiet_NaN(),false),0.f);
    Settings.Throttle=EKeys::SpaceBar; Settings.Deadzone=std::numeric_limits<float>::quiet_NaN(); Settings.Sanitize();
    TestEqual(TEXT("Invalid binding uses left Y"),Settings.Throttle,EKeys::Gamepad_LeftY);
    TestEqual(TEXT("Invalid deadzone restored"),Settings.Deadzone,.12f);
    Settings.Deadzone=99; Settings.Sanitize();
    TestEqual(TEXT("Deadzone bounded"),Settings.Deadzone,.4f);
    return true;
}
#endif
