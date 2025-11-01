# Glossary

## Profiling Concepts

**Profiling**
The process of measuring program performance characteristics such as execution time, memory usage, and resource utilization.

**Profiler**
A tool or library that collects performance measurements during program execution.

**Profile Block**
A named section of code whose execution time and resource usage are measured.

**Hit Count**
The number of times a profile block was executed during a profiling session.

**Elapsed Time**
The total CPU cycles or wall-clock time consumed by a profile block across all executions.

**Bandwidth**
The rate of data processing, measured in MB/s or GB/s. Calculated as bytes processed divided by elapsed time.

## CppProfiler Terminology

**Track**
A logical grouping of related profile blocks. CppProfiler supports multiple independent tracks for organizing measurements by subsystem or phase.

**Profiler Session**
A period of time during which profiling measurements are collected. Sessions are started with `Initialize()` and ended with `End()`.

**Profile Block Recorder**
Internal data structure storing accumulated statistics for a single profile block (hit count, elapsed time, page faults, bytes processed).

**Profiler Results**
Snapshot of profiling data captured at a point in time. Can be exported to CSV or printed to console.

**Repetition Testing**
Statistical performance analysis technique that runs tests multiple times and computes average, variance, min, and max metrics.

## Technical Terms

**RAII** (Resource Acquisition Is Initialization)
C++ programming pattern where resource management is tied to object lifetime. ProfileBlock uses RAII to ensure timing stops automatically when the block exits.

**RDTSC** (Read Time-Stamp Counter)
x86/x64 CPU instruction that reads the processor's time-stamp counter for high-precision timing.

**Zero-Cost Abstraction**
Programming technique where abstractions (like profiling macros) have no runtime overhead when disabled. CppProfiler achieves this by expanding disabled macros to empty statements.

**Page Fault**
CPU exception that occurs when a program accesses memory that is not currently in physical RAM. CppProfiler tracks page faults to analyze memory access patterns.

## Build and Integration

**Direct Build**
Integration mode where profiler source code is compiled directly into the application executable.

**Shared Library**
Integration mode where profiler is built as a separate DLL (Windows) or SO (Linux) and linked to the application.

**CMake Preset**
Named build configuration specifying compiler, compiler version, and build options. CppProfiler provides presets for Clang, GCC, and MSVC.

**Compile-Time Configuration**
Build-time settings specified via CMake variables that control profiler behavior (e.g., number of tracks, buffer sizes).

**PROFILER_ENABLED**
CMake variable controlling whether profiling code is compiled. When FALSE, profiling macros expand to empty statements.

## Measurement Units

**CPU Cycles**
Raw processor clock cycles. Converted to seconds using CPU frequency estimation.

**Seconds (sec)**
Wall-clock time in seconds, calculated from CPU cycles and estimated CPU frequency.

**Megabytes (MB)**
1,048,576 bytes (2^20). CppProfiler uses binary units for bandwidth calculations. Also referred to as MiB (Mebibyte).

**Gigabytes (GB)**
1,073,741,824 bytes (2^30). CppProfiler uses binary units for bandwidth calculations. Also referred to as GiB (Gibibyte).

## See Also

- [Getting Started](../users/GettingStarted.md) - Introduction to CppProfiler
- [Usage Reference](../users/UsageReference.md) - API reference

