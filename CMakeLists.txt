# ~/CMakeList.txt

cmake_minimum_required (VERSION 3.20)

project ("CppProfiler")

# In Windows, generated dlls and .exe must be at the same place
# This makes sure that the libsbml.dll will be at the same place
# as the tests.exe 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE STRING "" )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE STRING "" )

# Same purpose as above but for the dlls added with vcpkg
set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)

# For consistency we separate the potential .lib generated
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE STRING "")

add_compile_definitions(PROFILER_ENABLED PROFILE_LIB_SHARED)
add_subdirectory ("Profile")

add_subdirectory ("Tests")
