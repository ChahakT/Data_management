#!/bin/bash
arr=("SIMPLE_AGG" "SMART_AGG" "SMART_AGG_V2")

mkdir ../output/

for f in ${arr[@]}; do
    cmake -D_RUNTYPE=$f -D_BETA=2.0 -D_INTERSECT_V=0 ../ && make -j && ../mpi-install-script.sh mpi-agg-benchmark 2> ../output/err_${f}.txt > ../output/out_${f}.csv
done
