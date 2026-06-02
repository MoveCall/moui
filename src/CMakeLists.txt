# ---------------------------------------------------------------------------
# Moui — ESP Component + Desktop build
# ---------------------------------------------------------------------------

# ── Core sources (always compiled) ──
set(MOUI_CORE_SRCS
    hal/moui_hal_desc.c
    core/moui_fb.c
    core/moui_draw.c
    core/moui_theme.c
    core/moui_style.c
    core/moui_event_bus.c
    core/moui_debug.c
    font/moui_font.c
    input/moui_input.c
    input/moui_focus.c
    input/moui_indev.c
    backend/moui_backend_fb.c
    backend/moui_backend_page.c
    layout/moui_layout.c
    screen/moui_screen.c
    widget/moui_widget.c
    widget/moui_widget_time_picker.c
    widget/moui_widget_barchart.c
    widget/moui_widget_roller.c
    widget/moui_widget_icon_bar.c
    widget/moui_widget_treeview.c
    widget/moui_widget_logview.c
    widget/moui_widget_vlist.c
    layout/moui_layout_flex.c
    layout/moui_anchor.c
    core/moui_property.c
    anim/moui_timeline.c
)

# ── Conditional sources (ESP: Kconfig, Desktop: all-on) ──
if(ESP_PLATFORM)
    # Features
    if(CONFIG_MOUI_USE_DITHER)
        list(APPEND MOUI_CORE_SRCS core/moui_dither.c)
    endif()
    if(CONFIG_MOUI_USE_PATTERN)
        list(APPEND MOUI_CORE_SRCS core/moui_pattern.c)
    endif()
    if(CONFIG_MOUI_USE_QRCODE)
        list(APPEND MOUI_CORE_SRCS core/moui_qrcode.c)
    endif()
    if(CONFIG_MOUI_USE_ICONS)
        list(APPEND MOUI_CORE_SRCS core/moui_icons.c)
    endif()
    if(CONFIG_MOUI_USE_I18N)
        list(APPEND MOUI_CORE_SRCS core/moui_i18n.c)
    endif()
    if(CONFIG_MOUI_USE_ANIM)
        list(APPEND MOUI_CORE_SRCS anim/moui_ease.c anim/moui_anim.c)
    endif()
    if(CONFIG_MOUI_USE_GRID)
        list(APPEND MOUI_CORE_SRCS layout/moui_grid.c)
    endif()
    if(CONFIG_MOUI_USE_POPUP)
        list(APPEND MOUI_CORE_SRCS screen/moui_popup.c)
    endif()

    # Widgets
    if(CONFIG_MOUI_USE_LABEL)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_label.c)
    endif()
    if(CONFIG_MOUI_USE_BUTTON)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_button.c)
    endif()
    if(CONFIG_MOUI_USE_LIST)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_list.c)
    endif()
    if(CONFIG_MOUI_USE_SLIDER)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_slider.c)
    endif()
    if(CONFIG_MOUI_USE_SWITCH)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_switch.c)
    endif()
    if(CONFIG_MOUI_USE_PROGRESS)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_progress.c)
    endif()
    if(CONFIG_MOUI_USE_RADIO)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_radio.c)
    endif()
    if(CONFIG_MOUI_USE_CHART)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_chart.c)
    endif()
    if(CONFIG_MOUI_USE_SCROLL)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_scroll.c)
    endif()
    if(CONFIG_MOUI_USE_STATUSBAR)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_statusbar.c)
    endif()
    if(CONFIG_MOUI_USE_SPINNER)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_spinner.c)
    endif()
    if(CONFIG_MOUI_USE_TAB)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_tab.c)
    endif()
    if(CONFIG_MOUI_USE_DROPDOWN)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_dropdown.c)
    endif()
    if(CONFIG_MOUI_USE_TEXTINPUT)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_textinput.c)
    endif()
    if(CONFIG_MOUI_USE_GAUGE)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_gauge.c)
    endif()
    if(CONFIG_MOUI_USE_TABLE)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_table.c)
    endif()
    if(CONFIG_MOUI_USE_BTNMATRIX)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_btnmatrix.c)
    endif()
    if(CONFIG_MOUI_USE_CALENDAR)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_calendar.c)
    endif()
    if(CONFIG_MOUI_USE_IMAGE)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_image.c)
    endif()
    if(CONFIG_MOUI_USE_RING)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_ring.c)
    endif()
    if(CONFIG_MOUI_USE_KEYBOARD)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_keyboard.c)
    endif()
    if(CONFIG_MOUI_USE_EXTRA)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_extra.c)
    endif()
    if(CONFIG_MOUI_USE_MISC)
        list(APPEND MOUI_CORE_SRCS widget/moui_widget_misc.c)
    endif()

    # Drivers
    if(CONFIG_MOUI_USE_DRV_SSD1306)
        list(APPEND MOUI_CORE_SRCS drivers/moui_drv_ssd1306.c)
    endif()
    if(CONFIG_MOUI_USE_DRV_SSD1309)
        list(APPEND MOUI_CORE_SRCS drivers/moui_drv_ssd1309.c)
    endif()
    if(CONFIG_MOUI_USE_DRV_SH1106)
        list(APPEND MOUI_CORE_SRCS drivers/moui_drv_sh1106.c)
    endif()
    if(CONFIG_MOUI_USE_DRV_ST7565)
        list(APPEND MOUI_CORE_SRCS drivers/moui_drv_st7565.c)
    endif()
    if(CONFIG_MOUI_USE_DRV_ST7567)
        list(APPEND MOUI_CORE_SRCS drivers/moui_drv_st7567.c)
    endif()
    if(CONFIG_MOUI_USE_DRV_ST7920)
        list(APPEND MOUI_CORE_SRCS drivers/moui_drv_st7920.c)
    endif()
    if(CONFIG_MOUI_USE_DRV_UC1701)
        list(APPEND MOUI_CORE_SRCS drivers/moui_drv_uc1701.c)
    endif()
    if(CONFIG_MOUI_USE_DRV_UC8151)
        list(APPEND MOUI_CORE_SRCS drivers/moui_drv_uc8151.c)
    endif()
    if(CONFIG_MOUI_USE_DRV_SSD1677)
        list(APPEND MOUI_CORE_SRCS drivers/moui_drv_ssd1677.c)
    endif()
    if(CONFIG_MOUI_USE_DRV_CST836U)
        list(APPEND MOUI_CORE_SRCS drivers/moui_drv_cst836u.c)
    endif()

    # Fonts
    set(MOUI_FONT_SRCS "")
    if(CONFIG_MOUI_USE_FONT_ASCII_6X8)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_ascii_6x8.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_INTER_12)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_inter_12.c)
    endif()

    if(CONFIG_MOUI_USE_FONT_INTER_16)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_inter_16.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_INTER_24)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_inter_24.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_INTER_32)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_inter_32.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_INTER_48)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_inter_48.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_INTER_16_AA)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_inter_16_aa.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_INTER_24_AA)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_inter_24_aa.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_CN_12)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_cn_12.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_CN_16)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_cn_16.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_CN_24)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_cn_24.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_CN_28)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_cn_28.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_CN_32)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_cn_32.c)
    endif()
    if(CONFIG_MOUI_USE_FONT_WQY_16)
        list(APPEND MOUI_FONT_SRCS ../fonts/moui_font_wqy_16.c)
    endif()

    idf_component_register(
        SRCS ${MOUI_CORE_SRCS} ${MOUI_FONT_SRCS}
        INCLUDE_DIRS "."
    )

else()
    # Desktop: compile everything
    file(GLOB_RECURSE MOUI_ALL_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    )
    file(GLOB MOUI_FONT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/../fonts/*.c
    )
    add_library(moui_framework STATIC ${MOUI_ALL_SRCS})
    target_include_directories(moui_framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(moui_framework PRIVATE -Wall -Wextra)
    target_link_libraries(moui_framework PRIVATE m)

    add_library(moui_fonts STATIC ${MOUI_FONT_SRCS})
    target_include_directories(moui_fonts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../fonts)
    target_link_libraries(moui_fonts PRIVATE moui_framework)
endif()
