# Testing Guide

**This article is about:** CppProfiler's test framework, test organization, test scenarios, and how to add new tests. Read this to understand testing infrastructure and contribute tests.

## Prerequisites

Before reading this article, you should:

- Understand system architecture - see [Architecture](Architecture.md)
- Understand contribution workflow - see [Contributor Guide](ContributorGuide.md)

## Test Framework Overview

CppProfiler uses a **custom test framework** with no external dependencies. Tests are simple C++ functions called from `main()`.

**Framework Characteristics:**

- Single test executable per configuration
- Test functions called from `main()`
- Manual verification via console output and CSV export
- Success determined by successful execution (no crashes)

## Test Organization

### 4-Way Test Matrix

CppProfiler validates all integration modes:

1. **DirectBuild + ProfilerDisabled**: Source inclusion with profiling disabled
2. **DirectBuild + ProfilerEnabled**: Source inclusion with profiling enabled
3. **SharedLibraryLink + ProfilerDisabled**: DLL linking with profiling disabled
4. **SharedLibraryLink + ProfilerEnabled**: DLL linking with profiling enabled

**Rationale:** Ensures profiler works correctly in all build configurations.

### Test File Structure

```
src/Tests/
├── main.cpp                    # Test scenarios (283 lines)
├── DirectBuild/
│   ├── ProfilerDisabled/CMakeLists.txt
│   └── ProfilerEnabled/CMakeLists.txt
└── SharedLibraryLink/
    ├── ProfilerDisabled/CMakeLists.txt
    └── ProfilerEnabled/CMakeLists.txt
```

Same `main.cpp` used for all 4 configurations, ensuring consistent test coverage.

## Test Scenarios

### Basic Function Profiling

Tests `PROFILE_FUNCTION_TIME` macro:

```cpp
void TestFunction_ProfileFunction(Profile::u64 _arr[], Profile::u64 _count) {
    PROFILE_FUNCTION_TIME(0);
    for (Profile::u64 i = 0; i < _count; ++i) {
        _arr[i] = i;
    }
}
```

### Block Profiling

Tests `PROFILE_BLOCK_TIME` macro and RAII timing:

```cpp
void TestFunction_ProfileBlock(Profile::u64 _arr[], Profile::u64 _count) {
    for (Profile::u64 i = 0; i < _count; ++i) {
        PROFILE_BLOCK_TIME(TestFunction_ProfileBlock_Write, 0);
        _arr[i] = i;
    }
}
```

### Bandwidth Profiling

Tests bandwidth measurement:

```cpp
void TestFunction_Bandwidth(Profile::u64 _arr[], Profile::u64 _count) {
    PROFILE_FUNCTION_TIME_BANDWIDTH(0, sizeof(Profile::u64) * _count);
    for (Profile::u64 i = 0; i < _count; ++i) {
        _arr[i] = i;
    }
}
```

### Page Fault Tracking

Tests OS statistics collection:

```cpp
void TestFunction_PageFaultCounter() {
    Profile::u64 arraySize = 1024 * 1024;
    Profile::u8* arr = (Profile::u8*)malloc(sizeof(Profile::u8) * arraySize);
    PROFILE_FUNCTION_TIME_BANDWIDTH(0, sizeof(Profile::u8) * arraySize);
    for (Profile::u64 i = 0; i < arraySize; ++i) {
        arr[i] = (Profile::u8)i;
    }
    free(arr);
}
```

### Multi-Track Profiling

Tests multiple profiling tracks:

```cpp
void TestFunction_Track2(Profile::u64 _arr[], Profile::u64 _count) {
    PROFILE_FUNCTION_TIME(1);  // Use track 1
    for (Profile::u64 i = 0; i < _count; ++i) {
        _arr[i] = i;
    }
}
```

### Repetition Testing

Tests statistical analysis framework:

```cpp
void TestFunction_FixedRepetitionTesting() {
    Profile::u64* arr = (Profile::u64*)malloc(sizeof(Profile::u64) * 8192);
    Profile::u16 repetitionCount = 10;
    Profile::ProfilerResults* results = new Profile::ProfilerResults[repetitionCount];
    Profile::RepetitionProfiler* repetitionProfiler = new Profile::RepetitionProfiler();
    
    // Create and register tests
    repetitionProfiler->FixedCountRepetitionTesting(repetitionCount);
    
    delete[] results;
    delete repetitionProfiler;
    free(arr);
}
```

## Test Execution

### Build Configuration

```bash
cmake --preset Clang
cmake --build --preset "Clang Release"
```

### Running Tests Locally

```bash
ctest --preset "Clang Release"
```

### Expected Output

Tests produce console output showing profiling results and CSV export.

## CI/CD Integration

### GitHub Actions Workflow

Tests run automatically on:

- **Linux**: Clang 17/18/19, GCC 11/12/13 (Debug + Release)
- **Windows**: MSVC v143 x64/x86 (Debug + Release)

### Platform Coverage

4-way test matrix × 16 compiler/platform combinations = 64 total test runs in CI/CD.

## Adding New Tests

### Step-by-Step Guide

1. **Create test function** in `src/Tests/main.cpp`:

```cpp
void TestFunction_MyNewTest() {
    PROFILE_FUNCTION_TIME(0);
    // Test code here
}
```

2. **Call from main()**:

```cpp
int main() {
    // ... setup ...
    TestFunction_MyNewTest();
    // ... cleanup ...
}
```

3. **Rebuild and test**:

```bash
cmake --build --preset "Clang Release"
ctest --preset "Clang Release"
```

## See Also

- [Architecture](Architecture.md) - System design details
- [Contributor Guide](ContributorGuide.md) - Contribution workflow

