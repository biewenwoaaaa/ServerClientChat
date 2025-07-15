

# Add source to this project's executable.
file(GLOB_RECURSE server_sources "*.cpp")

add_executable(server 
${server_sources} 
${PROJECT_SOURCE_DIR}/src/sayHi.cpp
)
set_target_properties(server PROPERTIES DEBUG_POSTFIX "_d")
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

target_include_directories(server PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(server PRIVATE /usr/include/mysql++)
target_include_directories(server PRIVATE /usr/include/mysql)


target_include_directories(server PRIVATE /home/g/muduo/)

target_link_directories(server PRIVATE /home/g/build/release-cpp11/lib)

target_link_libraries(server PRIVATE mysqlpp)
target_link_libraries(server PRIVATE muduo_base muduo_net)
target_link_libraries(server PRIVATE fmt)
target_link_libraries(server PRIVATE hiredis redis++)



