# ~/CMakeList.txt

cmake_minimum_required (VERSION 3.20)

project ("CppProfiler")

# -- Cmake utility
set(CMAKE_CPPPROFILER_MSG_PREFIX "[CppProfiler] ")
function(msg _msg)
	message("${CMAKE_CPPPROFILER_MSG_PREFIX}${_msg}")
endfunction()

# -- Cmake options
# Options to control the build of the profiler
option(PROFILER_ENABLED "The profiling source code will be set up to be compiled in the project" ON)

# Options to control values of macros int he profiler with the same name
set(PROFILER_NAME_LENGTH 64 CACHE STRING "Maximal length of a profiler name")
set(NB_TRACKS 8 CACHE STRING "Maximal number of profiling tracks a profiler can hold")
set(PROFILE_TRACK_NAME_LENGTH 64 CACHE STRING "Maximal length of a profiling track name")
set(NB_TIMINGS 256 CACHE STRING "Maximal number of profiling blocks a profiler track can hold")

# Options to control the build of the tests
option(BUILD_PROFILER_TESTS "Tests that the profiler can build in all configurations including 
1. DirectBuild/ProfilerDisabled 2. DirectBuild/ProfilerEnabled
3. SharedLibraryLink/ProfilerDisabled 4. SharedLibraryLink/ProfilerEnabled. In cases 3 and 4,
the profiler is indeed built as a shared library and a separate executable is built to test
the link to the profiler library." ON)
option(RUNTIME_PROFILER_TESTS "Has an effect only if BUILD_PROFILER_TESTS is ON. In this case,
runs the executables using the profiler under the conditions 1. DirectBuild/ProfilerDisabled
2. DirectBuild/ProfilerEnabled 3. SharedLibraryLink/ProfilerDisabled
4. SharedLibraryLink/ProfilerEnabled. " ON)

# Options to control the build of the profiler as a library
option(BUILD_PROFILER_LIB "Build the profiler as a library" ON)
set(PROFILER_LIB_NAME "Profiler" CACHE STRING "Name of the profiler library")

# -- Cmake configuration
if (BUILD_PROFILER_TESTS)
	msg("Building tests")
	if (RUNTIME_PROFILER_TESTS)
		msg("Adding runtime tests.
		Don't forget to launch them after building using `ctest` command.
		As this is using a multi-config generator, specifying the config with `-C <CONFIG>` is mandatory.")
		enable_testing()
	endif()
	add_subdirectory("src/Tests/DirectBuild/ProfilerDisabled")
	add_subdirectory("src/Tests/DirectBuild/ProfilerEnabled")
	add_subdirectory("src/Tests/SharedLibraryLink/ProfilerDisabled")
	add_subdirectory("src/Tests/SharedLibraryLink/ProfilerEnabled")
else()
	msg("Not building tests")
	set(RUNTIME_PROFILER_TESTS OFF)
endif()

if (BUILD_PROFILER_LIB)
	add_subdirectory ("src/Profile")
endif()