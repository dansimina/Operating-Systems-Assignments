### 🔗 Assignment 3: Inter-Process Communication (`a3/`)
This assignment involves **implementing inter-process communication** using **named pipes (FIFOs)** and **shared memory**.

#### Implemented Features
- Establishing communication via **named pipes** (`REQ_PIPE`, `RESP_PIPE`).
- Handling various requests, including:
  - Ping request → Responding with `"PONG"`.
  - Shared memory creation and data writing.
  - File mapping using `mmap()`.
  - Reading from a file at offsets, from sections, or using a **logical memory space**.
  - Cleaning up resources properly upon receiving an **exit request**.

#### Constraints
- Used **memory-mapped files (`mmap()`)** instead of direct `read()` calls.
- Managed shared memory using **POSIX shared memory (`shm_open()`)**.
- Followed a strict **request-response protocol**.
