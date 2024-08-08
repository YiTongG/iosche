

# Lab 1 Linker - README

## Build and Run Instructions

1. **Navigate to the project directory:**
   ```bash
   cd lab1_linker
   ```

2. **Build the program:**
   Create a build directory, compile the program using CMake and Make:
   ```bash
   mkdir build && cd build && cmake .. && make
   ```
  
3. **Execute the Program:**
   Use the provided script with paths to the build directory and the `oslab1` executable:
   ```bash
   ./runit.sh /home/yg3370/lab1_linker/build /home/yg3370/lab1_linker/build/oslab1
   ```

   Ensure the paths for `linker` and `outdir` are set correctly:
   - `linker`: `/home/yg3370/lab1_linker/build/oslab1`
   - `outdir`: `/home/yg3370/lab1_linker/build`
