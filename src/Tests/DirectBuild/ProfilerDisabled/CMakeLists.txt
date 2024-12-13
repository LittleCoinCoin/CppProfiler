# ~/src/Tests/DirectBuild/ProfilerDisabled/CMakeLists.txt

msg("Configure Test: DirectBuild - ProfilerDisabled")

set(TargetName CppProfiler_Tests_DirectBuild_ProfilerDisabled)
add_executable(${TargetName}

"../../main.cpp"

"../../../Profile/Profiler.cpp"
"../../../Profile/OSStatistics.cpp"

)

target_compile_features(${TargetName} PUBLIC cxx_std_20)
target_compile_definitions(${TargetName} PRIVATE 

PROFILER_ENABLED=FALSE
PROFILER_NAME_LENGTH=${PROFILER_NAME_LENGTH}
NB_TRACKS=${NB_TRACKS}
PROFILE_TRACK_NAME_LENGTH=${PROFILE_TRACK_NAME_LENGTH}
NB_TIMINGS=${NB_TIMINGS}
PROFILE_BLOCK_NAME_LENGTH=${PROFILE_BLOCK_NAME_LENGTH}

)

target_include_directories(${TargetName} PRIVATE
	"../../../../headers"
)

if(RUNTIME_PROFILER_TESTS)
	add_test(NAME ${TargetName} COMMAND ${TargetName})
endif()