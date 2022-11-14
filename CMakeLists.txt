# Basic parameters; check that these match your project / environment
cmake_minimum_required(VERSION 3.9)

# Replace "game" with a name for your project (this is used the name of the output)
project(limevolleyball)

# Add your sources here (adding headers is optional, but helps some CMake generators)
set(PROJECT_SOURCE
        # Simple game that calls update() init() of wasm game
        game.cpp game.hpp
        # WASM4 semi emulation
        src/fake_wasm4_core.h
        src/framebuffer.c
        src/framebuffer.h
        src/util.h
        src/util.c
        src/fake_wasm4.cpp
        src/wasm4.h # Use this file instead of original wasm4.h
        # Actual WASM4 game source goes here
        src/images.h
        src/main.c
        )

# ... and any other files you want in the release here
set(PROJECT_DISTRIBS LICENSE README.md)

# Build configuration; approach this with caution!
if(MSVC)
  add_compile_options("/W4" "/wd4244" "/wd4324" "/wd4458" "/wd4100")
else()
  add_compile_options("-Wall" "-Wextra" "-Wdouble-promotion" "-Wno-unused-parameter")
endif()

find_package (32BLIT CONFIG REQUIRED PATHS ../32blit-sdk $ENV{PATH_32BLIT_SDK})

blit_executable (${PROJECT_NAME} ${PROJECT_SOURCE})
blit_assets_yaml (${PROJECT_NAME} assets.yml)
blit_metadata (${PROJECT_NAME} metadata.yml)
add_custom_target (flash DEPENDS ${PROJECT_NAME}.flash)
if(NOT MSVC)
  target_compile_options(${PROJECT_NAME} PUBLIC -Wno-error=double-promotion)
endif()

# setup release packages
install (FILES ${PROJECT_DISTRIBS} DESTINATION .)
set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set (CPACK_GENERATOR "ZIP" "TGZ")
include (CPack)
