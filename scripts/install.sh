#!/bin/bash - 
#===============================================================================
#
#          FILE: install.sh
# 
#         USAGE: ./install.sh 
# 
#   DESCRIPTION: Build linq-network in various ways depending if Windows, Linux
#		 or Buildroot.
# 
#       OPTIONS: ---
#       CREATED: 10/12/2021 18:54
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

#-------------------------------------------------------------------------------
# Defaults 
#-------------------------------------------------------------------------------
downloads="https://github.com/altronix/linq-network/releases/download";
dir_scripts=$(eval realpath "$(eval dirname "$0")");
dir_src=$(eval realpath "$dir_scripts/..");
dir_node=$(eval realpath "$dir_src/node_modules");
dir_node_dist=$(eval realpath "$dir_src/dist");
dir_node_binding=$(eval realpath "$dir_src/bindings/node/lib");
dir_node_prebuild=$(eval realpath "$dir_src/bindings/node/prebuilds");
dir_build="$PWD/build";
dir_prefix="$dir_build/install";
version=${LINQ_NETWORK_VERSION:-$(eval node "${dir_scripts}/read_version.js")};
config_buildroot=${LINQ_NETWORK_BUILDROOT:-false};
if [[ ${config_buildroot,,} = true ]]; then
	config_arch=${LINQ_NETWORK_ARCH:-linux-arm32};
else
	config_arch=${LINQ_NETWORK_ARCH:-linux-x64};
fi
config_fetch=${LINQ_NETWORK_FETCH:-false};
config_build_cmake=${LINQ_NETWORK_BUILD_CMAKE:-false};
config_build_node=${LINQ_NETWORK_BUILD_NODE:-false};
config_build_ts=${LINQ_NETWORK_BUILD_TS:-false};
config_build_shared=${LINQ_NETWORK_BUILD_SHARED:-false};
config_build_static=${LINQ_NETWORK_BUILD_STATIC:-true};
config_build_dependencies=${LINQ_NETWORK_BUILD_DEPENDENCIES:-false};
config_log_level=${LINQ_NETWORK_LOG_LEVEL:-info};

#---  FUNCTION  ----------------------------------------------------------------
#          NAME:  print_config
#   DESCRIPTION:  
#-------------------------------------------------------------------------------
function print_config {
	printf "VER                    : %s\n" "$version";
	printf "DIR scripts            : %s\n" "$dir_scripts";
	printf "DIR src                : %s\n" "$dir_src";
	printf "DIR node               : %s\n" "$dir_node";
	printf "DIR node dist          : %s\n" "$dir_node_dist";
	printf "DIR prefix             : %s\n" "$dir_prefix";
	printf "CFG arch               : %s\n" "$config_arch";
	printf "CFG config_fetch       : %s\n" "$config_fetch";
	printf "CFG build cmake        : %s\n" "$config_build_cmake";
	printf "CFG build node         : %s\n" "$config_build_node";
	printf "CFG build typescript   : %s\n" "$config_build_ts";
	printf "CFG build shared       : %s\n" "$config_build_shared";
	printf "CFG build static       : %s\n" "$config_build_static";
	printf "CFG build dependencies : %s\n" "$config_build_dependencies";
	printf "CFG log level          : %s\n" "$config_log_level";
}


#---  FUNCTION  ----------------------------------------------------------------
#          NAME:  install_binding
#   DESCRIPTION:  
#-------------------------------------------------------------------------------
function fetch {
	wget "${downloads}/v${version}/node-$config_arch.tar.gz" -O - \
		 > "$dir_node_dist/node-${config_arch}.tar.gz" || exit;
	tar -C "$dir_node_dist/src" \
		-xzvf "${dir_node_dist}/node-${config_arch}.tar.gz";
}

#---  FUNCTION  ----------------------------------------------------------------
#          NAME:  install_binding
#   DESCRIPTION:  
#-------------------------------------------------------------------------------
function install_binding {
	if [[ -f "$dir_build/Release/linq.node" ]]; then
			cp "$dir_build/Release/linq.node" "$dir_node_dist/src";
		else
			fetch
	fi
}

