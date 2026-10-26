$ErrorActionPreference = 'Stop'

$ProjectFile = 'E:\UnrealProjects\DroneGame\DroneSimTest\DroneSimTest.uproject'
$BuildBat = 'D:\games\UE_5.8\Engine\Build\BatchFiles\Build.bat'
$LogDirectory = 'E:\UnrealProjects\DroneGame\DroneSimTest\Saved\BuildLogs'

if (-not (Test-Path $ProjectFile)) { throw "Не найден проект: $ProjectFile" }
if (-not (Test-Path $BuildBat)) { throw "Не найден Unreal Build.bat: $BuildBat" }

New-Item -ItemType Directory -Force -Path $LogDirectory | Out-Null
$LogFile = Join-Path $LogDirectory ("DroneSimTestEditor_{0}.log" -f (Get-Date -Format 'yyyyMMdd_HHmmss'))

Write-Host "Сборка DroneSimTestEditor. Лог: $LogFile"
& $BuildBat DroneSimTestEditor Win64 Development "-Project=$ProjectFile" -WaitMutex -FromMsBuild -architecture=x64 2>&1 |
    Tee-Object -FilePath $LogFile

$BuildExitCode = $LASTEXITCODE
if ($BuildExitCode -eq 0) {
    Write-Host 'РЕЗУЛЬТАТ: СБОРКА УСПЕШНА' -ForegroundColor Green
} else {
    Write-Host "РЕЗУЛЬТАТ: ОШИБКА СБОРКИ, код $BuildExitCode. Полный лог: $LogFile" -ForegroundColor Red
}
