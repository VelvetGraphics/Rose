project (Rose)

file(GLOB_RECURSE SOURCE Source/Rose/*.cpp)

add_library(Rose STATIC ${SOURCE})
target_include_directories(Rose PUBLIC
        Source
        Vendor/entt
        Vendor/glm
)

add_subdirectory(Vendor/spdlog)
add_subdirectory(Vendor/GLFW)
find_package(Vulkan REQUIRED)
add_subdirectory(Vendor/vk-bootstrap)
add_subdirectory(Vendor/shaderc)

target_link_libraries(Rose PUBLIC
        spdlog
        glfw
        Vulkan::Vulkan
        vk-bootstrap
        shaderc
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

if (WIN32)
    target_compile_definitions(Rose PUBLIC ROSE_PLATFORM_WINDOWS)
    target_compile_definitions(Rose PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX)
elseif (APPLE)
    target_compile_definitions(Rose PUBLIC ROSE_PLATFORM_MACOS)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(Rose PUBLIC ROSE_PLATFORM_LINUX)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Dist")
    target_compile_definitions(Rose PUBLIC ROSE_DIST)

    target_compile_definitions(Rose PUBLIC SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_ERROR)
endif()

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT is_multi_config)
    set(ALLOWED_BUILD_TYPES "Debug" "Release" "RelWithDebInfo" "MinSizeRel" "Dist")
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${ALLOWED_BUILD_TYPES})
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Dist")
    message(STATUS "Configuring Rose for Distribution (All debug info stripped)...")

    if (MSVC)
        target_compile_options(Rose PRIVATE /O2 /Ob2 /DNDEBUG)
        target_link_options(Rose INTERFACE /DEBUG:NONE /INCREMENTAL:NO)
    else ()
        target_compile_options(Rose PRIVATE -O3 -DNDEBUG)
        target_link_options(Rose INTERFACE -s)
    endif ()
endif ()
