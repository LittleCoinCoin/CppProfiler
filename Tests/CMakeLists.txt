# ~/Tests/CMakeLists.txt

add_compile_definitions(PROFILER_ENABLED) # The Profiler must be enabled for the tests

add_executable(CppProfilerTests 

"main.cpp"

"../Profile/src/Profiler.cpp"
"../Profile/src/Timing.cpp"

)

set_property(TARGET CppProfilerTests PROPERTY CXX_STANDARD 20)

target_include_directories(CppProfilerTests PRIVATE
	"../Profile/headers"
)