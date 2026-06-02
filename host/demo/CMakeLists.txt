add_library(moui_demo STATIC
    demo_screens.c
)
target_include_directories(moui_demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(moui_demo PUBLIC moui_framework moui_fonts)
