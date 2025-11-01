# System Architecture

**This article is about:** CppProfiler's internal design, core components, build system, and cross-platform abstractions. Read this to understand how the profiler works internally.

## Executive Summary

CppProfiler is a lightweight C++20 profiling library emphasizing zero-cost abstraction, compile-time configuration, and cross-platform support. The architecture enables profiling code to compile away completely when disabled, resulting in zero runtime overhead.

**Key Characteristics:**

- **Zero-cost abstraction**: Profiling macros expand to empty statements when `PROFILER_ENABLED=FALSE`
- **Compile-time configuration**: Buffer sizes and limits configurable via CMake
- **Cross-platform support**: Windows, Linux (x86/ARM), and macOS compatibility
- **RAII-based profiling**: Automatic timing via scope-based `ProfileBlock` objects
- **Flexible deployment**: Supports both direct source inclusion and shared library linking

## System Architecture Overview

### High-Level Architecture

The profiler follows a layered architecture:

1. **User Application**: Invokes profiling macros
2. **Profile Namespace**: Core components (Profiler, ProfileTrack, ProfileBlock)
3. **OS Abstraction Layer**: Timer and Surveyor abstractions
4. **Platform-Specific APIs**: Windows (RDTSC, QueryPerformanceCounter), Linux (RDTSC, clock_gettime), ARM (clock_gettime)

### Component Interaction

The data flow is:

1. **Instrumentation**: User code invokes profiling macros
2. **Expansion**: Macros expand to create `ProfileBlock` RAII objects
3. **Registration**: Static initialization registers blocks with `Profiler`
4. **Timing**: `ProfileBlock` constructor/destructor capture timing data
5. **Aggregation**: `ProfileTrack` accumulates statistics per track
6. **Reporting**: `ProfilerResults` captures and formats output

## Directory Structure

```
CppProfiler/
├── headers/Profile/
│   ├── Profiler.hpp        # Core API (1177 lines)
│   ├── Export.hpp          # DLL export macros
│   ├── OSStatistics.hpp    # OS abstraction
│   └── Types.hpp           # Type aliases
├── src/Profile/
│   ├── Profiler.cpp        # Implementation
│   └── OSStatistics.cpp    # Platform-specific code
├── src/Tests/              # 4-way test matrix
├── doc/Doxyfile            # Documentation config
└── CMakeLists.txt          # Build configuration
```

## Core Components

### Profiler (Coordinator)

Central coordinator managing multiple profiling tracks.

**Location:** `headers/Profile/Profiler.hpp` (lines 562-837)

**Key API:**

```cpp
struct Profiler {
    void Initialize() noexcept;
    void End() noexcept;
    void Report() noexcept;
    void ExportToCSV(const char* path) noexcept;
    void SetTrackName(NB_TRACKS_TYPE idx, const char* name) noexcept;
    void ClearTracks() noexcept;
};
```

**Design:** Singleton-like access via global pointer, fixed-size arrays for zero-allocation runtime.

### ProfileTrack (Container)

Groups related profiling blocks into logical tracks.

**Location:** `headers/Profile/Profiler.hpp` (lines 397-471)

**Purpose:** Organizes profiling data by logical phase or subsystem.

### ProfileBlockRecorder (Statistics)

Stores accumulated statistics for a single profiled code block.

**Location:** `headers/Profile/Profiler.hpp` (lines 192-287)

**Tracks:** Hit count, elapsed time, page faults, processed bytes.

### ProfileBlock (RAII Timer)

RAII wrapper that automatically times code blocks.

**Location:** `headers/Profile/Profiler.hpp` (lines 172-187)

**Design Pattern:** Constructor starts timing, destructor stops timing. Ensures timing stops even if exceptions occur.

## Build System Architecture

### CMake Configuration

**Root CMakeLists.txt** configures:

- `PROFILER_ENABLED`: Enable/disable profiling (default: ON)
- `NB_TRACKS`: Maximum tracks (default: 8)
- `NB_TIMINGS`: Maximum blocks per track (default: 256)
- Name buffer sizes for profiler, tracks, and blocks

### Compiler Presets

**CMakePresets.json** supports:

- **Clang**: Versions 17, 18, 19
- **GCC**: Versions 11, 12, 13
- **MSVC**: v143 toolset, x64/x86

### Integration Modes

**Direct Build:** Compile profiler sources directly into application

**Shared Library:** Build profiler as DLL/SO, link to application

## Cross-Platform Abstraction

### Timer Abstraction

Provides CPU timing across platforms:

- **x86/x64**: RDTSC (Read Time-Stamp Counter)
- **ARM**: clock_gettime with CLOCK_MONOTONIC

### Surveyor Abstraction

Provides OS statistics:

- **Page Faults**: Memory access pattern analysis
- **Page Size**: Memory statistics calculation

## Design Patterns and Techniques

### RAII-Based Timing

ProfileBlock uses RAII to ensure timing stops automatically:

```cpp
{
    PROFILE_BLOCK_TIME(MyBlock, 0);
    // Timing starts in constructor
    // ... code ...
    // Timing stops in destructor
}
```

### Zero-Cost Abstraction

When `PROFILER_ENABLED=FALSE`, macros expand to empty statements:

```cpp
#define PROFILE_FUNCTION_TIME(trackIdx) // Empty when disabled
```

### Compile-Time Configuration

Buffer sizes configured via CMake, not runtime:

```cmake
set(NB_TRACKS 8)
set(NB_TIMINGS 256)
```

## Configuration System

All configuration is compile-time via CMake variables:

- `PROFILER_ENABLED`: Enable/disable profiling
- `NB_TRACKS`: Number of profiling tracks
- `NB_TIMINGS`: Blocks per track
- Name buffer sizes

## Dependencies

**Zero external dependencies.** CppProfiler uses only C++20 standard library features.

## See Also

- [Integration Guide](../users/Integration.md) - User integration guide
- [Contributor Guide](ContributorGuide.md) - Contribution workflow

## Next Steps

- **Understand integration points** - see [Contributor Guide](ContributorGuide.md)
- **Learn testing infrastructure** - see [Testing Guide](TestingGuide.md)

