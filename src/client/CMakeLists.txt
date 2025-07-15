


# Add source to this project's executable.
file(GLOB_RECURSE client_sources "*.cpp")

add_executable(client 
${client_sources} 
${PROJECT_SOURCE_DIR}/src/sayHi.cpp
)
set_target_properties(client PROPERTIES DEBUG_POSTFIX "_d")
set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

target_include_directories(client PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(client PRIVATE /home/g/muduo/)

target_link_directories(client PRIVATE /home/g/build/release-cpp11/lib)

target_link_libraries(client PRIVATE muduo_base muduo_net)




