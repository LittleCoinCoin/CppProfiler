# ~/CMakeList.txt

cmake_minimum_required (VERSION 3.20)

project ("CppProfiler")

# In this project, we are setting the number of timings and tracks to a default value
add_compile_definitions(NB_TIMINGS=256)
add_compile_definitions(NB_TRACKS=8)
add_compile_definitions(PROFILE_TRACK_NAME_LENGTH=64)
add_compile_definitions(PROFILER_NAME_LENGTH=64)

#In this project, by default we want to build the tests
option(BUILD_PROFILER_TESTS "Tests that the profiler can build in all configurations including 
1. DirectBuild/ProfilerDisabled 2. DirectBuild/ProfilerEnabled
3. SharedLibraryLink/ProfilerDisabled 4. SharedLibraryLink/ProfilerEnabled. In cases 3 and 4,
the profiler is indeed built as a shared library and a separate executable is built to test
the link to the profiler library." ON)

option(RUNTIME_PROFILER_TESTS "Has an effect only if BUILD_PROFILER_TESTS is ON. In this case,
runs the executables using the profiler under the conditions 1. DirectBuild/ProfilerDisabled
2. DirectBuild/ProfilerEnabled 3. SharedLibraryLink/ProfilerDisabled
4. SharedLibraryLink/ProfilerEnabled. " ON)

if (BUILD_PROFILER_TESTS)
	message("Building tests")
	if (RUNTIME_PROFILER_TESTS)
		message("Adding runtime tests.
		Don't forget to launch them after building using `ctest` command.
		As this is using a multi-config generator, specifying the config with `-C <CONFIG>` is mandatory.")
		enable_testing()
	endif()
	add_subdirectory("src/Tests/DirectBuild/ProfilerDisabled")
	add_subdirectory("src/Tests/DirectBuild/ProfilerEnabled")
	add_subdirectory("src/Tests/SharedLibraryLink/ProfilerDisabled")
	add_subdirectory("src/Tests/SharedLibraryLink/ProfilerEnabled")
else()
	message("Not building tests")
	set(RUNTIME_PROFILER_TESTS OFF)
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