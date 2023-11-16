# ~/Tests/CMakeLists.txt

add_executable(Tests 

"main.cpp"

)

set_property(TARGET Timer PROPERTY CXX_STANDARD 20)

target_link_libraries(Tests PRIVATE Timer Profiler)

target_include_directories(Tests PRIVATE
	"../Profiler/headers"
)