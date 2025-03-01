### 🗂️ Assignment 1: File System Operations (`a1/`)
This assignment focuses on interacting with a **custom binary file format** (`SF` - section file) and performing **file system operations**.

#### Implemented Features
- Listing directory contents with filtering options (**name suffix, permissions**).
- Parsing and validating SF files to extract metadata.
- Extracting a specific line from a section in an SF file (lines are stored in **reverse order**).
- Recursively searching for SF files that meet specific criteria.

#### Constraints
- Only **low-level system calls** (`open()`, `read()`, `write()`) were allowed.
- **No use of higher-level C functions** such as `fopen()`, `fgets()`, or `fprintf()`.
