# OS Concepts Project — Top 10 Frequent Words in enwik8

## Overview

This project finds the **top 10 most frequent words** in the **enwik8** dataset using three different implementations written in **C**. It compares their performance to study how multiprocessing and multithreading affect execution time on a multi-core machine.

---

## Course Information

- **University:** Birzeit University
- **Faculty:** Engineering & Technology
- **Department:** Electrical & Computer Engineering
- **Course:** ENCS3390 – Operating System Concepts
- **Semester:** First Semester, 2024/2025
- **Project:** Project 1
- **Student:** Veronica Wakileh — 1220245

---

## Project Objective

Compare three approaches to the same problem and measure their execution time:

1. **Naive (sequential)** — single process, no threads.
2. **Multiprocessing** — multiple child processes running in parallel.
3. **Multithreading** — multiple POSIX threads running in parallel.

For both parallel approaches, the program is tested with **2, 4, 6, and 8 workers**, and the results are analyzed using **Amdahl's Law**.

---

## Dataset

- **Name:** enwik8 (a 100 MB extract of English Wikipedia)
- **Source:** [https://huggingface.co/datasets/LTCB/enwik8](https://huggingface.co/datasets/LTCB/enwik8)

The dataset must be downloaded and saved as `test.txt` in the project directory before running the programs (this is the filename used inside the source code).

---

## Implemented Approaches

### 1. Naive Approach (`main.c`)
- Reads the dataset into one large array.
- Builds a unique-word frequency table sequentially.
- Prints the top 10 words.
- No parallelism — used as the baseline.

### 2. Multiprocessing Approach (`Multiprocessing.c`)
- The dataset is divided into equal chunks based on the number of child processes.
- Child processes are created with `fork()` and the parent waits for them with `wait()`.
- A shared-memory region (`mmap` with `MAP_SHARED | MAP_ANONYMOUS`) is used to merge each child's partial frequency table into a global one.
- Number of child processes is configured by the `NUM_PROCESSES` macro (tested with 2, 4, 6, 8).

### 3. Multithreading Approach (`Multithreading.c`)
- The dataset is divided into equal chunks based on the number of threads.
- Threads are created with `pthread_create()` and joined with `pthread_join()`.
- A `pthread_mutex_t` protects the global frequency table while threads merge their local results.
- Number of threads is configured by the `NUM_THREADS` macro (tested with 2, 4, 6, 8).

---

## Files Used

| File Name | Description |
|-----------|-------------|
| `main.c` | Naive sequential implementation. |
| `Multiprocessing.c` | Multiprocessing implementation using `fork()`, `wait()`, and shared memory. |
| `Multithreading.c` | Multithreading implementation using POSIX threads and a mutex. |

> The dataset file (`test.txt` / `enwik8`) is **not** included in the repository because of its size and must be downloaded separately.

---

## How to Compile and Run

Make sure `test.txt` (the enwik8 dataset) is in the same directory as the compiled programs.

```bash
# 1. Naive (sequential)
gcc main.c -o main
./main

# 2. Multiprocessing
gcc Multiprocessing.c -o multiprocessing
./multiprocessing

# 3. Multithreading (requires -pthread)
gcc Multithreading.c -o multithreading -pthread
./multithreading
```

To test with different numbers of workers, change the macro at the top of the source file and recompile:
- `Multiprocessing.c` → `#define NUM_PROCESSES 2 / 4 / 6 / 8`
- `Multithreading.c` → `#define NUM_THREADS 2 / 4 / 6 / 8`

---

## Performance Summary

| Approach          | Workers           | Execution Time (minutes) |
|-------------------|-------------------|--------------------------|
| Naive             | 1                 | 36.327685 |
| Multiprocessing   | 2 child processes | 15.187493 |
| Multiprocessing   | 4 child processes | 10.616745 |
| Multiprocessing   | 6 child processes | 7.758493  |
| Multiprocessing   | 8 child processes | 8.338094  |
| Multithreading    | 2 threads         | 10.255081 |
| Multithreading    | 4 threads         | 9.126498  |
| Multithreading    | 6 threads         | 8.256276  |
| Multithreading    | 8 threads         | 10.283141 |

**Best multiprocessing result:** 6 child processes (≈ 7.76 minutes).
**Best multithreading result:** 6 threads (≈ 8.26 minutes).

---

## Amdahl's Law

Amdahl's Law estimates the maximum theoretical speedup of a program when only part of it can be parallelized:

```
Speedup = 1 / ( S + (1 - S) / N )
```

Where:
- **S** is the serial fraction of the program.
- **N** is the number of cores or workers.

From the report:
- **Serial fraction (S) ≈ 0.000275%**
- **Maximum theoretical speedup ≈ 9.9977** (based on the available cores)
- **Optimal number of workers = 6** (best execution time for both multiprocessing and multithreading)

---

## Result Discussion

- The **naive** version was the slowest, since it does not use any parallelism.
- **Multiprocessing** improved performance significantly compared to the naive version, with the best result at **6 child processes**.
- **Multithreading** also improved performance, with the best result at **6 threads**.
- Going from 6 to **8 workers did not always improve performance**. The added overhead from creating extra processes/threads, scheduling, and merging partial results outweighs the benefit of more parallelism on this machine.
- Multiprocessing slightly outperformed multithreading at the optimal worker count, mainly because each process has its own memory and avoids contention on a shared mutex.

---

## Notes

- The project must be tested on a machine with **at least 4 CPU cores**, as required by the assignment.
- Execution times will vary depending on hardware, operating system, and current system load.
- Compiled executables (`main.exe`, `multiprocessing`, `multithreading`, etc.) are **not** included in the repository — they are produced by running the `gcc` commands above.

---

## Conclusion

This project shows in practice how parallelism affects the runtime of a real, data-heavy task. The naive version is simple but slow. Multiprocessing and multithreading both reduce the execution time noticeably, but only up to a point — adding more workers beyond the optimal number (6 in this case) increases overhead instead of improving performance. The Amdahl's Law analysis confirms that the small serial fraction limits the maximum possible speedup, and the measurements match the expected behavior of a multi-core system.
