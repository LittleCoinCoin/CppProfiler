# Best Practices

**This article is about:** Proven techniques for effective profiling, organizing measurements, and avoiding common pitfalls. Apply these practices to get the most from CppProfiler.

## Use Tracks to Organize Profiling

Organize related measurements into logical tracks:

```cpp
profiler.SetTrackName(0, "Initialization");
profiler.SetTrackName(1, "MainLoop");
profiler.SetTrackName(2, "Rendering");
profiler.SetTrackName(3, "Physics");
```

This makes reports easier to understand and helps identify performance bottlenecks in specific phases.

## Profile at Appropriate Granularity

Choose the right scope for profiling.

**Too coarse** - Entire application:

```cpp
int main() {
    PROFILE_FUNCTION_TIME(0);  // Not useful
    runApplication();
}
```

**Too fine** - Individual operations:

```cpp
for (int i = 0; i < 1000000; ++i) {
    PROFILE_BLOCK_TIME(SingleIteration, 0);  // Overhead dominates
    data[i] = i;
}
```

**Good** - Meaningful operations:

```cpp
void ProcessBatch() {
    PROFILE_FUNCTION_TIME(0);
    for (int i = 0; i < batchSize; ++i) {
        data[i] = compute(i);
    }
}
```

Profile functions or code blocks that represent meaningful work, not individual operations.

## Use Bandwidth Profiling for I/O

When measuring I/O operations, include bandwidth metrics:

```cpp
void LoadTexture(const char* path) {
    size_t fileSize = getFileSize(path);
    PROFILE_FUNCTION_TIME_BANDWIDTH(0, fileSize);
    loadFile(path);  // Profiler reports MB/s
}
```

This provides context for performance analysis - is the I/O slow or is the system just limited by disk speed?

## Reset Between Profiling Sessions

Clear profiling data between independent test runs:

```cpp
// Session 1
profiler.Initialize();
runTest1();
profiler.End();
profiler.Report();

profiler.ClearTracks();  // Reset for next session

// Session 2
profiler.Initialize();
runTest2();
profiler.End();
profiler.Report();
```

This prevents data from one session affecting the next.

## Use Repetition Testing for Benchmarks

Single runs are unreliable due to system variance. Use repetition testing for statistical analysis:

```cpp
// Single run: Unreliable
profiler.Initialize();
runAlgorithm();
profiler.End();

// Better: Statistical analysis
repetitionProfiler.FixedCountRepetitionTesting(100);
// Reports average, variance, min, max
```

Repetition testing reveals performance patterns and identifies outliers.

## See Also

- [Advanced Features](AdvancedFeatures.md) - Repetition testing details
- [Usage Reference](UsageReference.md) - API reference

