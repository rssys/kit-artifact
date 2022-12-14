#!/bin/bash

set -e

CUR_DIR="$(readlink -f $(dirname "$BASH_SOURCE"))"
source $CUR_DIR/../ae_common.sh

mkdir -p $AE_NEWBUGS_BASIC_TEST_DIR

# Build a mini exemplar test program corpus that could reproduce bugs found by KIT
mkdir -p $AE_NEWBUGS_BASIC_TEST_PROG_DIR
# 1: ptype
cp $AE_NEWBUGS_PROG_DIR/27060 $AE_NEWBUGS_PROG_DIR/93663 $AE_NEWBUGS_BASIC_TEST_PROG_DIR/
# 2: flowlabel/sendmsg
cp $AE_NEWBUGS_PROG_DIR/39965 $AE_NEWBUGS_PROG_DIR/79183 $AE_NEWBUGS_BASIC_TEST_PROG_DIR/
# 3: bind rds socket
cp $AE_NEWBUGS_PROG_DIR/82929 $AE_NEWBUGS_BASIC_TEST_PROG_DIR/
# 4: flowlabel/connect
cp $AE_NEWBUGS_PROG_DIR/39965 $AE_NEWBUGS_PROG_DIR/12463 $AE_NEWBUGS_BASIC_TEST_PROG_DIR/
# 5: socket alloc count in /proc/net/sockstat
cp $AE_NEWBUGS_PROG_DIR/35657 $AE_NEWBUGS_PROG_DIR/10120 $AE_NEWBUGS_BASIC_TEST_PROG_DIR/
# 6: socket cookie
cp $AE_NEWBUGS_PROG_DIR/3467 $AE_NEWBUGS_BASIC_TEST_PROG_DIR/
# 7: assoc id
cp $AE_NEWBUGS_PROG_DIR/75793 $AE_NEWBUGS_PROG_DIR/45898 $AE_NEWBUGS_BASIC_TEST_PROG_DIR/
# 8: proto memory usage in /proc/net/sockstat
cp $AE_NEWBUGS_PROG_DIR/5960 $AE_NEWBUGS_PROG_DIR/10120 $AE_NEWBUGS_BASIC_TEST_PROG_DIR/
# 9: proto memory usage in /proc/net/protocols
cp $AE_NEWBUGS_PROG_DIR/39728 $AE_NEWBUGS_PROG_DIR/85119 $AE_NEWBUGS_BASIC_TEST_PROG_DIR/

mkdir -p $AE_NEWBUGS_BASIC_TEST_RUN_DIR
mkdir -p $AE_NEWBUGS_BASIC_TEST_PRED_DIR

KIT_PROFILE_DEBUG=1 \
KIT_PROG_DIR=$AE_NEWBUGS_BASIC_TEST_PROG_DIR \
KIT_RUN_DIR=$AE_NEWBUGS_BASIC_TEST_RUN_DIR \
KIT_PROFILE_TRACE_DIR=$AE_NEWBUGS_BASIC_TEST_PROFILE_TRACE_DIR \
KIT_PROFILE_CONFIG_TMPL=$CUR_DIR/profile.json.tmpl \
KIT_TEST_CONFIG_TMPL=$CUR_DIR/test.json.tmpl  \
KIT_BASE_IMAGE=$AE_NEWBUGS_IMAGE \
KIT_IMAGE_SSHKEY=$AE_NEWBUGS_IMAGE_KEY \
KIT_KERNEL=$AE_NEWBUGS_KERNEL \
KIT_SNAPSHOT_IMAGE_DIR=$AE_NEWBUGS_BASIC_TEST_SNAPSHOT_IMAGE_DIR \
KIT_MEMTRACE_MAP=$AE_NEWBUGS_BASIC_TEST_PRED_DIR/memtrace_map_cs_1.gob \
KIT_PRED=$AE_NEWBUGS_BASIC_TEST_PRED_DIR/pred_cs.json \
KIT_RESULT_DIR=$AE_NEWBUGS_BASIC_TEST_RESULT_DIR \
VM_COUNT=10 \
$CUR_DIR/run.sh

# Aggregate test reports
result_dir="$(cat $AE_NEWBUGS_BASIC_TEST_RUN_DIR/result_dir)"
result_cluster=$AE_NEWBUGS_BASIC_TEST_DIR/aggregate.json
$MAIN_HOME/bin/resanalyze \
        -prog_dir $AE_NEWBUGS_BASIC_TEST_PROG_DIR \
        -result_dir $result_dir \
        -result_cluster $result_cluster

# Print statistics of the test report aggregation
python3 $CUR_DIR/aggregate_stats.py $result_cluster
