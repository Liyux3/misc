#!/bin/bash

# benchmark.sh - Run all benchmarks

echo "=== Sequential Benchmarks ===" > results.txt
echo "" >> results.txt

for size in 200 400 600 800; do
    echo "Running jacobi_seq $size $size" | tee -a results.txt
    ./jacobi_seq $size $size | tee -a results.txt
    echo "" >> results.txt
done

echo "=== Parallel Benchmarks ===" >> results.txt
echo "" >> results.txt

for size in 200 400 600 800 1000; do
    for threads in 2 4 8 16; do
        echo "Running jacobi_cond $size $size $threads" | tee -a results.txt
        ./jacobi_cond $size $size $threads | tee -a results.txt
        echo "" >> results.txt
    done
done

echo "Benchmarking complete. Results in results.txt"