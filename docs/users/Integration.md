# Integration Guide

**This article is about:** Adding CppProfiler to your project, configuring build settings, and choosing the right integration method. Read this after Getting Started.

## Prerequisites

Before reading this article, you should:

- Understand basic profiling concepts - see [Getting Started](GettingStarted.md)

## Integration Methods

CppProfiler offers three integration approaches. Choose based on your project structure.

### Option 1: Direct Source Inclusion

**Best for:** Single projects or when you want full control

Copy CppProfiler files to your project:

```
your_project/
├── external/CppProfiler/
│   ├── headers/Profile/
│   │   ├── Profiler.hpp
│   │   ├── Export.hpp
│   │   ├── OSStatistics.hpp
│   │   └── Types.hpp
│   └── src/Profile/
│       ├── Profiler.cpp
│       └── OSStatistics.cpp
```

Add to your CMakeLists.txt:

```cmake
add_executable(YourApp
    main.cpp
    external/CppProfiler/src/Profile/Profiler.cpp
    external/CppProfiler/src/Profile/OSStatistics.cpp
)

target_include_directories(YourApp PRIVATE
    external/CppProfiler/headers
)

target_compile_features(YourApp PRIVATE cxx_std_20)
```

### Option 2: Shared Library

**Best for:** Multiple projects sharing the same profiler

Build CppProfiler as a library:

```bash
git clone https://github.com/LittleCoinCoin/CppProfiler.git
cd CppProfiler
cmake --preset Clang
cmake --build --preset "Clang Release"
cmake --install out/build/Clang --config Release --prefix /usr/local
```

Link in your project:

```cmake
add_executable(YourApp main.cpp)

find_library(PROFILER_LIB CppProfiler PATHS /usr/local/lib)
target_link_libraries(YourApp ${PROFILER_LIB})

target_include_directories(YourApp PRIVATE /usr/local/include)
target_compile_features(YourApp PRIVATE cxx_std_20)
```

### Option 3: Git Submodule

**Best for:** Version-controlled integration

```bash
git submodule add https://github.com/LittleCoinCoin/CppProfiler.git external/CppProfiler
git submodule update --init --recursive
```

In your CMakeLists.txt:

```cmake
add_subdirectory(external/CppProfiler)
target_link_libraries(YourApp CppProfiler)
```

## CMake Configuration

### Basic Setup

Configure profiling behavior with compile definitions:

```cmake
target_compile_definitions(YourApp PRIVATE
    PROFILER_ENABLED=TRUE
    NB_TRACKS=8
    NB_TIMINGS=256
)
```

### Configuration Options

| Option | CMake Default | Header Fallback | Purpose |
|--------|---------------|-----------------|---------|
| `PROFILER_ENABLED` | ON | ON | Enable/disable profiling |
| `NB_TRACKS` | 8 | 2 | Maximum profiling tracks |
| `NB_TIMINGS` | 256 | 256 | Max blocks per track |
| `PROFILER_NAME_LENGTH` | 64 | 32 | Profiler name buffer size |
| `PROFILE_TRACK_NAME_LENGTH` | 64 | 32 | Track name buffer size |
| `PROFILE_BLOCK_NAME_LENGTH` | 64 | 32 | Block name buffer size |

**Important:** The "CMake Default" column shows values from this repository's build configuration. If you directly include CppProfiler sources without CMake, the "Header Fallback" values apply. To use different values in direct inclusion, define the macros before including the header:

```cpp
#define NB_TRACKS 4
#define NB_TIMINGS 512
#define PROFILER_NAME_LENGTH 128
#include "Profile/Profiler.hpp"
```

Or use compiler flags:

```bash
g++ -DNBTRACK=4 -DNB_TIMINGS=512 myapp.cpp
```

### Disabling the Profiler

For release builds with zero overhead:

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(YourApp PRIVATE PROFILER_ENABLED=FALSE)
else()
    target_compile_definitions(YourApp PRIVATE PROFILER_ENABLED=TRUE)
endif()
```

When disabled, all profiling macros expand to empty statements.

### Memory Footprint

Default configuration uses approximately **229 KB per Profiler instance**:

```
≈ 64 + 8 × (64 + 256 × (64 + 48)) bytes
```

To reduce memory usage:

- Decrease `NB_TIMINGS` if profiling fewer blocks
- Decrease `NB_TRACKS` if using fewer tracks
- Reduce name buffer sizes if shorter names suffice

## See Also

- [Architecture Overview](../devs/Architecture.md) - System design details
- [Contributor Guide](../devs/ContributorGuide.md) - Integration point details

## Next Steps

Now that you've integrated CppProfiler, you can:

- **Learn profiling macros** - see [Usage Reference](UsageReference.md)
- **Explore advanced features** - see [Advanced Features](AdvancedFeatures.md)
- **Review best practices** - see [Best Practices](BestPractices.md)

