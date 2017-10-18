#!/bin/bash
set -o errexit
set -o pipefail
set -o nounset
#set -o xtrace

# Get the repo and build directories, go to the build directory
repo_dir=$(dirname $0)
build_dir=$1
mkdir -p $build_dir
cd $build_dir

# Create extra symlinks
ln -s $repo_dir/deploy/ $build_dir
ln -s $repo_dir/scripts/ $build_dir
ln -s $repo_dir/setup/ $build_dir
ln -s $repo_dir/src/ $build_dir

# Build objects and do setup
mkdir -p $build_dir/obj
$repo_dir/setup/remote_attestation_setup.sh
cd $build_dir/obj
$repo_dir/setup/generate_racommons.sh -o .
