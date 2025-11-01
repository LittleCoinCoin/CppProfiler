# Contributor Guide

**This article is about:** Public API surface, integration points, advanced features, and extension mechanisms. Read this to understand how to extend or modify CppProfiler.

## Prerequisites

Before reading this article, you should:

- Understand system architecture - see [Architecture](Architecture.md)

## Executive Summary

CppProfiler provides a macro-based instrumentation API with three primary integration points:

1. **Profiling Macros**: `PROFILE_FUNCTION_TIME`, `PROFILE_BLOCK_TIME`
2. **Bandwidth Macros**: `PROFILE_FUNCTION_TIME_BANDWIDTH`, `PROFILE_BLOCK_TIME_BANDWIDTH`
3. **Profiler Management API**: `Profiler` class for session control and reporting

## Public API Surface

### Profiling Macros

**Function Profiling:**

```cpp
PROFILE_FUNCTION_TIME(trackIdx);
PROFILE_FUNCTION_TIME_BANDWIDTH(trackIdx, byteCount);
```

**Block Profiling:**

```cpp
PROFILE_BLOCK_TIME(blockName, trackIdx);
PROFILE_BLOCK_TIME_BANDWIDTH(blockName, trackIdx, byteCount);
```

**Location:** `headers/Profile/Profiler.hpp` (lines 106-128)

### Profiler Management API

**Session Control:**

```cpp
Profile::Profiler profiler;
profiler.SetProfilerName("MyApp");
Profile::SetProfiler(&profiler);
profiler.SetTrackName(0, "Main");
profiler.Initialize();
// ... profiled code ...
profiler.End();
profiler.Report();
profiler.ExportToCSV("results.csv");
```

**Location:** `headers/Profile/Profiler.hpp` (lines 562-837)

### Result Capture API

**Programmatic Access:**

```cpp
ProfilerResults results;
results.Capture(&profiler);
results.Report();
```

**Location:** `headers/Profile/Profiler.hpp` (lines 758-837)

## Core Integration Points

### Profiler Registration

**Function:** `Profile::SetProfiler(Profiler*)`  
**Location:** `src/Profile/Profiler.cpp` (lines 8-11)

Establishes global profiler instance for macro access. Must be called before any profiling macros execute.

### Block Index Registration

**Function:** `Profiler::GetProfileBlockRecorderIndex()`
**Location:** `headers/Profile/Profiler.hpp` (line 597)

Uses FNV-1a hashing to map source location (file, line, block name) to unique block index. Enables compile-time block identification without runtime overhead. This static function is called during macro expansion to register each profiled block.

### RAII Timing

**Class:** `ProfileBlock`  
**Location:** `headers/Profile/Profiler.hpp` (lines 172-187)

Constructor starts timing, destructor stops timing. Ensures timing stops even if exceptions occur.

### Timing Capture

**Functions:** `ProfileBlockRecorder::Open()` and `Close()`  
**Location:** `headers/Profile/Profiler.hpp` (lines 274-280)

Captures CPU timer values and page fault counts at block entry/exit.

## Advanced Features

### Repetition Testing Framework

**Classes:** `RepetitionTest`, `RepetitionProfiler`  
**Location:** `headers/Profile/Profiler.hpp` (lines 844-1176)

Enables statistical performance analysis by running tests multiple times and computing average, variance, min, max metrics.

### CSV Export

**Function:** `Profiler::ExportToCSV()`  
**Location:** `src/Profile/Profiler.cpp`

Exports profiling results to CSV format for analysis in spreadsheet applications.

## Extension Points

### Custom Result Processing

Access `ProfilerResults` structure to implement custom analysis:

```cpp
ProfilerResults results;
results.Capture(&profiler);

for (auto& track : results.tracks) {
    // Custom processing
}
```

### Custom Repetition Tests

Inherit from `RepetitionTest` to create custom test wrappers:

```cpp
struct MyTest : public Profile::RepetitionTest {
    void operator()() override {
        // Test code
    }
};
```

### Multi-Track Profiling

Organize measurements into logical tracks:

```cpp
profiler.SetTrackName(0, "Initialization");
profiler.SetTrackName(1, "Processing");
profiler.SetTrackName(2, "Cleanup");
```

## Implementation Techniques

### Macro Hygiene

Profiling macros use unique variable names to avoid conflicts:

```cpp
#define PROFILE_BLOCK_TIME(blockName, trackIdx) \
    static NB_TIMINGS_TYPE profileBlockRecorder_##trackIdx = ...; \
    Profile::ProfileBlock ProfiledBlock_##trackIdx(...)
```

### FNV-1a Hashing

Compile-time string hashing maps source locations to block indices:

```cpp
u64 hash = 14695981039346656037ULL;  // FNV-1a offset basis
// Hash file name, line number, block name
```

### Inline Performance

Critical functions marked `inline` for zero overhead:

```cpp
inline void Open(u64 _byteCount) { ... }
inline u64 Close() { ... }
```

### Conditional Compilation

When `PROFILER_ENABLED=FALSE`, macros expand to empty statements.

## See Also

- [Usage Reference](../users/UsageReference.md) - Public API reference
- [Architecture](Architecture.md) - System design details

## Next Steps

- **Learn testing infrastructure** - see [Testing Guide](TestingGuide.md)
- **Understand architecture** - see [Architecture](Architecture.md)

