### 🔄 Assignment 2: Processes and Threads (`a2/`)
This assignment involves **process creation**, **thread management**, and **synchronization in Linux**.

#### Implemented Features
- Creating a specific hierarchy of processes.
- Implementing thread synchronization within a process.
- Enforcing execution order constraints between threads.
- Implementing a thread barrier to limit the number of concurrently running threads.
- Synchronizing threads across different processes.

#### Constraints
- Used **POSIX threads (`pthread`)** and **semaphores** for synchronization.
- Allowed interaction with the testing script **only** through the provided `info()` and `init()` functions.
- No file system interactions or certain system calls (e.g., `sleep()`, `nanosleep()`) were allowed.
