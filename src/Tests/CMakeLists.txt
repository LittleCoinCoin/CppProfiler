# ~/src/Tests/CMakeLists.txt

add_compile_definitions(PROFILER_ENABLED) # The Profiler must be enabled for the tests

add_executable(CppProfilerTests 

"main.cpp"

"../Profile/Profiler.cpp"
"../Profile/Timing.cpp"

)

set_property(TARGET CppProfilerTests PROPERTY CXX_STANDARD 20)

target_include_directories(CppProfilerTests PRIVATE
	"../../headers"
)