param(
    [Parameter(Mandatory=$true)][ValidatePattern('^[a-z0-9]+(-[a-z0-9]+)*$')][string]$Slug,
    [Parameter(Mandatory=$true)][string]$Title,
    [ValidateSet('low','normal','foundation')][string]$Risk = 'normal'
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path $PSScriptRoot -Parent
Push-Location -LiteralPath $Root
try {
    $Revision = (& git rev-parse HEAD)
    if ($LASTEXITCODE -ne 0) { throw 'Cannot determine Git baseline' }
    $Before = @(& git status --short --untracked-files=all)
    if ($LASTEXITCODE -ne 0) { throw 'Cannot read Git status' }
    $Now = [TimeZoneInfo]::ConvertTimeBySystemTimeZoneId([DateTimeOffset]::UtcNow, 'Russian Standard Time')
    $Id = $Now.ToString('yyyyMMdd-HHmmss') + '-' + $Slug
    $Directory = Join-Path $Root 'Development/Changes'
    New-Item -ItemType Directory -Force -Path $Directory | Out-Null
    $Path = Join-Path $Directory ($Id + '.json')
    $Record = [ordered]@{
        schemaVersion=1; id=$Id; title=$Title; created=$Now.ToString('o'); risk=$Risk; status='in-progress'
        baseline=[ordered]@{ revision="$Revision"; existingChanges=$Before }
        request=''; acceptance=@(); changes=@(); decisions=@(); files=@()
        checks=@(); reviews=@(); limitations=@(); contentNotes=@()
    }
    $Json = $Record | ConvertTo-Json -Depth 12
    # CreateNew prevents overwriting a record even when invoked twice in the same second.
    $Stream = [IO.File]::Open($Path,[IO.FileMode]::CreateNew)
    try { $Bytes=[Text.UTF8Encoding]::new($false).GetBytes($Json); $Stream.Write($Bytes,0,$Bytes.Length) }
    finally { $Stream.Dispose() }
    Write-Output $Id
}
finally { Pop-Location }
