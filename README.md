# Operating Systems Memory Management Simulation

## Overview
This project simulates **Paged Memory Allocation** and **Demand Paged Memory Allocation** concepts from Chapter 3 of *Understanding Operating Systems* by McHoes & Flynn (8th edition).  
The work is divided into two programs:

1. **Program 1 – Paged Memory Allocation**
   - Accepts a job.
   - Divides the job into pages based on page size.
   - Calculates and displays internal fragmentation.
   - Randomly loads all pages of a job into available memory frames.
   - Performs logical to physical address resolution.
   - Displays Job Table (JT), Page Map Table (PMT), and Memory Map Table (MMT).

2. **Program 2 – Demand Paged Memory Allocation**
   - Accepts **multiple jobs**.
   - Divides jobs into pages (but does not load them immediately).
   - Pages are loaded **on demand** only when referenced (demand paging).
   - Simulates **page faults** and performs page replacement using **FIFO** or **LRU** algorithms.
   - Maintains and displays JT, PMT, and MMT dynamically.

These simulations demonstrate **virtual memory concepts**, particularly **paging, page faults, and page replacement algorithms**.

---

## Task Requirements (from lecturer)
- **Paged Memory Allocation**
  - Accept a Job.
  - Divide the Job into pages by page size.
  - Indicate internal fragmentation.
  - Load jobs into frames randomly.
  - Perform address resolution.

- **Demand Paged Memory Allocation**
  - Accept multiple Jobs.
  - Divide the Jobs into pages.
  - Load jobs into frames only when needed (on demand).
  - Perform address resolution.
  - Implement and compare **FIFO** and **LRU** replacement algorithms.

---

## Program 1: Paged Memory Allocation

### Key Features
- **Divide jobs into pages**: Calculates number of pages and leftover bytes.
- **Internal fragmentation**: If the last page is not fully used, wasted space is recorded.
- **Random frame allocation**: Jobs are mapped to random free frames.
- **Address resolution**: Logical addresses are mapped to physical addresses (frame number + offset).
- **Tables displayed**:
  - **Job Table**: Job ID, size, number of pages, internal fragmentation.
  - **Page Map Table**: Page numbers and their assigned frame numbers.
  - **Memory Map Table**: Frame number, availability, job ID, and page number.

### Demonstrated Concepts
- Fixed page size and fragmentation.
- Random allocation of physical memory frames.
- Basic address translation.

---

## Program 2: Demand Paged Memory Allocation

### Key Features
- **Multiple job support**: Imports jobs from a CSV file.
- **Demand paging**: Pages are *not loaded until first accessed*.  
  - Initial state: All frames free.  
  - On access: Page faults occur → missing page loaded.  
  - Already loaded pages → page hit (no fault).
- **Page replacement algorithms**:
  - **FIFO (First In, First Out)**: Oldest loaded page is evicted.
  - **LRU (Least Recently Used)**: Page that has not been used for the longest time is evicted.
- **Dynamic tables**:
  - **Job Table**: Job size, pages, pages currently loaded, page faults.
  - **Page Map Table**: Page–frame mappings, loaded status, and reference/modified bits.
  - **Memory Map Table**: Frame usage, job ID, page number, access times.

### Demonstrated Concepts
- **Page faults**: Triggered when a page is accessed but not in memory.
- **Replacement algorithms**: FIFO queue vs LRU time stamps.
- **Virtual memory behavior**: Jobs appear larger than physical memory and still execute by swapping pages.

---

## How the Programs Work

### Input
- Jobs are read from a CSV file (`jobs.csv`) with format:
```
jobID,jobSize
```

- Page size and memory frame size are fixed in the code (`512 bytes` in this simulation).

### Output
- **Menu-driven simulation**:
- View loaded jobs.
- Run simulation (allocation or demand paging).
- View JT, PMT, MMT tables.
- Perform address resolution.

### Example Flow (Demand Paging)
1. Program starts → user selects page replacement algorithm (FIFO or LRU).
2. Jobs loaded into Job Table (pages known but none loaded).
3. Simulation runs: random memory accesses cause **page faults**.
4. Pages are loaded into frames, with replacement if memory is full.
5. Final tables show:
 - How many pages were loaded.
 - Total page faults per job.
 - Current memory map.

---

## Important Structures in Code

- **Job**
- `jobID`, `jobSize`, `pageSize`
- `pages`: vector of page numbers
- `pageTable`: page → frame mapping
- `loadedPages`: pages currently in memory
- `pageFaults`: counter for faults

- **PageFrame**
- `frameID`, `frameSize`
- `isFree`: availability
- `jobID`: which job currently owns it
- `pageNumber`: which page is stored
- `accessTime`: used for LRU
- `modified`, `referenced`: status bits

---

## Differences Between Program 1 and Program 2

| Feature                      | Program 1: Paged Allocation | Program 2: Demand Paging |
|------------------------------|-----------------------------|---------------------------|
| **Jobs supported**           | One at a time              | Multiple jobs             |
| **Page loading**             | All pages loaded randomly  | Loaded on demand only     |
| **Internal fragmentation**   | Tracked and displayed      | Not tracked (demand paging ignores) |
| **Replacement algorithms**   | Not required               | FIFO and LRU implemented |
| **Fault handling**           | Not applicable             | Page faults trigger loads/replacements |

---

## How to Run
1. Compile the code (g++ or any C++ compiler):
 ```bash
 g++ paged_allocation.cpp -o paged_allocation
 g++ demand_paging.cpp -o demand_paging
```

2. Run executables:
```bash
./paged_allocation
./demand_paging
```

3. Ensure jobs.csv file exists in the same directory.