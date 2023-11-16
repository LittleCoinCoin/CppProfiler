# ~/Profiler/CMakeList.txt
add_library (Profiler SHARED

"src/Profiler.cpp"

)

set_property(TARGET Profiler PROPERTY CXX_STANDARD 20)

target_include_directories(Profiler PUBLIC

"./headers"
"../Timer/headers"

)

target_link_libraries(Profiler PUBLIC Timer)
