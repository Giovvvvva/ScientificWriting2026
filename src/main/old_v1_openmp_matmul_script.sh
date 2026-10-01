#!/bin/bash

cd /scicore/home/s-gpu-course/course_gpu_10/project/Mat_Mul/v1_openmp

mkdir -p logs results bin

rm -f logs/*

rm -f results/*

rm -rf bin/*

# best for sequential standard was close between 64 and 128
BLOCK_SIZES=(
    128
)

MAT_DIM=(
    32768
)

THREAD_COUNTS=(
    1
    2
    4
    8
    16
    32
    64
    128
)



REPS=5

for M in "${MAT_DIM[@]}"
do
    for N in "${BLOCK_SIZES[@]}"
    do
        if (( M < N || M % N != 0 )); then
            echo "Skipping block size ${N}: MAT_DIM=${M} is not divisible by it."
            continue
        fi
        for THREADS in "${THREAD_COUNTS[@]}"
        do
            for REP in $(seq 1 $REPS)
            do
                echo "Submitting M=${M}, N=${N}, THREADS=${THREADS}, REP=${REP}"

                sbatch \
                --cpus-per-task=${THREADS} \
                --export=ALL,OUTPUT_DIR=v1_openmp_results,M=${M},N=${N},THREADS=${THREADS},REP=${REP} \
                    v1_openmp_matmul.job
            done
        done
    done
done

