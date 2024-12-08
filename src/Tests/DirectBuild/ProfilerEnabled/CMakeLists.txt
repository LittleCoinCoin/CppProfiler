# ~/src/Tests/DirectBuild/ProfilerEnabled/CMakeLists.txt

msg("Configure Test: DirectBuild - ProfilerEnabled")

set(TargetName CppProfiler_Tests_DirectBuild_ProfilerEnabled)
add_executable(${TargetName} 

"../../main.cpp"

"../../../Profile/Profiler.cpp"
"../../../Profile/OSStatistics.cpp"

)

target_compile_features(${TargetName} PUBLIC cxx_std_20)
target_compile_definitions(${TargetName} PRIVATE 

PROFILER_ENABLED=TRUE
PROFILER_NAME_LENGTH=${PROFILER_NAME_LENGTH}
NB_TRACKS=${NB_TRACKS}
PROFILE_TRACK_NAME_LENGTH=${PROFILE_TRACK_NAME_LENGTH}
NB_TIMINGS=${NB_TIMINGS}

)

target_include_directories(${TargetName} PRIVATE
	"../../../../headers"
)

if(RUNTIME_PROFILER_TESTS)
	add_test(NAME ${TargetName} COMMAND ${TargetName})
endif()