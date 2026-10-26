param(
    [string]$EngineRoot = 'D:\games\UE_5.8',
    [ValidateSet('Development', 'Shipping')][string]$Configuration = 'Development'
)
$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path $PSScriptRoot -Parent
$ProjectFile = Join-Path $ProjectRoot 'DroneSimTest.uproject'
$RunUAT = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
if (-not (Test-Path -LiteralPath $RunUAT)) { throw "Не найден RunUAT: $RunUAT" }
$LogDirectory = Join-Path $ProjectRoot 'Saved\BuildLogs'
New-Item -ItemType Directory -Force -Path $LogDirectory | Out-Null
$LogFile = Join-Path $LogDirectory ("Package{0}_{1}.log" -f $Configuration, (Get-Date -Format 'yyyyMMdd_HHmmss'))
$Archive = Join-Path $ProjectRoot "Saved\Packaged\Quality$Configuration"
Write-Host "Упаковка $Configuration. Лог: $LogFile"
& $RunUAT BuildCookRun "-project=$ProjectFile" -noP4 -platform=Win64 "-clientconfig=$Configuration" -build -cook '-map=/Game/L_DroneTest' -stage -pak -archive "-archivedirectory=$Archive" -unattended -utf8output 2>&1 | Tee-Object -FilePath $LogFile
$PackageExitCode = $LASTEXITCODE
if ($PackageExitCode -eq 0) { Write-Host "Успешно. Билд: $Archive" }
else { Write-Host "Ошибка упаковки: $PackageExitCode. Лог: $LogFile" }
exit $PackageExitCode
