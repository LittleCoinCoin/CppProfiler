# ~/Tests/CMakeLists.txt

add_executable(Tests 

"main.cpp"

)

set_property(TARGET Tests PROPERTY CXX_STANDARD 20)

target_link_libraries(Tests PRIVATE Profile)

target_include_directories(Tests PRIVATE
	"../Profile/headers"
)