# ~/src/Tests/SharedLibraryLink/ProfilerDisabled/CMakeLists.txt

msg("Configure Test: SharedLibraryLink - ProfilerDisabled")

# Build the profiling library first
set(TargetLibName ${PROFILER_LIB_NAME}_TestDisabled)

add_library (${TargetLibName} SHARED
"../../../Profile/Profiler.cpp"
"../../../Profile/OSStatistics.cpp"
)

target_compile_features(${TargetLibName} PUBLIC cxx_std_20)
target_compile_definitions(${TargetLibName} PRIVATE 

# Sets the correct export macro
BUILD_PROFILER_LIB
#Unsetting is optional when BUILD_PROFILER_LIB is defined because it comes after in an elif block
#USE_PROFILER_LIB

# Usual profiler configuration options.
PROFILER_ENABLED=FALSE
PROFILER_NAME_LENGTH=${PROFILER_NAME_LENGTH}
NB_TRACKS=${NB_TRACKS}
PROFILE_TRACK_NAME_LENGTH=${PROFILE_TRACK_NAME_LENGTH}
NB_TIMINGS=${NB_TIMINGS}
PROFILE_BLOCK_NAME_LENGTH=${PROFILE_BLOCK_NAME_LENGTH}

)

target_include_directories(${TargetLibName} PUBLIC
	"../../../../headers"
)

# Build the test executable
set(TargetTestName CppProfiler_Tests_SharedLibraryLink_ProfilerDisabled)
add_executable(${TargetTestName}

"../../main.cpp"

)

target_compile_features(${TargetTestName} PUBLIC cxx_std_20)
target_compile_definitions(${TargetTestName} PRIVATE 

# Sets the correct export macro
#BUILD_PROFILER_LIB
USE_PROFILER_LIB

# The following compile definitions must be the same as the library to ensure
# the executable that uses it is expecting the same configuration as the one
# used when building the library.
PROFILER_ENABLED=FALSE
PROFILER_NAME_LENGTH=${PROFILER_NAME_LENGTH}
NB_TRACKS=${NB_TRACKS}
PROFILE_TRACK_NAME_LENGTH=${PROFILE_TRACK_NAME_LENGTH}
NB_TIMINGS=${NB_TIMINGS}

)

target_include_directories(${TargetTestName} PUBLIC
	"../../../../headers" # headers of the library
)

# Add the build dependency to make sure the library is built first
add_dependencies(${TargetTestName} ${TargetLibName})

# Link the library to the test executable
target_link_libraries(${TargetTestName} ${TargetLibName})

# Run the test
if(RUNTIME_PROFILER_TESTS)
	add_test(NAME ${TargetTestName} COMMAND ${TargetTestName})
endif()