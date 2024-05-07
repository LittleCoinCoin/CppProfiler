# ~/src/Tests/SharedLibraryLink/ProfilerEnabled/bin/CMakeLists.txt

#For this test, we start by 1) building the profiler library locally,
#then 2) we build the test executable and 3) link it with the profiler library

add_compile_definitions(USE_PROFILER_LIB)

#For this test, we enable the profiler
add_compile_definitions(PROFILER_ENABLED=1)

# 2) Build the test executable
set(TargetName CppProfiler_Tests_SharedLibraryLink_ProfilerEnabled)
add_executable(${TargetName} 
"../../../main.cpp"
)

set_property(TARGET ${TargetName} PROPERTY CXX_STANDARD 20)

# 3) Link the test executable with the profiler library
target_link_libraries(${TargetName} PRIVATE
	ProfilerEnabled
)

#Copy the profiler library to the output directory if on Windows
if(WIN32)
	add_custom_command(TARGET ${TargetName} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $ENV{TEST_SHAREDLIB_PROFILERENABLED_DIRECTORY}/ProfilerEnabled.dll ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>
	)
endif()