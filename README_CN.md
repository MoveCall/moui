<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="assets/moui-lockup-dark.svg">
    <source media="(prefers-color-scheme: light)" srcset="assets/moui-lockup-light.svg">
    <img alt="Moui UI Framework Logo" src="assets/moui-lockup-dark.svg" width="600">
  </picture>
</p>

<p align="center">
  <b>专为 OLED、全反射液晶屏 (RLCD) 及电子纸 (E-Paper) 打造的超轻量级单色/灰度 GUI 框架</b>
</p>

<p align="center">
  <b>简体中文</b> | <a href="README.md"><b>English</b></a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/语言-C11-blue.svg" alt="Language">
  <img src="https://img.shields.io/badge/ESP--IDF-%3E%3Dv5.0-red.svg" alt="ESP-IDF">
  <img src="https://img.shields.io/badge/开源协议-MIT-green.svg" alt="License">
  <img src="https://img.shields.io/badge/单元测试-71%2F71%20PASS-brightgreen.svg" alt="Tests">
  <img src="https://img.shields.io/badge/内存开销-%3C18KB-orange.svg" alt="RAM">
</p>

---

ESP 组件库索引: `movecall/moui`

> 📢 **项目状态与社区邀请**
> 
> Moui 目前处于 **快速迭代与早期预览阶段（Beta / Developer Preview）**。虽然核心架构与基础控件已通过自动化测试，但在更多真实硬件型号、复杂交互场景下的表现，仍非常需要广大开发者的共同探索与验证。
> 
> 我们非常期待您的参与！无论是在实际硬件上测试发现的 Bug、性能优化建议，还是全新的 Widget 需求，都欢迎提交 [Issues](../../issues) 或 [Pull Requests](../../pulls)。您的每一次测试与反馈，都是 Moui 走向成熟稳定的关键力量！🚀

## 📖 核心文档索引

- 📘 [**API 参考手册**](docs/api_reference.md) — 30 种控件、布局引擎与动画系统的完整 API 说明
- 🛠️ [**硬件移植指南**](docs/porting_guide.md) — MCU 移植、显示驱动接入与 Page-Buffer 极小内存模式步骤
- ⚡ [**性能与演进日志**](docs/optimization_log.md) — Benchmark 跑分测速、内存 Profiling 与版本演进日志

## 📦 安装与集成

```yaml
# idf_component.yml
dependencies:
  movecall/moui: "^0.1.3"
```

## ⚡ 极简上手示例 ("Hello Moui")

```c
#include "moui.h"

void app_init(moui_screen_mgr_t *mgr) {
    static moui_screen_t main_scr;
    moui_screen_init(&main_scr);

    // 创建一个标题标签
    static moui_widget_label_t title;
    moui_label_init(&title, "Hello Moui!", &moui_font_inter_16);
    title.base.bounds = (moui_rect_t){20, 20, 160, 30};
    moui_screen_add_widget(&main_scr, &title.base);

    // 压入屏幕栈显示
    moui_screen_push(mgr, &main_scr);
}
```

## ✨ 核心特性

- **极小内存开销**：Flash 占用仅 ~45 KB，静态 RAM 占用低至 ~18 KB
- **4 级灰度支持**：`moui_color_t` 支持 WHITE / LGRAY / DGRAY / BLACK，内置 1bpp 与 2bpp 双后端
- **30 款原生 Widget 控件库**：涵盖 `VirtualList`（百万级数据 < 1KB RAM）、`TimePicker`、`BarChart`、`Roller`、`IconBar`、`TreeView`、`LogView` 等
- **128 款 Material Mono 矢量图标库**：内置 16x16 常用矢量单色图标，支持 1x/2x/3x 整数无损缩放 (`moui_draw_icon_scaled`) 与矩形居中对齐
- **4 大现代化 UI 架构**：Flexbox 弹性布局 (`moui_layout_flex`)、Anchor 锚点约束 (`moui_anchor`)、Reactive 响应式数据绑定 (`moui_property`)、Timeline 关键帧动画 (`moui_timeline`)
- **流畅 50 fps 动画引擎**：支持 10 种缓动曲线 + 链式/并行动画组 + 渐进式 Lerp 平滑过渡
- **跨平台一致性**：同一套 UI C 语言代码无缝运行于嵌入式 MCU 及 SDL2 桌面模拟器
- **10 款内置显示驱动**：SSD1306 / SSD1309 / SH1106 / ST7565 / ST7567 / ST7920 / UC1701 / UC8151 / SSD1677 / ST7305
- **Kconfig 可裁剪架构**：通过 `idf.py menuconfig` 自由选择按需打包的控件、驱动与字体
- **字体 Dropback 降级链**：通过 `moui_font_set_fallback()` 实现 CJK 中英文自动混排渲染
- **后端抽象设计**：`moui_backend_t` 统一接口，支持 Full-FB 全帧缓冲与 Page-Buffer 页缓冲双模式
- **编码器原生交互**：中断安全环形缓冲区 + 焦点链管理 + 长按/捕获手势
- **单色专属优化**：7 种 Dither 抖动填充 + 8 种纹理填充 + QR 二维码生成 + RLE / Floyd-Steinberg 照片抖动算法
- **全向屏幕旋转**：支持 0° / 90° / 180° / 270° 旋转，提供 Mode A (零 RAM 像素映射) 与 Mode B (Flush 内存转置)

## 📁 项目目录结构

