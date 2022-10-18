#!/bin/bash

set -e

CUR_DIR="$(readlink -f $(dirname "$BASH_SOURCE"))"
source $CUR_DIR/../ae_common.sh


mkdir -p $AE_NEWBUGS_DF_IA_TEST_DIR
mkdir -p $AE_NEWBUGS_DF_IA_TEST_RUN_DIR
mkdir -p $AE_NEWBUGS_DF_IA_TEST_PRED_DIR

KIT_PROG_DIR=$AE_NEWBUGS_DF_IA_TEST_PROG_DIR \
KIT_RUN_DIR=$AE_NEWBUGS_DF_IA_TEST_RUN_DIR \
KIT_PROFILE_TRACE_DIR=$AE_NEWBUGS_DF_IA_TEST_PROFILE_TRACE_DIR \
KIT_PROFILE_CONFIG_TMPL=$CUR_DIR/profile.json.tmpl \
KIT_TEST_CONFIG_TMPL=$CUR_DIR/test.json.tmpl  \
KIT_BASE_IMAGE=$AE_NEWBUGS_IMAGE \
KIT_IMAGE_SSHKEY=$AE_NEWBUGS_IMAGE_KEY \
KIT_KERNEL=$AE_NEWBUGS_KERNEL \
KIT_SNAPSHOT_IMAGE_DIR=$AE_NEWBUGS_DF_IA_TEST_SNAPSHOT_IMAGE_DIR \
KIT_MEMTRACE_MAP=$AE_NEWBUGS_DF_IA_TEST_PRED_DIR/memtrace_map_cs_1.gob \
KIT_PRED=$AE_NEWBUGS_DF_IA_TEST_PRED_DIR/pred_cs.json \
KIT_RESULT_DIR=$AE_NEWBUGS_DF_IA_TEST_RESULT_DIR \
$CUR_DIR/run.sh

# Aggregate test reports
result_dir="$(cat $AE_NEWBUGS_DF_IA_TEST_RUN_DIR/result_dir)"
result_cluster=$AE_NEWBUGS_DF_IA_TEST_DIR/aggregate.json
$MAIN_HOME/bin/resanalyze \
        -prog_dir $AE_NEWBUGS_DF_IA_TEST_PROG_DIR \
        -result_dir $result_dir \
        -result_cluster $result_cluster

# Print statistics of the test report aggregation
python3 $CUR_DIR/aggregate_stats.py $result_cluster