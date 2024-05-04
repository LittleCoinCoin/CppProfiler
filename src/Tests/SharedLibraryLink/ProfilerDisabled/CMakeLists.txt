# ~/src/Tests/SharedLibraryLink/ProfilerDisabled/CMakeLists.txt

#For this test, we start by 1) building the profiler library locally,
#then 2) we build the test executable and 3) link it with the profiler library.

add_compile_definitions(BUILD_PROFILER_LIB)

#For this test, we disable the profiler.
add_compile_definitions(PROFILER_ENABLED=0)

# 1) Build the profiler library
set(LibraryName ProfilerDisabled)
add_library (${LibraryName} SHARED
"../../../Profile/Profiler.cpp"
"../../../Profile/Timing.cpp"
)
target_include_directories(${LibraryName} PRIVATE
	"../../../../headers"
)

# 2) Build the test executable
set(TargetName CppProfiler_Tests_SharedLibraryLink_ProfilerDisabled)
add_executable(${TargetName}
"../../main.cpp"
)

set_property(TARGET ${TargetName} PROPERTY CXX_STANDARD 20)
target_include_directories(${TargetName} PRIVATE
	"../../../../headers"
)

# 3) Link the test executable with the profiler library
target_link_libraries(${TargetName} PRIVATE
	${LibraryName}
)