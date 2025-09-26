#!/usr/bin/env python3
# parse_results.py

import re
import csv

data = []

with open('results.txt', 'r') as f:
    content = f.read()

# Parse sequential runs
seq_pattern = r'Running jacobi_seq (\d+) (\d+).*?Converged after (\d+) iterations.*?Elapsed time = +(\d+\.\d+) sec'
for match in re.finditer(seq_pattern, content, re.DOTALL):
    size = match.group(1)
    iterations = match.group(3)
    elapsed = match.group(4)
    data.append(['seq', size, '1', iterations, elapsed])

# Parse parallel runs
par_pattern = r'Running jacobi_cond (\d+) (\d+) (\d+).*?Converged after (\d+) iterations.*?Elapsed time = +(\d+\.\d+) sec'
for match in re.finditer(par_pattern, content, re.DOTALL):
    size = match.group(1)
    threads = match.group(3)
    iterations = match.group(4)
    elapsed = match.group(5)
    data.append(['par', size, threads, iterations, elapsed])

# Write CSV
with open('timings.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['Type', 'Size', 'Threads', 'Iterations', 'Elapsed_Time'])
    writer.writerows(data)

print(f"Parsed {len(data)} results into timings.csv")