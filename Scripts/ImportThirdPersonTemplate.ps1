param([string]$EngineRoot='D:/games/UE_5.8')
$ErrorActionPreference='Stop'
$Root=Split-Path $PSScriptRoot -Parent
$Pairs=@(
    @{Source="$EngineRoot/Templates/TemplateResources/High/Characters/Content";Dest="$Root/Content/Characters"},
    @{Source="$EngineRoot/Templates/TemplateResources/High/Input/Content";Dest="$Root/Content/Input"}
)
$CopyList=@()
foreach($Pair in $Pairs){
    if(-not(Test-Path -LiteralPath $Pair.Source -PathType Container)){throw "Template absent: $($Pair.Source)"}
    foreach($File in Get-ChildItem -LiteralPath $Pair.Source -File -Recurse){
        $Relative=$File.FullName.Substring(([IO.Path]::GetFullPath($Pair.Source)).Length+1)
        $CopyList+=@(@{Source=$File.FullName;Dest=(Join-Path $Pair.Dest $Relative)})
    }
}
$CopyList+=@(@{Source="$EngineRoot/Templates/TP_ThirdPersonBP/Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset";Dest="$Root/Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset"})
# Preflight every destination before writing anything. Existing custom assets are never overwritten.
foreach($Pair in $CopyList){
    if(-not(Test-Path -LiteralPath $Pair.Source -PathType Leaf)){throw "Source absent: $($Pair.Source)"}
    if((Test-Path -LiteralPath $Pair.Dest) -and (Get-FileHash -LiteralPath $Pair.Source).Hash -ne (Get-FileHash -LiteralPath $Pair.Dest).Hash){throw "Refusing to overwrite: $($Pair.Dest)"}
}
$Added=0
foreach($Pair in $CopyList){
    if(-not(Test-Path -LiteralPath $Pair.Dest)){
        New-Item -ItemType Directory -Force -Path (Split-Path $Pair.Dest -Parent) | Out-Null
        Copy-Item -LiteralPath $Pair.Source -Destination $Pair.Dest
        $Added++
    }
}
Write-Output "Epic Third Person template: added=$Added, files=$($CopyList.Count); no map/GameMode imported."
