## Build and Run Instructions

1. **Navigate to the project directory:**
   ```bash
   cd iosche
   ```

2. **Build the program:**
   Create a build directory, compile the program using CMake and Make:
   ```bash
   mkdir build && cd build && cmake .. && make
   ```
  
3. **Execute the Program:**
   Use the provided script with paths to the build directory and the `oslab1` executable:
   ```bash
   ./runit.sh /home/yg3370/iosche/build /home/yg3370/iosche/build/iosche && ./gradeit.sh /home/yg3370/iosche/build .  
   ```
