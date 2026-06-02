find_path(SDL2_INCLUDE_DIR SDL2/SDL.h
    HINTS /opt/homebrew/include /usr/local/include /usr/include
)
find_library(SDL2_LIBRARY SDL2
    HINTS /opt/homebrew/lib /usr/local/lib /usr/lib
)

if(NOT SDL2_INCLUDE_DIR OR NOT SDL2_LIBRARY)
    message(FATAL_ERROR "SDL2 not found")
endif()

add_executable(moui_sim
    sim_main.c
    sim_display.c
    sim_input.c
)

target_include_directories(moui_sim PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${SDL2_INCLUDE_DIR}
)

target_link_libraries(moui_sim PRIVATE
    moui_demo
    moui_framework
    moui_fonts
    ${SDL2_LIBRARY}
)

target_compile_options(moui_sim PRIVATE -Wall -Wextra)
