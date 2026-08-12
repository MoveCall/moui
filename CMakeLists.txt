# ---------------------------------------------------------------------------
# Moui — Root CMakeLists.txt (ESP-IDF Component + Desktop Simulator)
# ---------------------------------------------------------------------------

if(ESP_PLATFORM)
    # ESP-IDF Component Build: Skip top-level project() call to avoid
    # 'project command is not scriptable' error in ESP-IDF dependency scanning pass.
    add_subdirectory(src)
else()
    # Desktop Simulator / Unit Test Build
    cmake_minimum_required(VERSION 3.16)
    project(moui C)

    set(CMAKE_C_STANDARD 11)

    option(BUILD_SIMULATOR "Build desktop simulator" ON)
    option(BUILD_TESTS "Build unit tests" ON)

    add_subdirectory(src)
    add_subdirectory(host/demo)

    if(BUILD_SIMULATOR)
        add_subdirectory(host/simulator)
        add_subdirectory(host/apps/smartwatch)
        add_subdirectory(host/apps/epd_demo)
        add_subdirectory(host/apps/epd_4p26)
        add_subdirectory(host/apps/st7305_4p2)
    endif()

    if(BUILD_TESTS)
        add_executable(moui_tests test/test_all.c)
        target_include_directories(moui_tests PRIVATE ${CMAKE_SOURCE_DIR})
        target_link_libraries(moui_tests PRIVATE moui_framework moui_fonts)
        target_compile_options(moui_tests PRIVATE -Wall -Wextra -O1)

        add_executable(moui_bench test/benchmark.c)
        target_include_directories(moui_bench PRIVATE ${CMAKE_SOURCE_DIR})
        target_link_libraries(moui_bench PRIVATE moui_framework moui_fonts)
        target_compile_options(moui_bench PRIVATE -Wall -Wextra -O2)
    endif()
endif()
