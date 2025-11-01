# Advanced Features

**This article is about:** Statistical performance analysis using repetition testing, best performance search, and advanced profiling techniques. Read this after mastering basic usage.

## Prerequisites

Before reading this article, you should:

- Understand basic profiling - see [Usage Reference](UsageReference.md)

## Repetition Testing Framework

Repetition testing provides statistical performance analysis by running code multiple times and computing average, variance, minimum, and maximum metrics.

### Fixed-Count Repetition Testing

Run a test a fixed number of times:

```cpp
struct MyTest : public Profile::RepetitionTest {
    Profile::u64* data;
    Profile::u64 count;

    MyTest(const char* name, Profile::u64* d, Profile::u64 c)
        : RepetitionTest(name), data(d), count(c) {}

    void operator()() override {
        ProcessData(data, count);
    }
};

int main() {
    Profile::u64* data = new Profile::u64[1000000];
    Profile::u16 repetitionCount = 100;
    Profile::ProfilerResults* results = new Profile::ProfilerResults[repetitionCount];
    Profile::RepetitionProfiler repetitionProfiler;

    MyTest test("ProcessData Test", data, 1000000);
    repetitionProfiler.PushBackRepetitionTest(&test);
    repetitionProfiler.SetRepetitionResults(results);
    repetitionProfiler.FixedCountRepetitionTesting(repetitionCount);

    delete[] data;
    delete[] results;
}
```

The output shows average, variance, minimum, and maximum results across all repetitions.

### Best Performance Search

Find the best performance by running tests adaptively:

```cpp
Profile::ProfilerResults* results = new Profile::ProfilerResults[2];
Profile::RepetitionProfiler repetitionProfiler;

Profile::u64* data = new Profile::u64[1000000];
MyTest test("ProcessData Test", data, 1000000);

repetitionProfiler.PushBackRepetitionTest(&test);
repetitionProfiler.SetRepetitionResults(results);

// Run for 10 seconds total, wait 3 seconds per test for improvement
repetitionProfiler.BestPerfSearchRepetitionTesting(3, false, true, 10);

delete[] data;
delete[] results;
```

**Parameters:**

- `testTimeout` (3): Seconds to wait for improvement per test
- `reset` (false): Don't reset profiler between runs
- `clear` (true): Clear profiler before starting
- `globalTimeout` (10): Total seconds to run

**Behavior:**

- Runs test repeatedly
- Tracks best performance
- Resets timeout when improvement found
- Stops after global timeout or no improvement for test timeout

### Use Cases

Use repetition testing when:

- Benchmarking algorithm performance
- Comparing optimization approaches
- Analyzing performance variance
- Finding best-case and worst-case scenarios

## See Also

- [Best Practices](BestPractices.md) - When to use repetition testing
- [Usage Reference](UsageReference.md) - Basic profiling API

