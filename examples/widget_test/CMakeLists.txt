cmake_minimum_required(VERSION 3.16)

# Widget Test — full-component coverage test for the moui framework
# on the OnePage C61 (SSD1677 800x480 E-Paper, 7-key input).
#
# Set IDF_PATH, then:
#   idf.py set-target esp32c61 && idf.py build && idf.py flash monitor

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_SOURCE_DIR}/../../src"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(widget_test)
