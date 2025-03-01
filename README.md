# 🚀 Assignment Solutions

This repository contains solutions for multiple assignments related to **Operating Systems** concepts, including **process synchronization**, **inter-process communication**, and **file system manipulation**. Each assignment is implemented in **C** and follows the given specifications.

## 📂 Folder Structure

- 📁 `a1/` - Solution for **Assignment 1**: 🗂️ File System Operations  
- 📁 `a2/` - Solution for **Assignment 2**: 🔄 Processes and Threads  
- 📁 `a3/` - Solution for **Assignment 3**: 🔗 Inter-Process Communication  
- 📁 `eval/` - 📜 Contains **intermediate and final grades** for the assignments  

Each assignment folder includes **source code**, necessary headers, and any additional scripts required for testing. 📜 **Compilation and execution instructions** are provided in each assignment's directory.

---

## 📜 Assignment Details

### 🗂️ **Assignment 1: File System Operations (`a1/`)**
This assignment focuses on interacting with a **custom binary file format** (`SF` - section file) and performing **file system operations**.

#### ✅ **Implemented Features**
- 📂 **Listing directory contents** with filtering options (**name suffix, permissions**).
- 📑 **Parsing and validating SF files** to extract metadata.
- 📝 **Extracting a specific line** from a section in an SF file (lines are stored in **reverse order**).
- 🔎 **Recursively searching** for SF files that meet specific criteria.

#### ⚠️ **Constraints**
- ❌ Only **low-level system calls** (`open()`, `read()`, `write()`) were allowed.
- ❌ **No use of higher-level C functions** such as `fopen()`, `fgets()`, or `fprintf()`.

---

### 🔄 **Assignment 2: Processes and Threads (`a2/`)**
This assignment involves **process creation**, **thread management**, and **synchronization in Linux**.

#### ✅ **Implemented Features**
- 🏗️ **Creating a specific hierarchy of processes**.
- 🔗 **Implementing thread synchronization** within a process.
- 🎯 **Enforcing execution order constraints** between threads.
- 🚧 **Implementing a thread barrier** to limit the number of concurrently running threads.
- 🔄 **Synchronizing threads across different processes**.

#### ⚠️ **Constraints**
- ✅ Used **POSIX threads (`pthread`)** and **semaphores** for synchronization.
- ❌ Allowed interaction with the testing script **only** through the provided `info()` and `init()` functions.
- 🚫 No file system interactions or certain system calls (e.g., `sleep()`, `nanosleep()`) were allowed.

---

### 🔗 **Assignment 3: Inter-Process Communication (`a3/`)**
This assignment involves **implementing inter-process communication** using **named pipes (FIFOs)** and **shared memory**.

#### ✅ **Implemented Features**
- 🛠️ **Establishing communication** via **named pipes** (`REQ_PIPE`, `RESP_PIPE`).
- 📡 **Handling various requests**, including:
  - 🏓 **Ping request** → Responding with `"PONG"`.
  - 🗄️ **Shared memory creation** and data writing.
  - 📄 **File mapping** using `mmap()`.
  - 📥 **Reading from a file** at offsets, from sections, or using a **logical memory space**.
  - ❌ **Cleaning up resources** properly upon receiving an **exit request**.

#### ⚠️ **Constraints**
- ✅ Used **memory-mapped files (`mmap()`)** instead of direct `read()` calls.
- ✅ Managed shared memory using **POSIX shared memory (`shm_open()`)**.
- 🔒 Followed a strict **request-response protocol**.

---

## 📌 Notes
- ✅ Each assignment has been tested using the provided **Python testing script (`tester.py`)**.
- 📊 The `eval/` folder contains **intermediate and final grades** for the assignments.
- 🛠️ Solutions adhere to the given constraints, ensuring **efficient and correct implementation**.
