#!/bin/bash

set -e

if [ ! -z "$KIT_PROFILE_DEBUG" ]; then
        echo "Enable debugging mode for profiling."
fi
if [ -z "$KIT_PROG_DIR" ]; then
        echo "KIT_PROG_DIR not defined!"
        exit 1
fi
if [ -z "$KIT_RUN_DIR" ]; then
        echo "KIT_RUN_DIR not defined!"
        exit 1
fi
if [ -z "$KIT_PROFILE_TRACE_DIR" ]; then
        echo "KIT_PROFILE_TRACE_DIR not defined!"
        exit 1
fi
if [ -z "$KIT_PROFILE_CONFIG_TMPL" ]; then
        echo "KIT_PROFILE_CONFIG_TMPL not defined!"
        exit 1
fi
if [ -z "$KIT_TEST_CONFIG_TMPL" ]; then
        echo "KIT_TEST_CONFIG_TMPL not defined!"
        exit 1
fi
if [ -z "$KIT_BASE_IMAGE" ]; then
        echo "KIT_BASE_IMAGE not defined!"
        exit 1
fi
if [ -z "$KIT_IMAGE_SSHKEY" ]; then
        echo "KIT_IMAGE_SSHKEY not defined!"
        exit 1
fi
if [ -z "$KIT_KERNEL" ]; then
        echo "KIT_KERNEL not defined!"
        exit 1
fi
if [ -z "$KIT_SNAPSHOT_IMAGE_DIR" ]; then
        echo "KIT_SNAPSHOT_IMAGE_DIR not defined!"
        exit 1
fi
if [ -z "$KIT_MEMTRACE_MAP" ]; then
        echo "KIT_MEMTRACE_MAP not defined!"
        exit 1
fi
if [ -z "$KIT_PRED" ]; then
        echo "KIT_PRED not defined!"
        exit 1
fi
if [ -z "$KIT_RESULT_DIR" ]; then
        echo "KIT_RESULT_DIR not defined!"
        exit 1
fi
if [ -z "$VM_COUNT" ]; then
        export VM_COUNT=`expr $(nproc) \* 5 / 6`
        echo "No VM_COUNT specified, let it be $VM_COUNT"
fi

# Profile kernel memory access for programs
profile_config_basename="$(basename -- $KIT_PROFILE_CONFIG_TMPL)"
KIT_PROFILE_CONFIG=$KIT_RUN_DIR/"${profile_config_basename%.*}"
envsubst < $KIT_PROFILE_CONFIG_TMPL | tee $KIT_PROFILE_CONFIG
profile_args="-profile -config=$KIT_PROFILE_CONFIG"
if [ ! -z "$KIT_PROFILE_DEBUG" ]; then
        profile_args+=" -debug"
fi
$MAIN_HOME/bin/manager $profile_args
export KIT_PROFILE_TRACE_DATA_DIR="$(cat $KIT_RUN_DIR/profile_dir)"
mkdir -p $KIT_SNAPSHOT_IMAGE_DIR
export KIT_SNAPSHOT_IMAGE=$KIT_SNAPSHOT_IMAGE_DIR/"$(date +%Y%m%d_%H%M%S)"
KIT_SNAPSHOT_IMAGE_SRC="$(cat $KIT_RUN_DIR/snapshot_image)"
cp $KIT_SNAPSHOT_IMAGE_SRC $KIT_SNAPSHOT_IMAGE

# Generate test case prediction file
$MAIN_HOME/bin/pmcpc                                            \
        -gen_memtrace_map                                       \
        -prog_dir $KIT_PROG_DIR                                 \
        -trace_dir $KIT_PROFILE_TRACE_DATA_DIR                  \
        -max_call_stack 1                                       \
        -memtrace_map $KIT_MEMTRACE_MAP                         \
        -thread $(nproc)
$MAIN_HOME/bin/pmcpc                                            \
        -gen_pc_pred                                            \
        -memtrace_map $KIT_MEMTRACE_MAP                         \
        -max_prog 5                                             \
        -pred $KIT_PRED

# Run test cases
test_config_basename="$(basename -- $KIT_TEST_CONFIG_TMPL)"
KIT_TEST_CONFIG=$KIT_RUN_DIR/"${test_config_basename%.*}"
envsubst < $KIT_TEST_CONFIG_TMPL | tee $KIT_TEST_CONFIG
$MAIN_HOME/bin/manager -test -config=$KIT_TEST_CONFIG