#---  FUNCTION  ----------------------------------------------------------------
#          NAME:  install_binding
#   DESCRIPTION:  Detect if we are capable of installing everything from source
#                 If we can build from source we also detect if we need shared 
#                 libraries or not. On arm (aka buildroot) we use shared libs
#-------------------------------------------------------------------------------
function auto_detect {
	printf "attempting to automatically detect best way to build\n"

	if [[ 
		-d "$dir_src/bindings" &&
		-d "$dir_src/cmake" &&
		-d "$dir_src/dl" &&
		-d "$dir_src/src" &&
		-f "$dir_src/CMakeLists.txt" &&
		-f "$dir_src/binding.gyp" &&
		-f "$dir_src/tsconfig.base.json"
	   ]]; then
	   	# We  assume a full repository and build everything
		printf "detected source code\n"
		config_fetch=false;
		config_build_cmake=true;
		config_build_node=true;
		config_build_ts=true;
		if [[ ${config_arch} = linux-arm32 ]]; then
			printf "detected buildroot\n"
			config_build_cmake=false;
			config_build_shared=false;
			config_build_static=false;
			config_build_dependencies=false;
		else
			config_build_shared=false;
			config_build_static=true;
			config_build_dependencies=true;
		fi
	else
		# We were likely installed from an npm install command so we 
		# fetch prebuilt binaries
		printf "no source detected, using prebuilt binaries\n"
		config_fetch=true;
		config_build_cmake=false;
		config_build_node=false;
		config_build_ts=false;
	fi
}

#-------------------------------------------------------------------------------
# parse args
#-------------------------------------------------------------------------------
while getopts "v:Dcntfl:L:Aa:" opt; do
	case $opt in
		v) version=${OPTARG};;
		l) 
			if [[ ${OPTARG,,} = "static" ]]; then
				config_build_shared=false;
				config_build_static=true;
			elif [[ ${OPTARG,,} = "shared" ]]; then
				config_build_shared=true;
				config_build_static=false;
			elif [[ ${OPTARG,,} = "all" ]]; then
				config_build_shared=true;
				config_build_static=true;
			else
				exit 1;
			fi;;
		L) config_log_level=${OPTARG^^};;
		a) config_arch=${OPTARG^^};;
		f) config_fetch=true;;
		t) config_build_ts=true;;
		n) config_build_node=true;;
		c) config_build_cmake=true;;
		D) config_build_dependencies=true;;
		A) auto_detect;;
		*) printf "bad arg";;
	esac
done

#-------------------------------------------------------------------------------
#  prepare workspace and print configuration
#-------------------------------------------------------------------------------
print_config

if [[ ! -d ./dist/src ]]; then
	mkdir -p ./dist/src;
fi

#-------------------------------------------------------------------------------
# build cmake
#-------------------------------------------------------------------------------
if [[ ${config_build_cmake,,} = true ]]; then
	cmake "-S${dir_src}" \
		"-B${dir_build}" \
		"-DLOG_LEVEL=${config_log_level^^}" \
		"-DCMAKE_BUILD_TYPE=Release" \
		"-DCMAKE_INSTALL_PREFIX=$dir_prefix" \
		"-DBUILD_DEPENDENCIES=${config_build_dependencies^^}" \
		"-DBUILD_SHARED=${config_build_shared^^}" \
		"-DBUILD_STATIC=${config_build_static^^}" \
		"-DBUILD_APPS=FALSE" || exit 1;
		
	cmake --build "$dir_build" --config Release --target install || exit 1;
fi;

#-------------------------------------------------------------------------------
# build node binding
#-------------------------------------------------------------------------------
if [[ ${config_build_node,,} = true ]]; then
	if [[ ${config_build_shared,,} = true ]]; then
		LINQ_NETWORK_BUILD_SHARED=true \
			"$dir_node/.bin/node-gyp" configure || exit 1;
		LINQ_NETWORK_BUILD_SHARED=true \
			"$dir_node/.bin/node-gyp" build || exit 1;
	else
		"$dir_node/.bin/node-gyp" configure || exit 1;
		"$dir_node/.bin/node-gyp" build || exit 1;
	fi
	tar -czvf "$dir_node_prebuild/node-$config_arch.tar.gz" \
		-C "$dir_build/Release" linq.node;
fi

#-------------------------------------------------------------------------------
# build typescript
#-------------------------------------------------------------------------------
if [[ ${config_build_ts,,} = true ]]; then
	"$dir_node/.bin/tsc" -p "$dir_node_binding" || exit 1; 
	install_binding;
fi

#-------------------------------------------------------------------------------
# fetch tarball
#-------------------------------------------------------------------------------
if [[ ${config_fetch,,} = true ]]; then
	install_binding;
fi
