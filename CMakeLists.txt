cmake_minimum_required(VERSION 3.20)
project(RobloxStyleObby LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
set(FETCHCONTENT_QUIET TRUE)

# Pin raylib to 5.5
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

set(RAYGUI_INCLUDE_DIR "${raylib_SOURCE_DIR}/examples/shapes")

add_executable(obby
    src/main.cpp
    src/vec3.cpp
    src/aabb.cpp
    src/render_utils.cpp
    src/entity.cpp
    src/platform.cpp
    src/moving_platform.cpp
    src/crumble_tile.cpp
    src/spikes.cpp
    src/spinner.cpp
    src/bounce_pad.cpp
    src/checkpoint.cpp
    src/finish_zone.cpp
    src/button.cpp
    src/door.cpp
    src/game.cpp
    src/collision.cpp
    src/camera.cpp
    src/player.cpp
    src/level.cpp
    src/ui.cpp
    src/audio.cpp
    src/particle.cpp
    src/validator.cpp
)

target_include_directories(obby PRIVATE
    src
)

# Mark third-party raylib & raygui includes as SYSTEM to suppress external library warnings
target_include_directories(obby SYSTEM PRIVATE
    ${raylib_SOURCE_DIR}/src
    ${RAYGUI_INCLUDE_DIR}
)

if(MSVC)
    target_compile_options(obby PRIVATE /W4)
else()
    target_compile_options(obby PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

target_link_libraries(obby PRIVATE raylib)
