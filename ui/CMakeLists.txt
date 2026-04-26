cmake_minimum_required(VERSION 3.5)
project(ui)

# Create an option to switch between a system sdl library and a vendored SDL library
option(MYGAME_VENDORED "Use vendored libraries" OFF)

if(MYGAME_VENDORED)
    # This assumes you have added SDL as a submodule in vendored/SDL
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL3 package,
    # 2. look for the SDL3-shared component, and
    # 3. fail if the shared component cannot be found.
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

find_package(SDL3_ttf CONFIG REQUIRED)
 add_compile_options(-g -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes)

add_executable(ui ui.c ../fetcher.c ../cJSON.c ../path.c ../graph.c ../auto_completion.c ../challenge_generator.c ../deviation.c)


# Link to the actual SDL3 library.
target_link_libraries(ui PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf m curl utf8proc)
