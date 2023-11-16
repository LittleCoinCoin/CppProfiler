# ~/Profiler/CMakeList.txt

add_compile_definitions(BUILD_PROFILER_LIB)

add_library (Profile SHARED

"src/Profiler.cpp"
"src/Timing.cpp"

)

set_property(TARGET Profile PROPERTY CXX_STANDARD 20)

target_include_directories(Profile PUBLIC

"./headers"

)
