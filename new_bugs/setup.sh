#!/bin/bash

set -e

CUR_DIR="$(readlink -f $(dirname "$BASH_SOURCE"))"
source $CUR_DIR/../ae_common.sh

KERNEL_DIR=$MAIN_HOME/testsuite/kernel-memory-acccess-tracing/kernel/5.13/

# Build and instrument kernel
cp $CUR_DIR/.config $KERNEL_DIR/
pushd $KERNEL_DIR/ > /dev/null
make CC=$CC_MT/bin/gcc olddefconfig
make CC=$CC_MT/bin/gcc -j`nproc`
mkdir -p $AE_NEWBUGS_KERNEL_DIR
cp $KERNEL_DIR/arch/x86/boot/bzImage $AE_NEWBUGS_KERNEL_DIR

# Build image
mkdir -p $AE_NEWBUGS_IMAGE_DIR
pushd $AE_NEWBUGS_IMAGE_DIR/ > /dev/null
$MAIN_HOME/testsuite/image/create-image.sh

# Download test programs
mkdir -p $AE_NEWBUGS_PROG_DIR
pushd $AE_NEWBUGS_WORKDIR/ > /dev/null
wget https://www.cs.purdue.edu/homes/liu3101/kit/ae_newbugs_progs.tar.xz
tar -xf ae_newbugs_progs.tar.xz -C $AE_NEWBUGS_PROG_DIR --strip-components 2
rm ae_newbugs_progs.tar.xz