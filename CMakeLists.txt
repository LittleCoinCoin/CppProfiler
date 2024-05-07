# ~/CMakeList.txt

cmake_minimum_required (VERSION 3.20)

project ("CppProfiler")

# In this project, we are setting the number of timings and tracks to a default value
add_compile_definitions(NB_TIMINGS=256)
add_compile_definitions(NB_TRACKS=8)

#In this project, by default we want to build the tests
option(BUILD_PROFILER_TESTS "Build all tests executable for the profiler including 
1. DirectBuild/ProfilerDisabled 2. DirectBuild/ProfilerEnabled
3. SharedLibraryLink/ProfilerDisabled 4. SharedLibraryLink/ProfilerEnabled" ON)

if (BUILD_PROFILER_TESTS)
	message("Building tests")
	add_subdirectory("src/Tests/DirectBuild/ProfilerDisabled")
	add_subdirectory("src/Tests/DirectBuild/ProfilerEnabled")
	add_subdirectory("src/Tests/SharedLibraryLink/ProfilerDisabled")
	add_subdirectory("src/Tests/SharedLibraryLink/ProfilerEnabled")
endif()

option(ENABLE_PROFILER "The profiler will record data if this option is ON.
						Otherwise, it will just ignore the recording MACROS." ON)
if (ENABLE_PROFILER)
	message("Enabling profiler")
	add_compile_definitions(PROFILER_ENABLED=1)
endif()

# We want to build the profiler as a shared library
option(BUILD_PROFILER_LIB "Build the profiler as a library" ON)
if (BUILD_PROFILER_LIB)
	message("Building profiler as a library")
	add_subdirectory ("src/Profile")
endif()