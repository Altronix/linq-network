#!/bin/bash - 
#===============================================================================
#
#          FILE: fetch-tar.sh
# 
#         USAGE: ./fetch-tar.sh -a linux-arm32 -o file.tar.gz
# 
#   DESCRIPTION: Grab the linq-network tarball from the internet and extract it
# 		 to a specific location.
# 
#       OPTIONS: ---
#
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

#-------------------------------------------------------------------------------
# Helpful variables
#-------------------------------------------------------------------------------
scripts=$(eval dirname "$0");
downloads="https://github.com/altronix/linq-network/releases/download";
version=$(eval node "${scripts}/read_version.js");
arch=linux-x64;
out="${PWD}/linq.node";

#-------------------------------------------------------------------------------
# parse args
#-------------------------------------------------------------------------------
while getopts "a:o:" opt; do
	case $opt in
		a) arch="$OPTARG";;
		o) out="$OPTARG";;
		*) printf "bad arg";;
	esac
done

#-------------------------------------------------------------------------------
# create a temporary workspace directory
#-------------------------------------------------------------------------------
rm -rf ./.linq-tmp;
mkdir ./.linq-tmp;
cd ./.linq-tmp || exit;

#-------------------------------------------------------------------------------
# fetch tarball
#-------------------------------------------------------------------------------
wget "${downloads}/v${version}/node-${arch}.tar.gz" || exit;
tar -xzvf "./node-${arch}.tar.gz" || exit;
cd ..;
cp "./.linq-tmp/${arch}/linq.node" "${out}";

#-------------------------------------------------------------------------------
# clean up
#-------------------------------------------------------------------------------
rm -rf ./.linq-tmp;
