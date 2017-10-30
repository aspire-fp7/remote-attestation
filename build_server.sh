#!/bin/bash
set -o errexit
set -o pipefail
set -o nounset
#set -o xtrace

# Get the repo and build directories, go to the build directory
repo_dir=$(dirname $0)
build_dir=$1
mkdir -p $build_dir

# Build the server-side
echo "**** RA server environment setup ****"
makeOutTmpDir=$(mktemp -d)
make -C $repo_dir/src -s  manager OUTDIR=$makeOutTmpDir SRCDIR=$repo_dir/src ASCL=$2
mv $makeOutTmpDir/manager_exe $build_dir/ra_manager
rm -rf $makeOutTmpDir
echo "**** Server environment setup completed****"
