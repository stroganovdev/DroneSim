param([string]$Target = 'DroneSimTestEditor')

$ErrorActionPreference = 'Stop'

$ProjectFile = (Join-Path (Split-Path $PSScriptRoot -Parent) 'DroneSimTest.uproject')
$BuildBat = 'D:\games\UE_5.8\Engine\Build\BatchFiles\Build.bat'
$LogDirectory = (Join-Path (Split-Path $PSScriptRoot -Parent) 'Saved\BuildLogs')

if (-not (Test-Path $ProjectFile)) { throw "Не найден проект: $ProjectFile" }
if (-not (Test-Path $BuildBat)) { throw "Не найден Unreal Build.bat: $BuildBat" }

New-Item -ItemType Directory -Force -Path $LogDirectory | Out-Null
$LogFile = Join-Path $LogDirectory ("DroneSimTestEditor_{0}.log" -f (Get-Date -Format 'yyyyMMdd_HHmmss'))

Write-Host "Сборка DroneSimTestEditor. Лог: $LogFile"
& $BuildBat $Target Win64 Development "-Project=$ProjectFile" -WaitMutex -FromMsBuild -architecture=x64 2>&1 |
    Tee-Object -FilePath $LogFile

$BuildExitCode = $LASTEXITCODE
if ($BuildExitCode -eq 0) {
    Write-Host 'РЕЗУЛЬТАТ: СБОРКА УСПЕШНА' -ForegroundColor Green
} else {
    Write-Host "РЕЗУЛЬТАТ: ОШИБКА СБОРКИ, код $BuildExitCode. Полный лог: $LogFile" -ForegroundColor Red
}

exit $BuildExitCode

