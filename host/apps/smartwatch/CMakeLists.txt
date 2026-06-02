find_path(SDL2_INCLUDE_DIR SDL2/SDL.h
    HINTS /opt/homebrew/include /usr/local/include /usr/include
)
find_library(SDL2_LIBRARY SDL2
    HINTS /opt/homebrew/lib /usr/local/lib /usr/lib
)

add_executable(watch_sim watch_main.c)

target_include_directories(watch_sim PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/host/simulator
    ${SDL2_INCLUDE_DIR}
)

target_link_libraries(watch_sim PRIVATE
    moui_framework
    moui_fonts
    ${SDL2_LIBRARY}
)

# Link sim_display and sim_input objects
target_sources(watch_sim PRIVATE
    ${CMAKE_SOURCE_DIR}/host/simulator/sim_display.c
    ${CMAKE_SOURCE_DIR}/host/simulator/sim_input.c
)

target_compile_options(watch_sim PRIVATE -Wall -Wextra)
