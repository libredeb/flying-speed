cmake_minimum_required(VERSION 3.16)
project(flying_speed VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

pkg_check_modules(SDL2_IMAGE IMPORTED_TARGET SDL2_image)
if(NOT SDL2_IMAGE_FOUND)
    pkg_check_modules(SDL2_IMAGE IMPORTED_TARGET sdl2_image)
endif()
if(NOT SDL2_IMAGE_FOUND)
    message(FATAL_ERROR "SDL2_image not found (pkg-config: SDL2_image / sdl2_image)")
endif()

pkg_check_modules(SDL2_TTF IMPORTED_TARGET SDL2_ttf)
if(NOT SDL2_TTF_FOUND)
    pkg_check_modules(SDL2_TTF IMPORTED_TARGET sdl2_ttf)
endif()
if(NOT SDL2_TTF_FOUND)
    message(FATAL_ERROR "SDL2_ttf not found (pkg-config: SDL2_ttf / sdl2_ttf)")
endif()

pkg_check_modules(SDL2_MIXER IMPORTED_TARGET SDL2_mixer)
if(NOT SDL2_MIXER_FOUND)
    pkg_check_modules(SDL2_MIXER IMPORTED_TARGET sdl2_mixer)
endif()
if(NOT SDL2_MIXER_FOUND)
    message(FATAL_ERROR "SDL2_mixer not found (pkg-config: SDL2_mixer / sdl2_mixer)")
endif()

add_executable(flying-speed
    src/main.cpp
    src/Util.cpp
    src/Input.cpp
    src/Audio.cpp
    src/Assets.cpp
    src/Trail.cpp
    src/Parallax.cpp
    src/Pipes.cpp
    src/Bird.cpp
    src/Game.cpp
)

target_include_directories(flying-speed PRIVATE src)

target_link_libraries(flying-speed PRIVATE
    PkgConfig::SDL2
    PkgConfig::SDL2_IMAGE
    PkgConfig::SDL2_TTF
    PkgConfig::SDL2_MIXER
)

if(NOT MSVC)
    target_compile_options(flying-speed PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(TARGET flying-speed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:flying-speed>/assets"
    COMMENT "Copying assets next to the executable"
)

install(TARGETS flying-speed RUNTIME DESTINATION bin)
install(DIRECTORY assets DESTINATION share/flying-speed)
