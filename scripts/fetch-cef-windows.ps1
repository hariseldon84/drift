#Requires -Version 5.1
[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$CEF_VERSION  = "152.0.6+g708dc14+chromium-152.0.7977.83"
$CEF_PLATFORM = "windows64"
$CEF_SHA1     = "e5e3020627f4528bd43e22f4c4970000b0458e99"

$ScriptDir      = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot       = Split-Path -Parent $ScriptDir
$DepsDir        = Join-Path $RepoRoot ".deps"
$Distribution   = "cef_binary_${CEF_VERSION}_${CEF_PLATFORM}_minimal"
$Archive        = "${Distribution}.tar.bz2"
$ArchivePath    = Join-Path $DepsDir $Archive
$CefRoot        = Join-Path $DepsDir $Distribution
$EncodedVersion = $CEF_VERSION -replace '\+', '%2B'
$DownloadUrl    = "https://cef-builds.spotifycdn.com/cef_binary_${EncodedVersion}_${CEF_PLATFORM}_minimal.tar.bz2"

if (Test-Path (Join-Path $CefRoot "cmake\FindCEF.cmake")) {
    Write-Host "CEF is ready at $CefRoot"
    exit 0
}

if (-not (Test-Path $DepsDir)) {
    New-Item -ItemType Directory -Force -Path $DepsDir | Out-Null
}

if (-not (Test-Path $ArchivePath)) {
    Write-Host "Downloading pinned CEF $CEF_VERSION for $CEF_PLATFORM..."
    $PartialPath = "${ArchivePath}.partial"
    Invoke-WebRequest -Uri $DownloadUrl -OutFile $PartialPath -UseBasicParsing
    Move-Item -Path $PartialPath -Destination $ArchivePath
}

$ActualSha1 = (Get-FileHash -Path $ArchivePath -Algorithm SHA1).Hash.ToLower()
if ($ActualSha1 -ne $CEF_SHA1) {
    Write-Error "CEF archive checksum mismatch.`nExpected: $CEF_SHA1`nActual:   $ActualSha1`nDelete $ArchivePath and run this script again."
    exit 1
}

Write-Host "Extracting $Archive..."
& tar -xjf $ArchivePath -C $DepsDir

if (-not (Test-Path (Join-Path $CefRoot "cmake\FindCEF.cmake"))) {
    Write-Error "CEF extraction failed - expected directory not found: $CefRoot"
    exit 1
}

Write-Host "CEF is ready at $CefRoot"
