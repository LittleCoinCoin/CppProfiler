# Usage Reference

**This article is about:** Using profiling macros, managing profiler sessions, exporting results, and capturing data programmatically. This is the complete API reference.

## Prerequisites

Before reading this article, you should:

- Have CppProfiler integrated into your project - see [Integration Guide](Integration.md)
- Understand basic profiling concepts - see [Getting Started](GettingStarted.md)

## Profiling Macros

CppProfiler provides four macros for different profiling scenarios.

### PROFILE_FUNCTION_TIME

Profile an entire function automatically:

```cpp
void MyFunction() {
    PROFILE_FUNCTION_TIME(0);  // Track index 0
    // Function code here
}
```

The macro uses `__FUNCTION__` for automatic naming and measures from invocation to scope exit.

### PROFILE_BLOCK_TIME

Profile a specific code block:

```cpp
void MyFunction() {
    // Unprofiled code
    {
        PROFILE_BLOCK_TIME(MyBlockName, 0);
        // Profiled code here
    }
    // More unprofiled code
}
```

Use this to profile specific sections within a function.

### PROFILE_FUNCTION_TIME_BANDWIDTH

Profile a function and measure data throughput:

```cpp
void ProcessFile(const char* path) {
    FILE* file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    PROFILE_FUNCTION_TIME_BANDWIDTH(0, fileSize);
    char* buffer = new char[fileSize];
    fread(buffer, 1, fileSize, file);
    delete[] buffer;
    fclose(file);
}
```

Reports bandwidth in MB/s and GB/s.

### PROFILE_BLOCK_TIME_BANDWIDTH

Profile a block with bandwidth measurement:

```cpp
{
    PROFILE_BLOCK_TIME_BANDWIDTH(DataProcessing, 0, byteCount);
    // Process data
}
```

## Profiler Lifecycle

### Initialization

Create and configure the profiler:

```cpp
Profile::Profiler profiler;
profiler.SetProfilerName("MyApp");
Profile::SetProfiler(&profiler);
profiler.SetTrackName(0, "Main");
profiler.Initialize();
```

### Ending a Session

Stop profiling and prepare results:

```cpp
profiler.End();
```

### Reporting Results

Display profiling results:

```cpp
profiler.Report();
```

## Multi-Track Profiling

Organize profiling into logical tracks:

```cpp
profiler.SetTrackName(0, "Initialization");
profiler.SetTrackName(1, "Processing");
profiler.SetTrackName(2, "Cleanup");

// Use different tracks for different phases
PROFILE_FUNCTION_TIME(0);  // Initialization track
PROFILE_FUNCTION_TIME(1);  // Processing track
PROFILE_FUNCTION_TIME(2);  // Cleanup track
```

The report shows breakdown by track with elapsed time and percentage of total.

## CSV Export

Export results to CSV for analysis:

```cpp
profiler.End();
profiler.ExportToCSV("./ProfileResults/MyApp.csv");
```

### CSV File Format

The CSV file contains one row per profiled block with columns:

- Track Name, Track Elapsed, Track Elapsed in Seconds, Track Proportion in Total
- Block Name, Block Hit Count, Block Elapsed, Block Elapsed in Seconds
- Block Proportion in Track, Block Proportion in Total
- Block Associated Page Faults Count, Block Processed Byte Count, Block Bandwidth In Bytes

**Note:** The CSV header contains a typo "Secconds" in the Profiler::ExportToCSV function (line 364 of src/Profile/Profiler.cpp), while ProfilerResults::ExportToCSV correctly spells it "Seconds" (line 489). Both refer to elapsed time in seconds.

## Result Capture

Access results programmatically:

```cpp
profiler.End();

Profile::ProfilerResults results;
results.Capture(&profiler);

for (Profile::NB_TRACKS_TYPE i = 0; i < results.trackCount; ++i) {
    Profile::ProfileTrackResult& track = results.tracks[i];
    printf("Track: %s, Elapsed: %f sec\n", track.name, track.elapsedSec);

    for (Profile::NB_TIMINGS_TYPE j = 0; j < track.blockCount; ++j) {
        Profile::ProfileBlockResult& block = track.timings[j];
        double avgMs = 1000.0 * block.elapsedSec / block.hitCount;
        printf("  %s: %.3f ms avg\n", block.blockName, avgMs);
    }
}
```

## Next Steps

Now that you understand the profiling API, you can:

- **Explore advanced features** - see [Advanced Features](AdvancedFeatures.md)
- **Review best practices** - see [Best Practices](BestPractices.md)

