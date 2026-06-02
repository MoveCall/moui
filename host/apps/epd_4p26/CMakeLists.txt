find_path(SDL2_INCLUDE_DIR SDL2/SDL.h
    HINTS /opt/homebrew/include /usr/local/include /usr/include
)
find_library(SDL2_LIBRARY SDL2
    HINTS /opt/homebrew/lib /usr/local/lib /usr/lib
)

add_executable(epd_4p26 epd_4p26_main.c)

target_include_directories(epd_4p26 PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/host/simulator
    ${SDL2_INCLUDE_DIR}
)

target_link_libraries(epd_4p26 PRIVATE
    moui_demo
    moui_framework
    moui_fonts
    ${SDL2_LIBRARY}
)

target_sources(epd_4p26 PRIVATE
    ${CMAKE_SOURCE_DIR}/host/simulator/sim_display.c
    ${CMAKE_SOURCE_DIR}/host/simulator/sim_input.c
)

target_compile_options(epd_4p26 PRIVATE -Wall -Wextra)
