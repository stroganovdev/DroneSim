param(
    [Parameter(Mandatory=$true)][ValidatePattern('^[0-9]{8}-[0-9]{6}-[a-z0-9]+(-[a-z0-9]+)*$')][string]$ChangeId,
    [ValidateSet('Snapshot','Editor','EditorAndTests','Validate')][string]$Mode='Validate',
    [string]$EngineRoot='D:/games/UE_5.8'
)
$ErrorActionPreference='Stop'
$Root=Split-Path $PSScriptRoot -Parent
$RecordPath=Join-Path $Root ("Development/Changes/" + $ChangeId + ".json")
function Save-Record {
    $Record | ConvertTo-Json -Depth 18 | Set-Content -LiteralPath $RecordPath -Encoding UTF8
}
function Add-Check([string]$Name,[string]$Method,[string]$Status,[string]$Evidence,[string]$Detail) {
    $Relative=$Evidence.Substring($Root.Length+1).Replace('\','/')
    $Hash=if(Test-Path -LiteralPath $Evidence -PathType Leaf){(Get-FileHash -LiteralPath $Evidence -Algorithm SHA256).Hash}else{''}
    $Record.checks=@($Record.checks | Where-Object { $_.name -ne $Name }) + @([pscustomobject]@{
        name=$Name; method=$Method; status=$Status; evidence=$Relative; sha256=$Hash
        detail=$Detail; revision=((& git rev-parse HEAD) -join ''); scope=@(& git status --short --untracked-files=all)
    })
    if($Status -eq 'failed'){$Record.status='partial'}
    Save-Record
}
function Assert-Evidence($Item) {
    if([string]::IsNullOrWhiteSpace($Item.detail) -or [string]::IsNullOrWhiteSpace($Item.method)){throw 'Missing check/review detail or method'}
    if($Item.status -eq 'passed'){
        if([string]::IsNullOrWhiteSpace($Item.evidence)){throw 'Passed check/review needs evidence'}
        $EvidencePath=[IO.Path]::GetFullPath((Join-Path $Root $Item.evidence))
        if(-not $EvidencePath.StartsWith($Root+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Evidence must stay inside project'}
        if(-not(Test-Path -LiteralPath $EvidencePath -PathType Leaf)){throw "Evidence missing: $EvidencePath"}
        if($Item.sha256 -and (Get-FileHash -LiteralPath $EvidencePath -Algorithm SHA256).Hash -ne $Item.sha256){throw "Evidence changed: $EvidencePath"}
    }
}
function Get-ScopeHashes {
    $Hashes=@()
    foreach($File in $Record.files){
        if([string]::IsNullOrWhiteSpace($File)){throw 'Blank source path'}
        $Path=[IO.Path]::GetFullPath((Join-Path $Root $File))
        if(-not $Path.StartsWith($Root+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Source path leaves project'}
        if(-not(Test-Path -LiteralPath $Path -PathType Leaf)){
            & git ls-files --error-unmatch -- $File *> $null
            if($LASTEXITCODE -ne 0){
                if(-not $Record.baseline.revision){throw "Missing untracked scope path: $File"}
                $Normalized=$File.Replace('\','/')
                & git cat-file -e "$($Record.baseline.revision):$Normalized" 2> $null
                if($LASTEXITCODE -ne 0){throw "Path absent from index and baseline: $File"}
            }
            $Hashes+=@([pscustomobject]@{path=$File.Replace('\','/');sha256='<deleted>'})
            continue
        }
        if($File.Replace('\','/') -like 'Development/Changes/*'){continue}
        $Hashes+=@([pscustomobject]@{path=$File.Replace('\','/');sha256=(Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash})
    }
    return $Hashes
}
function Assert-Scope {
    $Current=@(Get-ScopeHashes)
    if(-not $Record.sourceHashes -or @($Record.sourceHashes).Count -ne $Current.Count){throw 'Source snapshot missing; set files, then run -Mode Snapshot before checks'}
    foreach($Hash in $Current){
        $Match=@($Record.sourceHashes | Where-Object {$_.path -eq $Hash.path -and $_.sha256 -eq $Hash.sha256})
        if($Match.Count -ne 1){throw "Source changed since verification snapshot: $($Hash.path). Recheck relevant evidence."}
    }
}
Push-Location -LiteralPath $Root
try {
    $Record=Get-Content -LiteralPath $RecordPath -Raw | ConvertFrom-Json
    if($Record.schemaVersion -ne 1 -or $Record.id -ne $ChangeId){throw 'Unsupported schema or record ID mismatch'}
    if($Mode -eq 'Snapshot'){
        if(@($Record.checks | Where-Object {$_}).Count -or @($Record.reviews | Where-Object {$_}).Count){throw 'Snapshot must precede checks/reviews; create a new iteration after changes'}
        $Record | Add-Member -NotePropertyName sourceHashes -NotePropertyValue @(Get-ScopeHashes) -Force
        if(@($Record.sourceHashes).Count -eq 0){throw 'Set non-journal files before Snapshot'}
        Save-Record
        Write-Output 'Source scope captured'
        exit 0
    }
    Assert-Scope
    if($Mode -ne 'Validate'){
        $Run=Get-Date -Format 'yyyyMMdd-HHmmss-fff'
        $RunDir=Join-Path $Root ("Saved/QualityHarness/$ChangeId/$Run")
        New-Item -ItemType Directory -Force -Path $RunDir | Out-Null
        $BuildLog=Join-Path $RunDir 'editor-build.log'
        $ErrorActionPreference='Continue'
        $LASTEXITCODE=$null
        & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'BuildDroneSimTestEditor.ps1') *> $BuildLog
        $Code=$LASTEXITCODE
        $ErrorActionPreference='Stop'
        if($null -eq $Code){$Code=1}
        Add-Check 'editor-build' 'compiler' $(if($Code -eq 0){'passed'}else{'failed'}) $BuildLog "Exit=$Code; build script: Scripts/BuildDroneSimTestEditor.ps1"
        if($Code -ne 0){throw "Editor build failed ($Code): $BuildLog"}
        if($Mode -eq 'EditorAndTests'){
            $Editor=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
            $Report=Join-Path $RunDir 'Automation'
            $TestLog=Join-Path $RunDir 'automation.log'
            try {
                if(-not(Test-Path -LiteralPath $Editor -PathType Leaf)){throw "Editor commandlet missing: $Editor"}
                $ErrorActionPreference='Continue'
                $LASTEXITCODE=$null
                & $Editor (Join-Path $Root 'DroneSimTest.uproject') '-ExecCmds=Automation RunTests DroneSimTest' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$Report" "-UserDir=$RunDir/User" "-abslog=$TestLog" -unattended -nop4 -nosplash -NullRHI -nosound *> (Join-Path $RunDir 'automation-stdout.log')
                $TestExit=$LASTEXITCODE
                $ErrorActionPreference='Stop'
                if($null -eq $TestExit -or $TestExit -ne 0){throw "Automation exit=$TestExit"}
                $Results=Get-Content -LiteralPath (Join-Path $Report 'index.json') -Raw | ConvertFrom-Json
                foreach($Field in @('succeeded','failed','notRun','inProcess')){
                    if($null -eq $Results.$Field -or ($Results.$Field -isnot [ValueType] -or $Results.$Field -is [bool]) -or $Results.$Field -lt 0 -or [double]$Results.$Field -ne [math]::Floor([double]$Results.$Field)){throw "Malformed automation counter: $Field"}
                }
                if($Results.succeeded -lt 1 -or $Results.failed -gt 0 -or $Results.notRun -gt 0 -or $Results.inProcess -gt 0){throw 'Automation report is empty, failed or incomplete'}
                Add-Check 'native-tests' 'automation' 'passed' (Join-Path $Report 'index.json') "DroneSimTest suite: passed=$($Results.succeeded), failed=$($Results.failed), notRun=$($Results.notRun)"
            } catch {
                $_ | Out-String | Add-Content -LiteralPath $TestLog
                Add-Check 'native-tests' 'automation' 'failed' $TestLog $_.Exception.Message
                throw
            }
        }
        Write-Output "Checks recorded: $RecordPath"
        exit 0
    }
    foreach($Key in @('title','request')) { if([string]::IsNullOrWhiteSpace($Record.$Key)){throw "Missing $Key"} }
    foreach($Key in @('acceptance','changes','files','contentNotes')){if(-not $Record.$Key -or @($Record.$Key | Where-Object {[string]::IsNullOrWhiteSpace($_)}).Count -gt 0){throw "Missing $Key"}}
    if($Record.risk -notin @('low','normal','foundation')){throw 'Unknown risk'}
    if($Record.status -notin @('verified','partial','blocked')){throw 'Close as verified, partial or blocked before validation'}
    if(@($Record.checks | Where-Object {$_.name -eq 'diff-review'}).Count -ne 1){throw 'Exactly one diff-review check required'}
    foreach($Check in $Record.checks){
        if($Check.status -notin @('passed','failed','not-run')){throw 'Unknown check status'}
        Assert-Evidence $Check
    }
    foreach($Review in $Record.reviews){
        if($Review.status -notin @('passed','failed') -or [string]::IsNullOrWhiteSpace($Review.reviewer)){throw 'Invalid reviewer/status'}
        Assert-Evidence $Review
    }
    if($Record.status -eq 'verified'){
        if(@($Record.checks | Where-Object {$_.status -ne 'passed' -and $_.required -ne $false}).Count){throw 'Required checks must pass'}
        if(@($Record.checks | Where-Object {$_.status -eq 'failed'}).Count -or @($Record.reviews | Where-Object {$_.status -eq 'failed'}).Count){throw 'Failed evidence cannot be verified'}
        if(@($Record.checks | Where-Object {$_.name -eq 'diff-review' -and $_.status -eq 'passed'}).Count -ne 1){throw 'Diff review must pass'}
        $RuntimeFiles=@($Record.files | Where-Object {$_.Replace('\','/') -match '^(Source|Plugins|Content|Config)/'})
        if($RuntimeFiles.Count -gt 0 -and @($Record.checks | Where-Object {$_.name -eq 'editor-build' -and $_.status -eq 'passed'}).Count -ne 1){throw 'Runtime changes require successful Editor build'}
        if($RuntimeFiles.Count -gt 0 -and @($Record.checks | Where-Object {$_.status -eq 'passed' -and $_.method -ne 'compiler' -and $_.name -ne 'diff-review'}).Count -lt 1){throw 'Runtime changes require behavioral verification'}
        if($Record.risk -eq 'foundation'){
            $PassedReviews=@($Record.reviews | Where-Object {$_.status -eq 'passed'})
            if($PassedReviews.Count -lt 2){throw 'Foundation needs two passing review passes'}
            if(@($PassedReviews.method | Select-Object -Unique).Count -lt 2){throw 'Foundation reviews need distinct methods'}
            if(@($Record.checks | Where-Object {$_.status -eq 'passed' -and $_.method -ne 'compiler' -and $_.name -ne 'diff-review'}).Count -lt 1){throw 'Foundation needs behavioral evidence beyond compilation/review'}
        }
    } elseif(@($Record.limitations).Count -eq 0){throw 'Partial/blocked needs limitations'}
    $Lines=@("# $($Record.title)","", "ID: $ChangeId | Risk: $($Record.risk) | Status: $($Record.status)", "", $Record.request)
    foreach($Key in @('acceptance','changes','decisions','files','limitations','contentNotes')){
        $Lines+=@('',"## $Key",'')
        $Lines+=@($Record.$Key | ForEach-Object {"- $_"})
    }
    $Lines+=@('','## Checks and reviews','')
    foreach($Item in @($Record.checks)+@($Record.reviews)){$Lines+="- [$($Item.status)] $($Item.method) $($Item.reviewer): $($Item.detail) (evidence: $($Item.evidence))"}
    $Lines+=@('',"Editable source: $ChangeId.json. Raw Saved evidence is local and not versioned.")
    $Lines | Set-Content -LiteralPath ([IO.Path]::ChangeExtension($RecordPath,'.md')) -Encoding UTF8
    Write-Output "VALID: $RecordPath"
    exit 0
} catch {
    Write-Error $_ -ErrorAction Continue
    exit 1
} finally {Pop-Location}
