# Getting Started with CppProfiler

**This article is about:** Installing CppProfiler, running your first profiling example, and understanding the output. Start here if you're new to CppProfiler.

## Introduction

CppProfiler is a lightweight, header-based C++ profiling library designed for minimal runtime overhead and flexible integration. It provides:

- **Zero-cost abstraction**: Profiling code compiles away completely when disabled
- **RAII-based timing**: Automatic measurement via scope-based objects
- **Cross-platform support**: Windows, Linux (x86/ARM), and macOS
- **Flexible deployment**: Source inclusion or shared library linking
- **Statistical analysis**: Repetition testing with average, variance, min, max

### Use Cases

CppProfiler is ideal for:

- Performance bottleneck identification
- Algorithm comparison and optimization
- I/O bandwidth measurement
- Memory access pattern analysis (via page fault tracking)

## Prerequisites

### System Requirements

CppProfiler requires:

- **Operating Systems**: Windows (x86, x64), Linux (x86, x64, ARM), or macOS (ARM)
- **Compilers**: Clang ≥17, GCC ≥11, or MSVC v143 (Visual Studio 2022)
- **C++ Standard**: C++20 or later
- **Build Tools**: CMake ≥3.20

### Dependencies

CppProfiler has **no external library dependencies**. Everything you need is included.

## Quick Start

### Minimal Example

Here's a complete working example that profiles a function:

```cpp
#include <Profile/Profiler.hpp>

void ProcessData(Profile::u64* data, Profile::u64 count) {
    PROFILE_FUNCTION_TIME(0);  // Profile on track 0
    for (Profile::u64 i = 0; i < count; ++i) {
        data[i] = i * i;
    }
}

int main() {
    Profile::Profiler profiler;
    profiler.SetProfilerName("MyApp");
    Profile::SetProfiler(&profiler);
    profiler.SetTrackName(0, "Main");
    profiler.Initialize();

    Profile::u64* data = new Profile::u64[1000000];
    ProcessData(data, 1000000);

    profiler.End();
    profiler.Report();
    delete[] data;
    return 0;
}
```

### Expected Output

When you run this example, you'll see output like:

```
---- Estimated CPU Frequency: 3600000000 ----
---- Profiler Report: MyApp ----
Track: Main
  Elapsed: 0.0023 sec (100.0% of total)

  Block Name      Hit Count    Time (ms)    % Track    % Total
  ProcessData     1            2.30         100.0      100.0
```

### Understanding the Results

The profiler report shows:

- **Block Name**: The function or code block being profiled
- **Hit Count**: How many times the block executed
- **Time (ms)**: Total elapsed time in milliseconds
- **% Track**: Percentage of time within this track
- **% Total**: Percentage of total profiling session time

## Next Steps

Now that you understand CppProfiler basics, you can:

- **Integrate CppProfiler into your project** - see [Integration Guide](Integration.md)
- **Learn about all profiling macros** - see [Usage Reference](UsageReference.md)
- **Explore advanced features** - see [Advanced Features](AdvancedFeatures.md)

