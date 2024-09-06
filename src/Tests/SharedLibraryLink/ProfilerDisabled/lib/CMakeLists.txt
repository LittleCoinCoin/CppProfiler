# ~/src/Tests/SharedLibraryLink/ProfilerDisabled/lib/CMakeLists.txt

#For this test, we start by 1) building the profiler library locally,
#then 2) we build the test executable and 3) link it with the profiler library.

add_compile_definitions(BUILD_PROFILER_LIB)

#For this test, we disable the profiler.
add_compile_definitions(PROFILER_ENABLED=0)

# 1) Build the profiler library
set(LibraryName ProfilerDisabled)
add_library (${LibraryName} SHARED
"../../../../Profile/Profiler.cpp"
"../../../../Profile/OSStatistics.cpp"
)

set_property(TARGET ${LibraryName} PROPERTY CXX_STANDARD 20)

target_include_directories(${LibraryName} PUBLIC
	"../../../../../headers"
)

set(ENV{TEST_SHAREDLIB_PROFILERDISABLED_DIRECTORY} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)