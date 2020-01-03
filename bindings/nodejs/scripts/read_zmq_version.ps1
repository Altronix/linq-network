$path = $args[0]
$MAJOR = cat $path | Select-String -Pattern "^#define +ZMQ_VERSION_MAJOR +[0-9]+$" | %{ $_.Line.Split(' ')[2]; }
$MINOR = cat $path | Select-String -Pattern "^#define +ZMQ_VERSION_MINOR +[0-9]+$" | %{ $_.Line.Split(' ')[2]; }
$PATCH = cat $path | Select-String -Pattern "^#define +ZMQ_VERSION_PATCH +[0-9]+$" | %{ $_.Line.Split(' ')[2]; }
$VERSION = "$($MAJOR)_$($MINOR)_$($PATCH)"
Write-Output "$($VERSION)"
