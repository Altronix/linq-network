param (
	[Alias('p')] [string] $Platform="win32-x64",
	[Alias('f')] [switch] $Fetch=$false,
	[Alias('L')] [string] $LogLevel="INFO",
	[Alias('st')] [switch] $BuildStatic=$true,
	[Alias('sh')] [switch] $BuildShared=$false,
	[Alias('t')] [switch] $BuildTypescript=$false,
	[Alias('n')] [switch] $BuildNode=$false,
	[Alias('c')] [switch] $BuildCmake=$false,
	[Alias('d')] [switch] $BuildDependencies=$false,
	[Alias('a')] [switch] $AutoDetect=$false
)

$CurrentLocation = Get-Location
$DirScripts = $PSScriptRoot
$DirRoot = Resolve-Path $DirScripts\..
$DirSrc = "$DirRoot\src"
$DirNode = "$DirRoot\node_modules"
$DirNodeDist = "$DirRoot\dist"
$DirBuild = "$CurrentLocation\build"
$DirPrefix = "$DirBuild\install"
$Downloads = "https://github.com/altronix/linq-network/releases/download";
$Version = node $DirScripts\read_version.js

function PrintConfig 
{
	echo "DIR scripts           : $DirScripts";
	echo "DIR src               : $DirSrc";
	echo "DIR node              : $DirNode";
	echo "DIR node dist         : $DirNodeDist";
	echo "DIR build             : $DirBuild";
	echo "DIR prefix            : $DirPrefix";
	echo "CFG Platform          : $Platform";
	echo "CFG Fetch             : $Fetch";
	echo "CFG BuildCmake        : $BuildCmake";
	echo "CFG BuildNode         : $BuildNode";
	echo "CFG BuildTypescript   : $BuildTypescript";
	echo "CFG BuildShared       : $BuildShared";
	echo "CFG BuildStatic       : $BuildStatic";
	echo "CFG BuildDependencies : $BuildDependencies";
	echo "CFG LogLevel          : $LogLevel";
	echo "CFG AutoDetect        : $AutoDetect";

}

function fetch
{
	New-Item -ItemType Directory -Force -Path "$DirNodeDist\src";
	cd $DirNodeDist\src
	wget "${downloads}/v${version}/node-$arch.tar.gz"
	tar -xzvf "./node-$arch.tar.gz"
}

function AutoDetect
{
	echo "Running auto detection..."
	if(
	   (test-path  "$DirRoot\bindings") -and
	   (test-path  "$DirRoot\cmake") -and
	   (test-path  "$DirRoot\dl") -and
	   (test-path  "$DirRoot\src") -and
	   (test-path  "$DirRoot\CMakeLists.txt") -and
	   (test-path  "$DirRoot\binding.gyp") -and
	   (test-path  "$DirRoot\tsconfig.base.json")
	  )
	{
		echo "Detected source code...";
		$BuildCmake = $true;
		$BuildNode = $true;
		$BuildTypescript = $true;
		$BuildStatic = $true;
		$BuildDependencies = $true;
	}
	else
	{
		echo "No source detected...";
		$Fetch = $true
		$BuildCmake = $false;
		$BuildNode = $false;
		$BuildTypescript = $false;
		$BuildDependencies = $false;
	}
}

if ($AutoDetect) { . AutoDetect }

echo "Printing configuration..."
. PrintConfig
