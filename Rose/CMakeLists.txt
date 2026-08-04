project(Rose)

file(GLOB_RECURSE SOURCE Source/Rose/*.cpp)

add_library(Rose STATIC ${SOURCE})
target_include_directories(Rose PUBLIC
        Source
        Vendor/entt
)

add_subdirectory(Vendor/spdlog)

target_link_libraries(Rose PUBLIC
        spdlog
)

file(GLOB_RECURSE ROSE_COMMON_INCLUDE Source/Rose/Common/CommonInclude.hpp)

target_precompile_headers(Rose PUBLIC
        ${ROSE_COMMON_INCLUDE}
)

get_filename_component(
        ROSE_COMMON_INCLUDE_ABS
        "${ROSE_COMMON_INCLUDE}"
        ABSOLUTE
        BASE_DIR "Source"
)

if (MSVC)
    target_compile_options(Rose PUBLIC
            "$<$<COMPILE_LANGUAGE:CXX>:/FI${ROSE_COMMON_INCLUDE}>"
    )
else ()
    target_compile_options(Rose PUBLIC
            "$<$<COMPILE_LANGUAGE:CXX>:SHELL:-include \"${ROSE_COMMON_INCLUDE_ABS}\">"
    )
endif ()