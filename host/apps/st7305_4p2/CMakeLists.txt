find_path(SDL2_INCLUDE_DIR SDL2/SDL.h
    HINTS /opt/homebrew/include /usr/local/include /usr/include
)
find_library(SDL2_LIBRARY SDL2
    HINTS /opt/homebrew/lib /usr/local/lib /usr/lib
)

add_executable(st7305_4p2 st7305_4p2_main.c)

target_include_directories(st7305_4p2 PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/host/simulator
    ${SDL2_INCLUDE_DIR}
)

target_link_libraries(st7305_4p2 PRIVATE
    moui_demo
    moui_framework
    moui_fonts
    ${SDL2_LIBRARY}
)

target_sources(st7305_4p2 PRIVATE
    ${CMAKE_SOURCE_DIR}/host/simulator/sim_display.c
    ${CMAKE_SOURCE_DIR}/host/simulator/sim_input.c
)

target_compile_options(st7305_4p2 PRIVATE -Wall -Wextra)
