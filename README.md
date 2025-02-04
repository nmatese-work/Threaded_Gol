# Conway's Game of Life - Parallel Implementation

## Overview
This project is an implementation of **Conway's Game of Life** using parallel programming techniques. It was developed as part of the **CS 31 course at Swarthmore College**. The program supports three different output modes:

- **No animation** (Performance mode)
- **ASCII animation** (Terminal visualization)
- **ParaVisi animation** (Graphical visualization using ParaVisi library)

## Features
- Multithreaded execution using **POSIX threads (pthreads)**
- Supports both **row-wise and column-wise parallelism**
- Synchronization using **mutex locks** and **barriers**
- Dynamic memory allocation for optimized board storage
- Performance measurement through runtime tracking

## Installation & Compilation
### Prerequisites
Ensure you have the following installed:
- **GCC (GNU Compiler Collection)**
- **POSIX threads (pthreads) library**
- **ParaVisi library** (for graphical visualization mode)

### Compilation
To compile the program, use:
```sh
make
```
This will generate an executable named `gol`.

## Usage
Run the program using the following command:
```sh
./gol <config_file> <output_mode> <num_threads> <row_vs_col> <print_info>
```
### Arguments:
- `<config_file>`: Input file containing the initial board state
- `<output_mode>`:
  - `0` - No visualization
  - `1` - ASCII animation
  - `2` - ParaVisi animation
- `<num_threads>`: Number of threads to use for parallel execution
- `<row_vs_col>`:
  - `0` - Row-wise parallelization
  - `1` - Column-wise parallelization
- `<print_info>`:
  - `0` - Do not print partitioning info
  - `1` - Print partitioning info

### Example Runs:
```sh
./gol file1.txt 0 4 0 1  # No animation, 4 threads, row-wise, print info
./gol file1.txt 1 2 1 0  # ASCII animation, 2 threads, column-wise, no info
./gol file1.txt 2 8 0 1  # ParaVisi animation, 8 threads, row-wise, print info
```

## File Format (Configuration File)
The configuration file should be formatted as follows:
```
<rows>
<columns>
<iterations>
<initial_live_cells_count>
<row1> <col1>
<row2> <col2>
...
```
Example:
```
10
10
50
5
1 1
2 2
3 3
4 4
5 5
```
This defines a **10x10** grid, running for **50 iterations**, with **5 initial live cells** at the specified coordinates.

## Implementation Details
- The main **struct gol_data** holds all necessary simulation data.
- **pthread_mutex_t my_mutex** ensures safe access to shared variables.
- **pthread_barrier_t my_barrier** synchronizes threads at each iteration.
- Each thread calculates a partition of the grid, updating it based on **Game of Life rules**.
- Synchronization mechanisms prevent race conditions and ensure correctness.

## Performance Optimization
- The simulation dynamically assigns grid partitions to threads for efficient load balancing.
- The **animation step is adjustable** via `SLEEP_USECS` to modify animation speed.
- Memory is allocated efficiently using **1D array representation for 2D grids**.

## Author
**Nick Matese**  
**Date:** 12/11/24  
Swarthmore College, CS 31

## License
**Copyright (c) 2023 Swarthmore College Computer Science Department, Swarthmore PA**

