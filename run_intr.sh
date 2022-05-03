#!/bin/bash
arr=("SIMPLE_INTERSECT" "SMART_INTERSECT")

mkdir -p ../output/

for f in ${arr[@]}; do
    (cmake -D_RUNTYPE=$f -D_BETA=58.9 -D_INTERSECT_V=0 ../ && make -j) &> ../output/build_0_${f}.txt
    ../mpi-install-script.sh mpi-intersect-benchmark 2> ../output/err_0_${f}.txt > ../output/out_0_${f}.csv
done

for f in ${arr[@]}; do
    (cmake -D_RUNTYPE=$f -D_BETA=58.9 -D_INTERSECT_V=1 ../ && make -j) &> ../output/build_1_${f}.txt
    ../mpi-install-script.sh mpi-intersect-benchmark 2> ../output/err_1_${f}.txt > ../output/out_1_${f}.csv
done