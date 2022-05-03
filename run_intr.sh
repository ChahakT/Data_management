#!/bin/bash
arr=("SMART_INTERSECT" "SIMPLE_INTERSECT")

mkdir -p ../output/
set -x
for V in `seq 0 1`; do
	for f in ${arr[@]}; do
	    echo "(cmake -D_RUNTYPE=$f -D_BETA=58.9 -D_INTERSECT_V=${V} ../ && make -j2) &> ../output/build_${V}_${f}.txt"
	    (cmake -D_RUNTYPE=$f -D_BETA=58.9 -D_INTERSECT_V=${V} ../ && make -j2) &> ../output/build_${V}_${f}.txt
	    ../mpi-install-script.sh mpi-intersect-benchmark 2> ../output/err_${V}_${f}.txt | tee ../output/out_${V}_${f}.csv
	done
done
