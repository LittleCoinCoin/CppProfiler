# ~/src/Tests/DirectInclude/ProfilerDisabled/CMakeLists.txt

add_compile_definitions(PROFILER_ENABLED=0)
set(TargetName CppProfiler_Tests_DirectBuild_ProfilerDisabled)
add_executable(${TargetName}

"../../main.cpp"

"../../../Profile/Profiler.cpp"
"../../../Profile/OSStatistics.cpp"

)

set_property(TARGET ${TargetName} PROPERTY CXX_STANDARD 20)

target_include_directories(${TargetName} PRIVATE
	"../../../../headers"
)

if(RUNTIME_PROFILER_TESTS)
	add_test(NAME ${TargetName} COMMAND ${TargetName})
endif()