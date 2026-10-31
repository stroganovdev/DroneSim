param()
$ErrorActionPreference='Stop'
$Root=Split-Path $PSScriptRoot -Parent
$Suffix=[Guid]::NewGuid().ToString('N').Substring(0,10)
$Id='20990101-000000-harness-selftest-'+$Suffix
$RecordPath=Join-Path $Root ("Development/Changes/$Id.json")
$Rendered=[IO.Path]::ChangeExtension($RecordPath,'.md')
$Evidence=Join-Path $Root ("Saved/QualityHarness/SelfTests/$Suffix.txt")
$Results=@()
function Save-Fixture { $Fixture | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $RecordPath -Encoding UTF8 }
function Expect-Validation([string]$Name,[bool]$Pass) {
    Save-Fixture
    $ErrorActionPreference='Continue'
    $Output=& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'Invoke-QualityHarness.ps1') -ChangeId $Id -Mode Validate 2>&1
    $Code=$LASTEXITCODE
    $ErrorActionPreference='Stop'
    if(($Code -eq 0) -ne $Pass){throw "$Name expected pass=$Pass got exit=$Code : $Output"}
    $script:Results+=@([pscustomobject]@{name=$Name;passed=$true;exitCode=$Code})
}
try {
    New-Item -ItemType Directory -Force -Path (Split-Path $RecordPath -Parent),(Split-Path $Evidence -Parent) | Out-Null
    if(Test-Path -LiteralPath $RecordPath){throw 'Unexpected fixture collision'}
    'SYNTHETIC harness self-test evidence; not a game test.' | Set-Content -LiteralPath $Evidence -Encoding UTF8
    $Relative=$Evidence.Substring($Root.Length+1).Replace('\','/')
    $Fixture=[pscustomobject]@{
        schemaVersion=1;id=$Id;title='SYNTHETIC self-test';risk='low';status='verified';request='Check validator'
        acceptance=@('Validation gates behave as documented');changes=@('Synthetic fixture');files=@('Scripts/TestQualityHarness.ps1')
        decisions=@();limitations=@();contentNotes=@('Not actual game evidence')
        checks=@([pscustomobject]@{name='diff-review';method='review';status='passed';detail='Synthetic scope only';evidence=$Relative;sha256=(Get-FileHash $Evidence).Hash})
        sourceHashes=@([pscustomobject]@{path='Scripts/TestQualityHarness.ps1';sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'TestQualityHarness.ps1')).Hash})
        reviews=@()
    }
    Expect-Validation 'Valid low-risk record renders' $true
    if(-not(Test-Path -LiteralPath $Rendered)){throw 'No Markdown generated'}
    $OriginalAcceptance=$Fixture.acceptance
    $Fixture.acceptance=$null
    Expect-Validation 'Missing acceptance rejected' $false
    $Fixture.acceptance=$OriginalAcceptance
    $OriginalHash=$Fixture.sourceHashes[0].sha256
    $Fixture.sourceHashes[0].sha256='STALE'
    Expect-Validation 'Stale source snapshot rejected' $false
    $Fixture.sourceHashes[0].sha256=$OriginalHash
    $Fixture.checks[0].status='failed'
    Expect-Validation 'Failed check cannot be verified' $false
    $Fixture.checks[0].status='passed';$Fixture.risk='foundation'
    Expect-Validation 'Foundation without review rejected' $false
    $Fixture.reviews=@(
        [pscustomobject]@{reviewer='synthetic-self';method='contract-review';status='passed';detail='Synthetic contract';evidence=$Relative},
        [pscustomobject]@{reviewer='synthetic-independent';method='consumer-review';status='passed';detail='Synthetic extension';evidence=$Relative}
    )
    Expect-Validation 'Two reviews without behavior rejected' $false
    $Fixture.checks+=@([pscustomobject]@{name='fixture-test';method='behavior';status='passed';detail='Synthetic behavior';evidence=$Relative})
    Expect-Validation 'Foundation with distinct evidence accepted structurally' $true
    $Fixture.checks[1].status='not-run'
    Expect-Validation 'Required skipped check rejected' $false
    $Fixture.checks[1].status='passed'
    'Changed evidence' | Add-Content -LiteralPath $Evidence
    Expect-Validation 'Tampered evidence rejected' $false
    $Fixture.checks[0].sha256=(Get-FileHash $Evidence).Hash
    $Fixture.checks[1].status='not-run';$Fixture.checks[1].detail='Synthetic unavailable device';$Fixture.status='partial';$Fixture.limitations=@('Device test unavailable')
    Expect-Validation 'Partial remains explicit' $true
    $Report=Join-Path $Root 'Saved/QualityHarness/SelfTestResults.json'
    @{success=$true;tests=$Results} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $Report -Encoding UTF8
    Write-Output "PASS: $($Results.Count) harness cases; $Report"
} finally {
    # Remove only files this invocation owns; never user records.
    foreach($Owned in @($RecordPath,$Rendered,$Evidence)){
        if(Test-Path -LiteralPath $Owned){Remove-Item -LiteralPath $Owned}
    }
}
