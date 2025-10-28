cmake_minimum_required(VERSION 4.0)
project(Rollback_Learning)

set(CMAKE_CXX_STANDARD 17)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(raylib)
    endif()
endif()

add_subdirectory(src/3rd-party/plog) # Adds plog to your CMake project

include_directories(nbnet-2.0)


add_executable(Rollback_Learning Source.cpp
        Network.cpp
        Network.h
)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} wsock32 ws2_32)
endif()

target_link_libraries(${PROJECT_NAME} raylib)
target_link_libraries(${PROJECT_NAME} plog::plog)