```
moui/
├── src/                核心框架代码 (跨平台)
│   ├── hal/            HAL 硬件抽象层 + 显示描述符
│   ├── core/           绘图引擎、Dither 抖动、纹理、二维码、128 款图标、主题、事件
│   ├── backend/        后端抽象: moui_backend_t + FB / Page-Buffer
│   ├── drivers/        10 款显示驱动模板 (含 ST7305 4.2 寸驱动)
│   ├── font/           字体引擎与混排降级链
│   ├── input/          输入环形队列、焦点管理器、输入设备映射
│   ├── anim/           10 种缓动函数 + Timeline 关键帧动画引擎
│   ├── widget/         30 款原生控件 (含 VirtualList, TimePicker, BarChart, Roller 等)
│   ├── layout/         容器布局: Stack、Grid、Flexbox、Anchor
│   └── screen/         屏幕栈管理器 + 7 种切页过渡动画 + Popup 弹窗
├── fonts/              字体点阵数据 + 生成器工具
├── examples/           ESP-IDF 移植示例与 HAL 参考
├── host/               桌面模拟器 + Demo 演练套件 (微信风格 Chat Demo, App 框架 Demo)
├── tools/              图片转 C 数组工具 (img_to_c.py / gen_font.py)
├── idf_component.yml   IDF 组件元数据
└── CMakeLists.txt      双模构建脚本 (ESP-IDF + Linux/macOS CMake)
```

## 🚀 快速上手

### 桌面模拟器

```bash
cmake -B build && cmake --build build
./build/host/apps/st7305_4p2/st7305_4p2
```

内置模拟器：
- `st7305_4p2` — 4.2 寸 300x400 全反射式液晶屏 (RLCD) 模拟器（内置 17 个 Demo 演示屏：10 万条 VirtualList 虚拟列表、微信风格 Chat Demo、App 4-Tab 框架 Demo、128 图标画廊、实景黑白照片抖动测试）
- `moui_sim` — 标准 128x64 OLED 模拟器
- `watch_sim` — 智能手表 UI 模拟器

模拟器按键说明：`方向键 Up/Down` 切换焦点 / `Enter` 确认 / `ESC` 返回 / `R` 旋转屏幕方向

### ESP-IDF 嵌入式开发

1. 在项目中添加依赖组件（参见上文安装说明）
2. 运行 `idf.py menuconfig` > Moui Configuration 选择所需驱动与控件
3. 参考 `examples/esp32s3_port/` 完成 HAL 层配置

### 3 步接入新屏幕驱动

```c
// 1. 实现底层的两个写指令/写数据函数
void my_write_cmd(uint8_t cmd, void *user) { /* SPI/I2C 命令 */ }
void my_write_data(const uint8_t *d, uint32_t len, void *user) { /* 数据 */ }

// 2. 初始化驱动结构体
moui_drv_ssd1306_t display;
moui_drv_ssd1306_init(&display, &cfg);

// 3. 创建绘图上下文
moui_draw_ctx_t ctx;
moui_draw_ctx_init_be(&ctx, moui_drv_ssd1306_backend(&display));
```

## 🧩 控件列表 (30 款)

| 控件 (Widget) | 功能描述 |
|---|---|
| Label | 文本标签 (支持反色、自动换行、比例字体) |
| List | 滚动列表 (带动画选中指示框) |
| VirtualList | 高性能 Cell 复用虚拟列表 (10 万+ 数据项 <1KB RAM) |
| Button | 按钮 (支持 Push 押按 / Toggle 开关 / Checkbox 复选) |
| Slider | 滑块选择器 |
| Chart | 实时折线波形图 (环形缓冲区) |
| ScrollView | 自由滚动视图 (带右侧像素滚动条) |
| StatusBar | 顶部状态栏 (标题 + 时间 + 电池信号) |
| Progress | 进度条 |
| Radio | 单选框组 |
| Spinner | 数字微调计数器 |
| Tab | 选项卡切页 |
| Dropdown | 下拉选择菜单 |
| TextInput | 单行字符输入框 |
| TimePicker | 时分秒 3D 滚轮选择器 |
| BarChart | 动态直方柱状图 (带网格参考线与 Dither 抖动填充) |
| Roller | 3D 透视滚轮选择框 |
| IconBar | RSSI WiFi 信号柱 + 智能电池电量状态栏 |
| TreeView | 层级树状节点视图 (支持 [+] / [-] 展开与折叠) |
| LogView | 控制台日志视图 (内置环形缓冲区与置底滚动) |
| Gauge | 半圆仪表盘 |
| Table | 数据表格 |
| BtnMatrix | 按钮矩阵 |
| Switch | 滑动开关 |
| Calendar | 日历控件 |
| Image | 位图显示 (支持 RLE 压缩与 Floyd-Steinberg 抖动) |
| Ring | 环形弧度进度条 |
| Keyboard | 屏幕虚拟键盘 |
| Extra | 点阵 / 独立大数字 / 步进器 / 迷你 Sparkline 折线 / 清单 Checklist |
| Misc | 跑马灯 Marquee / 徽章角标 Badge / 分隔线 Divider / 加载动画 Loading |

## ⚡ 性能指标与优化记录

| 指标 | 参数 / 性能 |
|---|---|
| 帧率 (Frame Rate) | 50 fps 标称 |
| `hline` 极速绘制 | 字节级 memset 优化，获得 3-5 倍渲染提升 |
| 180° 旋转转置 | < 12 μs / 帧 (Mode B 获得 20 倍性能提升) |
| 全屏 DMA 刷屏 | 1.6 ms (8064 B @ 40 MHz) |
| 智能刷新机制 (Smart Refresh) | 无变化自动跳过渲染，零 CPU 占用 |
| 内存安全与稳定性 | 70 / 70 项单元测试 **100% PASS** |
| Code Flash 占用 | ~45 KB |
| RAM 占用 | ~18 KB (VirtualList < 1 KB) |

## 📄 开源许可

MIT License
