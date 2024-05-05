# ~/src/Tests/SharedLibraryLink/ProfilerDisabled/bin/CMakeLists.txt

#For this test, we start by 1) building the profiler library locally,
#then 2) we build the test executable and 3) link it with the profiler library.

add_compile_definitions(USE_PROFILER_LIB)

#For this test, we disable the profiler.
add_compile_definitions(PROFILER_ENABLED=0)

# 2) Build the test executable
set(TargetName CppProfiler_Tests_SharedLibraryLink_ProfilerDisabled)
add_executable(${TargetName}
"../../../main.cpp"
)

set_property(TARGET ${TargetName} PROPERTY CXX_STANDARD 20)

# 3) Link the test executable with the profiler library
target_link_libraries(${TargetName} PRIVATE
	ProfilerDisabled
)

add_custom_command(TARGET ${TargetName} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $ENV{TEST_SHAREDLIB_PROFILERDISABLED_DIRECTORY}/ProfilerDisabled.dll ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>
)