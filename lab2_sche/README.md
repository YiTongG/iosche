 **Build the program:**
   Create a build directory, compile the program using CMake and Make:
   ```bash
   mkdir build && cd build && cmake .. && make
   ```
  
 **Execute the Program:**
   Use the provided script with paths to the build directory and the `oslab1` executable:
   ```bash
 ./runit.sh /home/yg3370/sche/build /home/yg3370/sche/build/scheduler && ./gradeit.sh /home/yg3370/sche/build .    ```

   Ensure the paths for `linker` and `outdir` are set correctly:
   - `linker`: `/home/yg3370/sche/build/scheduler`
   - `outdir`: `/home/yg3370/sche/build`
