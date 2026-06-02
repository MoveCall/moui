# Demo Screens

The `demo_screens.c` file contains all 12 demo pages shared between the simulator and ESP32 builds.

## Structure

Each screen is a self-contained section:

| Line | Screen | Widgets Used |
|------|--------|--------------|
| 15 | Main Menu | Label, List |
| 88 | Dashboard | Label, Chart×2, Slider |
| 176 | Settings | Label, Slider×3, Button×4, Popup |
| 260 | Dual Waveform | Label, Chart×2 |
| 314 | Graphics Demo | Custom canvas (circle/line/rect animations) |
| 392 | Stress Test | Custom canvas (43 primitives/frame) |
| 469 | Widgets 1 | StatusBar, Progress, Loading, Dropdown, Switch, Radio, Spinner, Gauge |
| 591 | Widgets 2 | Tab, TextInput, BtnMatrix, Marquee, Badge, Divider, Icons |
| 716 | Mono Effects | Dithering, Pattern fills, QR code |
| 852 | Widgets 3 | Calendar, Table, ScrollView |
| 933 | Image Test | Full-screen bitmap |
| 963 | New Features | Dark mode toggle, Grid layout, Proportional font, Label wrap |
| 1038 | About | Label, Button, Confirm popup, Chinese font |

## Adding a New Screen

1. Add forward declaration: `static void push_myscreen(void);`
2. Add to `main_items[]` array
3. Add `case N: push_myscreen(); break;` in `on_main_select`
4. Update `moui_list_set_items` count
5. Write `setup_myscreen_screen()` function
6. Write `push_myscreen()` one-liner
7. Call `setup_myscreen_screen()` in `demo_setup_all_screens()`

## Platform Entry Points

- **Simulator**: `host/simulator/sim_main.c` — SDL2 init + main loop
- **ESP32**: See `examples/get_started/` for a minimal ESP-IDF project

Both call `demo_init(&mgr)` → `demo_setup_all_screens()` → `moui_screen_push(mgr, demo_get_main_screen())`.
