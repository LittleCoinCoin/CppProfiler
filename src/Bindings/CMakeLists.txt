# ~/src/Bindings/CMakeList.txt

add_compile_definitions(PROFILER_ENABLED=1)

set(targetName "PyProfile")
pybind11_add_module(${targetName} 

"../Profile/Profiler.cpp"
"../Profile/OSStatistics.cpp"

"./Python.cpp"
)

target_include_directories(${targetName} PRIVATE ../../headers)

# Add a post build command to copy the libraries *.pyd and *.lib files to the
#../Tests/Bindings folder in the build directory
add_custom_command(TARGET ${targetName} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${targetName}> ${CMAKE_CURRENT_BINARY_DIR}/../Tests/Bindings
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:${targetName}>/${targetName}.lib ${CMAKE_CURRENT_BINARY_DIR}/../Tests/Bindings
	COMMENT "Copying $<TARGET_FILE:${targetName}> to ${CMAKE_CURRENT_BINARY_DIR}/../Tests/Bindings"
)