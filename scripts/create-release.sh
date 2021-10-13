#!/bin/bash - 
#===============================================================================
#
#          FILE: create-release.sh
# 
#         USAGE: ./create-release.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#       CREATED: 10/13/2021 16:06
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

#-------------------------------------------------------------------------------
# Helpful file locations
#-------------------------------------------------------------------------------
dir_scripts=$(eval realpath "$(eval dirname "$0")");
dir_src=$(eval realpath "$dir_scripts/..");

cd "$dir_src/bindings/node/prebuilds" || exit 1;
tar -czvf node-linux-arm32.tar.gz ./linux-arm32;
tar -czvf node-linux-x64.tar.gz ./linux-x64;

# todo push to github and bitbucket release assets using curl api
