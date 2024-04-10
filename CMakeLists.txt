# ~/CMakeList.txt

cmake_minimum_required (VERSION 3.20)

project ("CppProfiler")

# In Windows, generated dlls and .exe must be at the same place
# This makes sure that the .dll will be at the same place as the .exe 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE STRING "" )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE STRING "" )

# Same purpose as above but for the dlls added with vcpkg
set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)

# For consistency we separate the potential .lib generated
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE STRING "")

# In this project, we are setting the number of timings and tracks to a default value
# In this project, we are setting the number of timings and tracks to a default value
add_compile_definitions(NB_TIMINGS=64)
add_compile_definitions(NB_TRACKS=8) # cannot exceed 256 (u8)

#In this project, by default we want to build the tests
option(BUILD_PROFILER_TESTS "Build the test executable for the profiler" ON)
if (BUILD_PROFILER_TESTS)
	message("Building tests")
	add_subdirectory ("src/Tests")
endif()

option(ENABLE_PROFILER "The profiler will record data if this option is ON.
						Otherwise, it will just ignore the recording MACROS." ON)
if (ENABLE_PROFILER)
	message("Enabling profiler")
	add_compile_definitions(_PROFILER_ENABLED)
endif()

# We want to build the profiler as a shared library
option(BUILD_PROFILER_LIB "Build the profiler as a library" ON)
if (BUILD_PROFILER_LIB)
	message("Building profiler as a library")
	add_subdirectory ("src/Profile")
endif()