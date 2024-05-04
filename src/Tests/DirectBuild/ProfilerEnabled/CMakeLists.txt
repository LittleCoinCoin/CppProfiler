# ~/src/Tests/DirectInclude/ProfilerEnabled/CMakeLists.txt

add_compile_definitions(PROFILER_ENABLED=1)
set(TargetName CppProfiler_Tests_DirectBuild_ProfilerEnabled)
add_executable(${TargetName} 

"../../main.cpp"

"../../../Profile/Profiler.cpp"
"../../../Profile/Timing.cpp"

)

set_property(TARGET ${TargetName} PROPERTY CXX_STANDARD 20)

target_include_directories(${TargetName} PRIVATE
	"../../../../headers"
)