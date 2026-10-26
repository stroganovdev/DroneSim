#if WITH_DEV_AUTOMATION_TESTS
#include "DroneFlightSettings.h"
#include "Misc/AutomationTest.h"
#include <limits>
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDroneSettingsValidationTest, "DroneSimTest.Settings.Validation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDroneSettingsValidationTest::RunTest(const FString& Parameters)
{
    FDroneFlightSettings Settings;
    Settings.MassGrams = 10000.f;
    Settings.Expo = -4.f;
    Settings.RollRate = 100.f;
    Settings.CenterSensitivity = 300.f;
    Settings.CameraFov = std::numeric_limits<float>::quiet_NaN();
    UDroneSettingsSubsystem::Sanitize(Settings);
    TestEqual(TEXT("Mass clamp"), Settings.MassGrams, 120.f);
    TestEqual(TEXT("Expo clamp"), Settings.Expo, 0.f);
    TestEqual(TEXT("Center cannot exceed axis maximum"), Settings.CenterSensitivity, 100.f);
    TestEqual(TEXT("Invalid FOV uses default"), Settings.CameraFov, 100.f);
    FDroneFlightSettings Default;
    UDroneSettingsSubsystem::Sanitize(Default);
    TestEqual(TEXT("Default Meteor mass is unchanged"), Default.MassGrams, 45.f);
    TestEqual(TEXT("Default Meteor rates are unchanged"), Default.RollRate, 670.f);
    return true;
}
#endif
