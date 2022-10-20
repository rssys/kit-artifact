#!/bin/bash

build_config_dir=$(readlink -f $1)

source $build_config_dir/build_env.sh

build_args="-k /linux"
if [ -z "$COMMIT_ID" ]; then
        echo "Require COMMIT_ID."
        exit 1
fi
build_args+=" -c $COMMIT_ID"
if [ -z "$CC" ]; then
        echo "Require CC."
        exit 1
fi
build_args+=" -x $CC"
if [ -z "$CONFIG" ]; then
        echo "Require CONFIG."
        exit 1
fi
build_args+=" -f /linux-cfg/$CONFIG"
if [ ! -z "$PATCH" ]; then
        build_args+=" -p /linux-cfg/$PATCH"
fi
sudo docker run -v $(readlink -f $KERNEL)/:/linux -v $(readlink -f $1):/linux-cfg $DOCKER_IMAGE $build_args