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
	[Alias('a')] [switch] $AutoDetect=$false,
	[Alias('v')] [string] $Version=(node $PSScriptRoot\read_version.js)
)

$CurrentLocation = Get-Location
$DirScripts = $PSScriptRoot
$DirRoot = Resolve-Path $DirScripts\..
$DirSrc = "$DirRoot\src"
$DirNode = "$DirRoot\node_modules"
$DirNodeDist = "$DirRoot\dist"
$DirNodeBinding = "$DirRoot\bindings\node\lib"
$DirBuild = "$CurrentLocation\build"
$DirPrefix = "$DirBuild\install"
$Downloads = "https://github.com/altronix/linq-network/releases/download";

function PrintConfig 
{
	echo "VER                   : $Version";
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
	$tarball = "$downloads/v$version/node-$Platform.tar.gz";
	echo $tarball
	$outfile = "$DirNodeDist\node-$Platform.tar.gz";
	New-Item -ItemType Directory -Force -Path "$DirNodeDist\src";
	$response = Invoke-WebRequest -Uri $tarball -OutFile $outfile
	tar -C "$DirNodeDist\src" -xzvf "$DirNodeDist\node-$Platform.tar.gz"
}

function InstallBinding
{
	if ((test-path "$DirBuild\Release\linq.node"))
	{
		cp "$DirBuild\Release\linq.node" "$DirNodeDist\src"
	}
	else
	{
		. fetch
	}
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

. PrintConfig

if ($BuildCmake)
{
	echo "Building CMake..."
	mkdir -Force build/install
	cmake.exe `
	"-S$DirRoot" `
	"-B$DirBuild" `
	"-DLOG_LEVEL=$LogLevel" `
	"-DCMAKE_BUILD_TYPE=Release" `
	"-DCMAKE_INSTALL_PREFIX=$DirPrefix" `
	"-DBUILD_DEPENDENCIES=$BuildDependencies" `
	"-DBUILD_SHARED=$BuildShared" `
	"-DBUILD_STATIC=$BuildStatic" `
	"-DBUILD_APPS=FALSE";
	
	cmake.exe --build "$DirBuild" --config Release --target install;
}

if ($BuildNode)
{
	echo "Building NodeJS Addon..."
	$result = & "$DirNode/.bin/node-gyp.cmd" configure
	$result = & "$DirNode/.bin/node-gyp.cmd" build
	cd "$DirRoot\bindings\node\prebuilds";
	tar -czvf node-win32-x64.tar.gz -C "$DirBuild\Release" linq.node;
	cd "$CurrentLocation";
}

if ($BuildTypescript)
{
	echo "Building Typescript..."
	$result = & "$DirNode/.bin/tsc.cmd" -p "$DirNodeBinding";
	. InstallBinding
}

if ($Fetch)
{
	echo "Fetching prebuild addon..."
	. InstallBinding
}
