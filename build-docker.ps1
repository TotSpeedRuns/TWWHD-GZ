param(
    [Parameter(Mandatory = $true)][string]$OriginalRpx,
    [Parameter(Mandatory = $true)][string]$PracticeRpx,
    [string]$OutputDirectory = (Join-Path $PSScriptRoot ('build/docker-' + (Get-Date).ToUniversalTime().ToString('yyyyMMddTHHmmssZ')))
)
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$original = (Resolve-Path -LiteralPath $OriginalRpx).Path
$practice = (Resolve-Path -LiteralPath $PracticeRpx).Path
if (!(Test-Path -LiteralPath $original -PathType Leaf) -or !(Test-Path -LiteralPath $practice -PathType Leaf)) { throw 'Both inputs must be RPX files.' }
$null = New-Item -ItemType Directory -Path $OutputDirectory -Force
$outputPath = (Resolve-Path -LiteralPath $OutputDirectory).Path
& docker build --platform linux/amd64 -t twwhd-gz-builder $repoRoot
if ($LASTEXITCODE -ne 0) { throw 'Docker image build failed.' }
& docker run --rm --platform linux/amd64 --mount "type=bind,source=$repoRoot,target=/project,readonly" --mount "type=bind,source=$original,target=/inputs/original.rpx,readonly" --mount "type=bind,source=$practice,target=/inputs/practice.rpx,readonly" --mount "type=bind,source=$outputPath,target=/output" twwhd-gz-builder
if ($LASTEXITCODE -ne 0) { throw 'Container build failed.' }
