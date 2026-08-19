project(Thorn)

file(GLOB_RECURSE SOURCE Thorn/*.cpp)
add_executable(Thorn ${SOURCE})

target_include_directories(Thorn PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Thorn PUBLIC Rose)