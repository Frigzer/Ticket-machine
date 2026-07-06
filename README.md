## Build with Conan

Requirements:

- Conan 2
- CMake >= 3.25
- Ninja
- C++23 compiler

First-time Conan setup:

```powershell
conan profile detect --force
```

### Configure dependencies:

```powershell
conan install . -s build_type=Debug -s compiler.cppstd=23 -c tools.cmake.cmaketoolchain:generator=Ninja --build=missing
```

### Build

```powershell
cmake --preset conan-debug
cmake --build --preset conan-debug
```