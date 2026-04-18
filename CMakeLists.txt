cmake_minimum_required(VERSION 4.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(lollipopak_engine)

include_directories(
    thirdparty
    thirdparty/glm
    thirdparty/lua
    thirdparty/LuaBridge/Source
    thirdparty/box2d
    headers
)

file(GLOB LUA_SOURCES "thirdparty/lua/*.c")
file(GLOB HEADER_SOURCES "headers/*.cpp")
file(GLOB_RECURSE BOX2D_SOURCES 
    "thirdparty/box2d/collision/*.cpp"
    "thirdparty/box2d/common/*.cpp"
    "thirdparty/box2d/dynamics/*.cpp"
    "thirdparty/box2d/rope/*.cpp"
)

add_executable(lollipopak_engine game_engine.cpp ${HEADER_SOURCES} ${LUA_SOURCES} ${BOX2D_SOURCES})

if(DEFINED EMSCRIPTEN)
    target_compile_options(lollipopak_engine PRIVATE -Wno-error)
    set_target_properties(lollipopak_engine PROPERTIES 
        SUFFIX ".html"
        COMPILE_FLAGS "-sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_MIXER=2 -sUSE_SDL_TTF=2"
    )
    target_link_options(lollipopak_engine PRIVATE
        -sUSE_SDL=2
        -sUSE_SDL_IMAGE=2
        -sUSE_SDL_MIXER=2
        -sUSE_SDL_TTF=2
        -sALLOW_MEMORY_GROWTH=1
        -sWASM=1
        "--preload-file=${CMAKE_SOURCE_DIR}/resources@/resources"
    )
else()
    include_directories(
        macSDL/SDL2
        macSDL/SDL2_image
        macSDL/SDL2_mixer
        macSDL/SDL2_ttf
    )
    target_link_libraries(lollipopak_engine
        "-F${CMAKE_SOURCE_DIR}/macSDL/SDL2/lib -framework SDL2"
        "-F${CMAKE_SOURCE_DIR}/macSDL/SDL2_image/lib -framework SDL2_image"
        "-F${CMAKE_SOURCE_DIR}/macSDL/SDL2_mixer/lib -framework SDL2_mixer"
        "-F${CMAKE_SOURCE_DIR}/macSDL/SDL2_ttf/lib -framework SDL2_ttf"
    )
    set_target_properties(lollipopak_engine PROPERTIES
        BUILD_RPATH "${CMAKE_SOURCE_DIR}/macSDL/SDL2/lib;${CMAKE_SOURCE_DIR}/macSDL/SDL2_image/lib;${CMAKE_SOURCE_DIR}/macSDL/SDL2_mixer/lib;${CMAKE_SOURCE_DIR}/macSDL/SDL2_ttf/lib"
    )
endif()