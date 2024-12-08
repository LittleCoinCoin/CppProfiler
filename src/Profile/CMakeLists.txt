# ~/src/Profile/CMakeList.txt

msg("Building library with name '${PROFILER_LIB_NAME}'")

add_library (${PROFILER_LIB_NAME} SHARED

"./Profiler.cpp"
"./OSStatistics.cpp"

)

target_compile_features(${PROFILER_LIB_NAME} PUBLIC cxx_std_20)
target_compile_definitions(${PROFILER_LIB_NAME} PRIVATE 

# Sets the correct export macro
# -- IMPORTANT -- This should be set to FALSE in the target that uses this profiler lib -- IMPORTANT --
BUILD_PROFILER_LIB=TRUE 

# Optional when BUILD_PROFILER_LIB is defined because it comes in an elif block, but
# I leave it for clarity.
# -- IMPORTANT -- This should be set to TRUE in the target that uses this profiler lib -- IMPORTANT --
USE_PROFILER_LIB=FALSE

# Profiler settings
# -- IMPORTANT -- These should also be set in the target that uses this profiler lib -- IMPORTANT -- 
PROFILER_ENABLED=$<BOOL:${PROFILER_ENABLED}>
PROFILER_NAME_LENGTH=${PROFILER_NAME_LENGTH}
NB_TRACKS=${NB_TRACKS}
PROFILE_TRACK_NAME_LENGTH=${PROFILE_TRACK_NAME_LENGTH}
NB_TIMINGS=${NB_TIMINGS}

)

target_include_directories(${PROFILER_LIB_NAME} PUBLIC

"../../headers"

)
