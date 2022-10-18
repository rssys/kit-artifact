#!/bin/bash

set -e

cd $(readlink -f $(dirname $BASH_SOURCE))
wget https://www.cs.purdue.edu/homes/liu3101/kit/known_bugs_dep.tar.xz
tar -xJf ./known_bugs_dep.tar.xz
rm ./known_bugs_dep.tar.xz