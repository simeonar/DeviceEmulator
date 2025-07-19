# Troubleshooting: yaml-cpp/yaml.h not found during build

If you see an error like:

```
fatal error C1083: Cannot open include file: 'yaml-cpp/yaml.h': No such file or directory
```

Follow these steps to resolve it:

## 1. Check vcpkg installation
- Make sure vcpkg is installed in your project directory (e.g., `D:/Prgrmrng/Projects/DeviceEmulator/vcpkg`).
- Check that yaml-cpp is installed for the correct triplet:
  ```
  .\vcpkg\vcpkg.exe list | findstr yaml-cpp
  ```
  You should see something like `yaml-cpp:x64-windows`.
  If not, install it:
  ```
  .\vcpkg\vcpkg.exe install yaml-cpp:x64-windows
  ```

## 2. Clean build directory
- Delete the `build` directory completely:
  ```
  Remove-Item -Recurse -Force .\build
  ```

## 3. Configure CMake with vcpkg toolchain and triplet
- Run CMake with the absolute path to the vcpkg toolchain and specify the triplet:
  ```
  cmake -S iot-emulator -B build -DCMAKE_TOOLCHAIN_FILE=D:/Prgrmrng/Projects/DeviceEmulator/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
  ```

## 4. Fix CMakeLists.txt for devices
- Make sure `iot-emulator/devices/CMakeLists.txt` contains:
  ```cmake
  target_link_libraries(devices PUBLIC mqtt_device yaml-cpp::yaml-cpp)
  ```
  This ensures yaml-cpp include directories are available for all device sources.

## 5. Build the project
- Build with:
  ```
  cmake --build build --config Release
  ```

## 6. If the error persists
- Check that the file `yaml-cppConfig.cmake` exists in:
  ```
  D:/Prgrmrng/Projects/DeviceEmulator/vcpkg/installed/x64-windows/share/yaml-cpp/
  ```
- If not, reinstall yaml-cpp as shown above.
- Make sure you are not mixing x86 and x64 builds.

---

**Summary:**
- Always link yaml-cpp in the target that directly includes its headers.
- Always use the correct vcpkg toolchain and triplet for your build.
- Clean build directory after changing dependencies or CMakeLists.txt.
