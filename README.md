# CppProfiler

A lightweight, header-based C++20 profiling library with zero-cost abstraction when disabled.

## Quick Start

```cpp
#include "Profile/Profiler.hpp"

int main() {
    Profile::Profiler profiler;
    Profile::SetProfiler(&profiler);
    profiler.Initialize();

    {
        PROFILE_BLOCK_TIME("MyBlock", 0);
        // Your code here
    }

    profiler.PrintResults();
}
```

## Features

- **Zero-cost abstraction** when profiling is disabled
- **Easy integration** - Direct source inclusion or shared library linking
- **Cross-platform** - Windows, Linux (x86/ARM), macOS
- **Statistical analysis** - Average, variance, min, max metrics
- **Flexible reporting** - Console output and CSV export

## Documentation

Complete documentation is available in the [`docs/`](docs/) directory:

- **[Getting Started](docs/users/GettingStarted.md)** - Installation and first example
- **[Integration Guide](docs/users/Integration.md)** - Add to your project
- **[Usage Reference](docs/users/UsageReference.md)** - Complete API reference
- **[Advanced Features](docs/users/AdvancedFeatures.md)** - Statistical analysis and testing
- **[Best Practices](docs/users/BestPractices.md)** - Profiling techniques
- **[System Architecture](docs/devs/Architecture.md)** - For contributors
- **[Full Documentation Index](docs/index.md)** - All articles and guides

## Requirements

- C++20 compiler
- CMake 3.20+ (for building)
- No external dependencies

## License

MIT License - See LICENSE file for details
