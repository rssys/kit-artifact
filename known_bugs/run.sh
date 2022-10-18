#!/bin/bash

set -e
source $(readlink -f $(dirname $BASH_SOURCE))/../ae_common.sh

AE_KB_DEP_DIR=$AE_HOME/known_bugs/dep
export AE_KB_KERNEL_DIR=$AE_KB_DEP_DIR/kernel
export AE_KB_IMAGE_DIR=$AE_KB_DEP_DIR/images

echo "Checking the image directory..."
if [ ! -d $AE_KB_IMAGE_DIR ]
then
        echo "No image directory at $AE_KB_IMAGE_DIR."
else
        echo "Found $AE_KB_IMAGE_DIR."
fi

echo "Checking the kernel directory..."
if [ ! -d $AE_KB_KERNEL_DIR ]
then
        echo "No image directory at $AE_KB_KERNEL_DIR"
else
        echo "Found $AE_KB_KERNEL_DIR."
fi

for dir in $AE_HOME/known_bugs/src/*/; do
        echo "Reproducing known bug in $dir..."
        echo "Generating config file..."
        envsubst < $dir/config.json.tmpl | tee $dir/config.json
        echo "Running test..."
        $MAIN_HOME/bin/manager -test -config=$dir/config.json -debug
done