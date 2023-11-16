# ~/Timer/CmakeLists.txt

add_library(Timer SHARED

"src/Timing.cpp"

)

set_property(TARGET Timer PROPERTY CXX_STANDARD 20)

target_include_directories(Timer PUBLIC "./headers")