# LVGL 图形界面学习笔记（v7 为主 + v7/v8 对比 + 图形化设计 + 配置裁剪）

> 本笔记基于实际项目经验整理（机械臂 800×480 + 空气质量检测仪 320×240），用**真实代码**讲解 LVGL 的移植、使用、图形化设计、配置裁剪。
> 版本：LVGL v7.11 为主，附 v7/v8 差异对比表。

---

## 目录

1. [LVGL 是什么 & 为什么用](#一lvgl-是什么--为什么用)
2. [LVGL 运行机制（核心理解）](#二lvgl-运行机制核心理解)
3. [显示移植 lv_port_disp](#三显示移植-lv_port_disp)
4. [输入移植 lv_port_indev](#四输入移植-lv_port_indev)
5. [tick 与任务集成](#五tick-与任务集成)
6. [核心概念：对象/样式/事件/动画](#六核心概念对象样式事件动画)
7. [常用控件详解](#七常用控件详解)
8. [GUI Guider 图形化设计（重点）](#八gui-guider-图形化设计重点)
9. [配置裁剪 lv_conf.h（重点）](#九配置裁剪-lv_confh重点)
10. [内存管理与性能优化](#十内存管理与性能优化)
11. [v7 vs v8 差异对比](#十一v7-vs-v8-差异对比)
12. [踩坑记录](#十二踩坑记录)
13. [问答](#十三问答)

---

## 一、LVGL 是什么 & 为什么用

**LVGL（Light and Versatile Graphics Library）** = 嵌入式图形库，帮你在单片机上做出手机 UI 一样的界面。

**为什么不用裸机画点**：
- 裸机画点只能画矩形/直线，做不出"按钮、滑块、下拉框、进度条"
- LVGL 内置几十种控件 + 动画 + 事件系统 + 主题，开箱即用

**为什么 LVGL 而不是其他**：
| 库 | 特点 |
|----|------|
| **LVGL** | 免费、开源、资源占用小、控件丰富、社区活跃 |
| TouchGFX | 商用收费、效果好但贵 |
| emWin | 商用收费 |
| AWTK | 国产、支持多平台 |

**LVGL 版本现状**：
- **v7.x**（本笔记主讲的）：API 较老，但稳定、资料多、老项目在用
- **v8.x**：API 重构（样式系统改了），更现代
- **v9.x**：最新，改动更大

> **面试点**：能讲清"LVGL 是什么 + 为什么选它 + 版本差异"就是合格的 GUI 选型回答。

---

## 二、LVGL 运行机制（核心理解）

### 2.1 LVGL 不是操作系统

LVGL 是**库**，不是 RTOS。它需要你提供：
1. **tick 心跳**：告诉它时间（动画/事件用）
2. **周期调用 `lv_task_handler()`**：驱动它工作
3. **两个移植回调**：`disp_flush`（显示）+ `touchpad_read`（输入）

```
你的主循环（每 5ms）
  └── lv_task_handler()
        ├── 调 touchpad_read → 获取触摸
        ├── 判断坐标落在哪个控件 → 触发事件
        ├── 检查需要重绘的区域 → 渲染到内部缓冲
        └── 调 disp_flush → 把缓冲刷到屏幕
```

> **核心理解：LVGL 是"被驱动"的**——你不调 `lv_task_handler()`，它什么都不做。它和 FreeRTOS 是**独立**的，可以配合（LVGL 任务 + FreeRTOS），也可以裸机跑。

### 2.2 LVGL 的三层结构

```
┌─────────────────────────────┐
│ 应用层：控件、事件、动画       │  ← 你写的代码
├─────────────────────────────┤
│ LVGL 内核：渲染、样式、布局    │  ← LVGL 库
├─────────────────────────────┤
│ 移植层：disp_flush/touchpad  │  ← 你写的适配代码（连接硬件）
└─────────────────────────────┘
```

### 2.3 显示缓冲机制（重点）

LVGL 画图需要一个**缓冲**（画布），画完交给 `disp_flush` 刷到屏幕：

```
LVGL 渲染 → 内部缓冲（RAM） → disp_flush → LCD
```

**三种缓冲配置**（lv_port_disp.c 注释里有）：

| 配置 | 缓冲大小 | 特点 |
|------|---------|------|
| **单缓冲（本项目）** | 800×10 行 = 16KB | 简单，渲染完才刷，速度一般 |
| 双缓冲 | 2 × 16KB = 32KB | 渲染和刷新并行（DMA），快 |
| 全屏双缓冲 | 2 × 768KB | 最快，但 64KB RAM 放不下 |

**为什么不能全屏缓冲**：800×480×2字节 = 768KB，STM32F103 只有 64KB RAM。用"部分缓冲"（10 行），LVGL 分块渲染。

> **面试点**：能讲清"为什么用部分缓冲" = 理解嵌入式内存限制。

### 2.4 v7 与 v8 的核心差异（提前看，后面详讲）

| 方面 | v7 | v8 |
|------|----|----|
| 样式 API | `lv_obj_set_style_local_xxx(obj, part, state, val)` | `lv_obj_set_style_xxx(obj, val, selector)` |
| 隐藏控件 | `lv_obj_set_hidden(obj, true)` | `lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN)` |
| 对齐 | `lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0)` | `lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0)` |
| 屏幕加载 | `lv_scr_load_anim(scr, anim, time, delay, autodel)` | `lv_scr_load_anim(scr, anim, time, delay, false)` |
| 堆剩余 | `lv_mem_monitor(&mon); mon.free_size` | `lv_mem_get_free()` |

---

## 三、显示移植（lv_port_disp）

> 移植 = 让 LVGL 知道"怎么把你的界面画到我的屏幕上"。核心是**三步**：初始化显示 → 分配缓冲 → 实现 disp_flush。

### 3.1 移植三步（以机械臂 800×480 为例）

**第一步：初始化显示（disp_init）**

```c
static void disp_init(void)
{
    LCD_Init();              // 初始化 LCD（SSD1963 等）
    LCD_Display_Dir(1);      // 设置横屏
    LCD_Clear(WHITE);        // 清屏
}
```

**第二步：分配显示缓冲**

```c
// 800 × 10 行 × 2字节(RGB565) = 16KB
static lv_disp_buf_t draw_buf_dsc_1;
static lv_color_t draw_buf_1[LV_HOR_RES_MAX * 10];   // 10 行缓冲
lv_disp_buf_init(&draw_buf_dsc_1, draw_buf_1, NULL, LV_HOR_RES_MAX * 10);
```

**第三步：注册显示驱动**

```c
lv_disp_drv_t disp_drv;
lv_disp_drv_init(&disp_drv);
disp_drv.hor_res = 800;          // 屏幕宽
disp_drv.ver_res = 480;          // 屏幕高
disp_drv.flush_cb = disp_flush;  // 刷新回调
disp_drv.buffer = &draw_buf_dsc_1;
lv_disp_drv_register(&disp_drv); // 注册！
```

### 3.2 实现 disp_flush（两种方案，你的两个项目都用过）

**disp_flush 的职责**：把 LVGL 渲染好的像素区域（area）写到屏幕。

**参数**：
- `area`：要刷新的矩形区域（x1,y1,x2,y2）
- `color_p`：像素数据指针

**方案 A：整区域填充（空气质量检测仪）**

```c
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    LCD_Color_Fill(area->x1, area->y1, area->x2, area->y2, (uint16_t *)color_p);
    lv_disp_flush_ready(disp_drv);   // ★ 必须调用！
}
```

**方案 B：逐行填充（机械臂，解决花屏）**

```c
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    int32_t y;
    for (y = area->y1; y <= area->y2; y++)
    {
        // 每次只写一行
        LCD_Color_Fill(area->x1, y, area->x2, y, (uint16_t *)color_p);
        color_p += lv_area_get_width(area);   // 指针移到下一行
    }
    lv_disp_flush_ready(disp_drv);
}
```

> **为什么机械臂用逐行**：整区域批量写会花屏（排线信号不稳定），逐行写每行之间有一次函数调用间隙，信号稳定。速度略慢但稳定。

### 3.3 `lv_disp_flush_ready` 为什么必须调用（重点）

```c
lv_disp_flush_ready(disp_drv);
```

- 它的作用：**告诉 LVGL"缓冲刷完了，可以重新用了"**
- 不调用 → LVGL 一直等 → **界面卡死**
- 如果用了 DMA，要在 DMA 完成中断里调用

> **面试点**：`lv_disp_flush_ready` 是移植最容易漏、漏了最致命的一步。

### 3.4 三种缓冲配置对比（面试常问）

| 配置 | RAM 占用 | 速度 | 适用 |
|------|---------|------|------|
| 单缓冲 | 800×10×2=16KB | 慢（渲染完才刷）| RAM 紧张（本项目）|
| 双缓冲 | 32KB | 快（渲染/刷新并行）| RAM 够用 + DMA |
| 全屏双缓冲 | 768KB×2 | 最快 | 大 RAM 芯片（F429 等）|

> **面试点**：能讲出"单缓冲 vs 双缓冲"的区别和取舍 = 理解显示流水线。

---

## 四、输入移植（lv_port_indev）

> 触摸移植 = 让 LVGL 知道"手指在哪"。核心是**一个回调**：`touchpad_read`。

### 4.1 注册输入设备

```c
lv_indev_drv_t indev_drv;
lv_indev_drv_init(&indev_drv);
indev_drv.type = LV_INDEV_TYPE_POINTER;   // 类型：触摸屏
indev_drv.read_cb = touchpad_read;        // 读取回调
lv_indev_drv_register(&indev_drv);
```

### 4.2 实现 touchpad_read（项目真实代码）

```c
static bool touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    TouchPoint_t tp;
    if (touch_scan(&tp, 1))   // 从 GT911 读坐标
    {
        data->point.x = tp.x;   // 屏幕坐标 X
        data->point.y = tp.y;   // 屏幕坐标 Y
        data->state = LV_INDEV_STATE_PR;   // 按下
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;  // 松开
    }
    return false;
}
```

### 4.3 LVGL 如何使用它

每次 `lv_task_handler()` 内部：
1. 调 `touchpad_read` 获取当前触摸状态
2. 判断坐标落在哪个控件上（命中检测）
3. 触发该控件的事件（PRESSED/CLICKED/...）
4. 调用你注册的事件回调

**事件回调注册**：

```c
lv_obj_set_event_cb(ui->screen_1_btn_5, btn_5_event_handler);
// 之后按钮被点击时，LVGL 自动调 btn_5_event_handler
```

> **面试点**：触摸坐标要在 touchpad_read 里**转换成屏幕方向一致**的坐标（横屏/竖屏要映射），否则点击位置错位。

---

## 五、tick 与任务集成

### 5.1 tick 时钟（LVGL 的时间来源）

LVGL 需要知道时间（动画、长按检测、去抖）。**tick 必须持续喂**，否则动画不转、事件不触发。

**FreeRTOS 环境下**（项目做法）：用 tick 钩子

```c
void vApplicationTickHook(void)
{
    lv_tick_inc(1);   // 每 1ms 喂一次
}
```

**裸机环境下**：用定时器中断

```c
void TIM7_IRQHandler(void)
{
    lv_tick_inc(1);   // 定时器 1ms 中断
}
```

> **面试点**：`lv_tick_inc` 喂的间隔要稳定（1ms），喂的不准 → 动画速度不对。

### 5.2 LVGL 任务（周期调用 lv_task_handler）

**FreeRTOS 任务**（项目做法）：

```c
void vLvglTaskFunction(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(5);   // 5ms 周期

    for (;;)
    {
        lv_task_handler();       // 驱动 LVGL
        vTaskDelayUntil(&xLastWakeTime, xPeriod);  // 绝对延时，周期稳定
    }
}
```

**为什么用 `vTaskDelayUntil`（绝对延时）而不是 `vTaskDelay`**：
- `vTaskDelay(5)`：从"调用时刻"起延时 5ms，实际周期 = 5ms + 执行时间（漂移）
- `vTaskDelayUntil`：从"上次唤醒时刻"起延时，周期恒定

### 5.3 LVGL 线程安全（重点中的重点）

**LVGL 不是线程安全的**——所有 LVGL 操作（创建控件、改属性、刷界面）**必须在同一个任务里做**。

**项目架构**：

```c
// Task_Lvgl（唯一操作 LVGL 的任务）
void Task_Lvgl(void *pvParameters)
{
    setup_ui(&guider_ui);   // 创建界面（GUI Guider 生成）

    for (;;)
    {
        if (g_lv_update_req)      // 别的任务请求更新界面
        {
            g_lv_update_req = 0;
            AutoDisp_Update(...); // 在 LVGL 任务里操作控件（安全）
        }
        lv_task_handler();
        vTaskDelay(5ms);
    }
}

// 其他任务（如串口收数据）只能置标志：
g_lv_update_req = 1;   // 不能直接调 LVGL API！
```

**为什么不能在别的任务直接调 LVGL API**：会和其他任务正在执行的 LVGL 操作冲突，破坏内部链表/状态 → 卡死或花屏。

> **面试点**："LVGL 线程安全"是高频问题。标准回答：LVGL 操作集中在单任务 + 其他任务用标志位请求更新。

### 5.4 GUI Guider 生成的界面怎么加载

```c
// GUI Guider 生成：setup_scr_screen_1() 创建界面 + guider_ui 结构体保存控件指针
setup_scr_screen_1(&guider_ui);   // 创建屏幕1
lv_scr_load(guider_ui.screen_1);  // 加载屏幕1（显示）
```

---

## 六、核心概念：对象/样式/事件/动画

### 6.1 对象（lv_obj）—— 万物皆对象

LVGL 里**一切控件都是对象**（按钮、标签、滑块...都是 lv_obj）。对象组成**父子树**：

```
screen（屏幕，根对象）
├── label（文字标签）
├── btn（按钮）
│   └── label（按钮上的文字）
└── slider（滑块）
```

**创建对象的两个参数**：`parent`（父对象）+ `copy`（复制模板，NULL=新建）：

```c
lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);   // 创建按钮，父=当前屏幕
lv_obj_t *label = lv_label_create(btn, NULL);        // 创建标签，父=按钮
```

**对象的父子关系决定**：
- **位置**：子对象位置相对父对象
- **显示**：父隐藏子也隐藏，父删除子也删除
- **事件**：点击子对象，父也能收到（事件冒泡）

### 6.2 样式（Style）—— v7 的样式系统（重点，v8 改了）

**v7 样式结构**：每个对象有"部件（part）+ 状态（state）"两个维度：

```c
// v7 设置样式：lv_obj_set_style_local_xxx(obj, part, state, value)
lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
//                            对象  部件=主体     状态=默认         颜色=红
```

**部件（PART）**：对象的不同部分

| 部件 | 含义 |
|------|------|
| `LV_OBJ_PART_MAIN` | 主体（背景/边框）|
| `LV_OBJ_PART_INDICATOR` | 指示器（进度条/滑块的值部分）|
| `LV_OBJ_PART_KNOB` | 旋钮（滑块圆点）|
| `LV_BAR_PART_BG/INDIC` | 进度条背景/指示 |

**状态（STATE）**：对象的不同状态

| 状态 | 含义 |
|------|------|
| `LV_STATE_DEFAULT` | 默认 |
| `LV_STATE_PRESSED` | 按下 |
| `LV_STATE_FOCUSED` | 聚焦 |
| `LV_STATE_CHECKED` | 选中（开关）|
| `LV_STATE_DISABLED` | 禁用 |

**项目真实例子（auto_disp.c 示意框）**：

```c
/* 浅灰色背景容器 */
lv_obj_set_style_local_bg_color(cont_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xE5E5E5));
lv_obj_set_style_local_bg_opa(cont_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
/* 去掉默认边框 */
lv_obj_set_style_local_border_width(cont_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
lv_obj_set_style_local_radius(cont_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
```

> **面试点**：v7 样式必须带 `part` 和 `state` 两个参数——这是 v7 和 v8 最大的语法差异（v8 合并成 selector）。

### 6.3 事件（Event）—— 交互的核心

**事件 = 用户操作触发回调**。LVGL 检测到触摸/按键 → 触发对应事件 → 调用你注册的回调。

**注册事件回调**：

```c
lv_obj_set_event_cb(ui->screen_1_slider_1, slider_1_event_handler);
```

**回调函数格式**（事件 + 对象指针）：

```c
static void slider_1_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)   // 滑块拖动时
    {
        int16_t pct = lv_slider_get_value(obj);   // 读当前值
        servo_target[1] = pct * 180 / 100;        // 做处理
        lv_label_set_text_fmt(label, "%d", servo_target[1]);   // 更新显示
    }
}
```

**常用事件**：

| 事件 | 触发时机 | 典型控件 |
|------|---------|---------|
| `LV_EVENT_CLICKED` | 点击完成（按下+松开）| 按钮 |
| `LV_EVENT_PRESSED` | 按下瞬间 | 按钮 |
| `LV_EVENT_RELEASED` | 松开瞬间 | 按钮 |
| `LV_EVENT_VALUE_CHANGED` | 值改变 | 滑块/进度条/下拉框 |
| `LV_EVENT_SCROLL_BEGIN` | 开始滚动 | 列表 |

**项目真实例子（机械臂按钮回调）**：

```c
static void btn_5_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        int16_t pct = lv_slider_get_value(guider_ui.screen_1_slider_1);  // 读滑块
        // 注意：读滑块要用 guider_ui 全局引用，不能用 obj（obj 是按钮自己！）
        ...
    }
}
```

> **经典坑**：按钮回调里的 `obj` 是**按钮本身**，不是滑块！读其他控件要用全局 `guider_ui` 引用。

### 6.4 动画（Animation）

**v7 动画**：`lv_anim_t` 结构体描述动画

```c
lv_anim_t a;
lv_anim_init(&a);
lv_anim_set_var(&a, obj);          // 动画对象
lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);   // 执行函数（改什么）
lv_anim_set_values(&a, 0, 100);    // 从 0 到 100
lv_anim_set_time(&a, 500);         // 500ms
lv_anim_start(&a);                 // 启动
```

**最常用的"动画"其实是进度条自带动画**：

```c
lv_bar_set_value(bar, value, LV_ANIM_ON);   // LV_ANIM_ON = 平滑过渡
```

### 6.5 隐藏/删除/对齐（v7 API）

```c
lv_obj_set_hidden(obj, true);            // 隐藏（v8 用 lv_obj_add_flag）
lv_obj_del(obj);                         // 删除
lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0);   // 居中（v7 中间参数是 NULL）
lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);  // 左上角
```

---

## 七、常用控件详解（项目实战）

### 7.1 标签 lv_label（显示文字）

```c
lv_obj_t *label = lv_label_create(parent, NULL);
lv_label_set_text(label, "你好");            // 静态文字
lv_label_set_text_fmt(label, "%d", 25);      // 格式化（整数）
lv_label_set_text_fmt(label, "%.2f", 3.14);  // 格式化（浮点，见坑）

/* 设置字体（中文要外部字库！） */
lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &Myfont_24);
lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x333333));
```

> **浮点坑**：v7 默认 `LV_SPRINTF_DISABLE_FLOAT=1`（禁用浮点），`lv_label_set_text_fmt("%.2f")` 打不出小数！解决：① 改成 0（费 RAM）② 用 `snprintf` 先格式化再 `lv_label_set_text`（项目做法）。

### 7.2 按钮 lv_btn（交互入口）

```c
lv_obj_t *btn = lv_btn_create(parent, NULL);
lv_obj_set_pos(btn, 50, 100);       // 位置
lv_obj_set_size(btn, 100, 50);      // 大小
lv_obj_set_event_cb(btn, btn_handler);   // 事件
lv_obj_t *label = lv_label_create(btn, NULL);   // 按钮文字
lv_label_set_text(label, "确定");
lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);   // 文字居中
```

### 7.3 滑块 lv_slider（调节值）

```c
lv_obj_t *slider = lv_slider_create(parent, NULL);
lv_slider_set_range(slider, 0, 100);       // 范围 0~100
lv_obj_set_event_cb(slider, slider_handler);

// 回调里：
static void slider_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED) {
        int16_t val = lv_slider_get_value(obj);   // 读当前值
        lv_slider_set_value(obj, val, LV_ANIM_OFF);  // 设值（不带动画）
    }
}
```

> **项目用法**：机械臂 5 个轴 = 5 个滑块（0~100 百分比 → 转 0~180 角度）。

### 7.4 进度条 lv_bar（显示占比）

```c
lv_obj_t *bar = lv_bar_create(parent, NULL);
lv_bar_set_range(bar, 0, 100);            // 范围
lv_bar_set_value(bar, 60, LV_ANIM_ON);    // 设值（带动画平滑）

/* 设置指示器颜色（v7） */
lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_GREEN);
```

> **项目用法**：空气质量检测仪 4 个进度条显示温度/VOC/甲醛/CO₂ 占比。

### 7.5 下拉框 lv_dropdown（选择）

```c
lv_obj_t *ddlist = lv_dropdown_create(parent, NULL);
lv_dropdown_set_options(ddlist, "组1\n组2\n组3");   // 选项用 \n 分隔
lv_obj_set_event_cb(ddlist, ddlist_handler);

// 回调里：
if (event == LV_EVENT_VALUE_CHANGED) {
    uint16_t sel = lv_dropdown_get_selected(obj);   // 选中索引 0/1/2
    current_group = sel;   // 切换动作组
}
```

> **项目用法**：机械臂动作组选择（组1/组2/组3）。

### 7.6 开关 lv_switch / 复选框 lv_checkbox

```c
lv_obj_t *sw = lv_switch_create(parent, NULL);
lv_switch_on(sw, LV_ANIM_ON);   // 打开

lv_obj_t *cb = lv_checkbox_create(parent, NULL);
lv_checkbox_set_text(cb, "选项");
```

---

## 八、GUI Guider 图形化设计（重点）

> **GUI Guider = NXP 官方的 LVGL 图形化设计工具**——拖拽控件生成代码，不用手写界面布局。项目界面都是这么做的。

### 8.1 GUI Guider 工作流

```
① GUI Guider 里拖控件（按钮/滑块/标签/进度条...）
② 设置控件属性（位置/大小/颜色/文字/事件）
③ 导出代码（生成 setup_scr_screen_x.c + gui_guider.h + events_init.c）
④ Keil 工程加入生成文件
⑤ 手写事件回调（events_init.c）
⑥ 主任务里 setup_ui(&guider_ui) 加载
```

### 8.2 生成的文件结构

| 文件 | 内容 |
|------|------|
| `gui_guider.h` | `guider_ui` 结构体（所有控件的指针）+ 函数声明 |
| `setup_scr_screen_x.c` | 每个屏幕的创建函数（建控件+设属性）|
| `events_init.c` | 事件回调框架（空函数，手写填充）|
| `generated/images/` | 生成的图片资源 |
| `guider_fonts/` | 生成的字体 |

### 8.3 `guider_ui` 结构体（核心）

```c
// gui_guider.h（简化）
typedef struct {
    lv_obj_t *screen;           // 屏幕1
    lv_obj_t *screen_btn_1;     // 屏幕1的按钮1
    lv_obj_t *screen_1_slider_1;// 屏幕1的滑块1
    lv_obj_t *screen_1_label_8; // 屏幕1的标签8
    ...
} lv_ui;

extern lv_ui guider_ui;   // 全局实例
```

**访问任何控件**：`guider_ui.screen_1_slider_1` —— 这就是为什么事件回调里能跨控件操作。

### 8.4 控件命名规则

```
screen_btn_1        = 屏幕 screen 上的按钮 btn_1
screen_1_slider_2   = 屏幕 screen_1 上的滑块 slider_2
screen_2_btn_3      = 屏幕 screen_2 上的按钮 btn_3
```

> 命名规则：`{屏幕名}_{控件类型}_{编号}`。看懂命名就能快速定位控件。

### 8.5 手写事件回调（项目真实代码）

GUI Guider 生成 `events_init.c` 的**空回调框架**，你在里面填充逻辑：

```c
// events_init.c（机械臂项目）
static void btn_5_event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        int16_t pct = lv_slider_get_value(guider_ui.screen_1_slider_1);
        if (pct < 100) {
            pct++;
            int16_t angle = pct * 180 / 100;          // 百分比 → 角度
            servo_target[1] = angle;                   // 更新控制目标
            lv_slider_set_value(guider_ui.screen_1_slider_1, pct, LV_ANIM_OFF);
            lv_label_set_text_fmt(guider_ui.screen_1_label_8, "%d", angle);
        }
    }
}

// 注册回调（events_init_screen_1 里）
void events_init_screen_1(lv_ui *ui)
{
    lv_obj_set_event_cb(ui->screen_1_slider_1, slider_1_event_handler);
    lv_obj_set_event_cb(ui->screen_1_btn_5, btn_5_event_handler);
    ...
}
```

### 8.6 屏幕切换（lv_scr_load / lv_scr_load_anim）

```c
/* 直接切换（无动画） */
lv_scr_load(guider_ui.screen_1);

/* 带动画切换（v7 最后一个参数 autodel=是否自动删除旧屏幕） */
lv_scr_load_anim(guider_ui.screen_1, LV_SCR_LOAD_ANIM_NONE, 1000, 1000, false);
```

> **autodel 参数坑（机械臂踩过）**：`autodel=true` 会在动画结束后**删除旧屏幕**。如果旧屏幕上有全局指针（如 AutoDisp 的控件），删除后指针变野指针 → 再访问就卡死。解决：要么 `autodel=false`（不删除），要么删除前把指针置 NULL。

### 8.7 GUI Guider 的局限与手动补充

GUI Guider 擅长**静态布局**，动态逻辑（事件回调、定时更新、动态创建）需要手写：

| 场景 | 做法 |
|------|------|
| 静态界面 | GUI Guider 拖拽 |
| 事件回调 | events_init.c 手写 |
| 定时刷新数据 | 定时器/任务里改 label |
| 动态创建控件 | 代码里 `lv_obj_create`（项目 AutoDisp 做法）|
| 中文字体 | GUI Guider 加字库 或 外部 XBF 字库 |

---

## 九、配置裁剪（lv_conf.h）（重点）

> **lv_conf.h = LVGL 的"总开关"**。LVGL 功能非常多（几十种控件、动画、图片、字体...），但你的项目只用其中几个。**关掉不用的 → 代码体积小、RAM 省**。对 64KB RAM 的 STM32F1 来说，裁剪是必须的。

### 9.1 先看项目实际配置

**空气质量检测仪 lv_conf.h 关键项**：

```c
#define LV_HOR_RES_MAX    (320)   // 屏幕宽（LVGL 认为的最大宽）
#define LV_VER_RES_MAX    (240)   // 屏幕高
#define LV_COLOR_DEPTH    16      // 颜色深度：RGB565（2字节/像素）
#define LV_MEM_SIZE       (20U * 1024U)   // LVGL 堆大小 20KB
#define LV_SPRINTF_DISABLE_FLOAT 0        // 允许浮点格式化（打小数）
```

### 9.2 颜色深度（LV_COLOR_DEPTH）—— 内存大头

| 深度 | 每像素 | 800×480 缓冲 | 色彩 |
|------|--------|------------|------|
| 16（RGB565）| 2 字节 | 16KB(10行) | 65536 色 |
| 32（ARGB8888）| 4 字节 | 32KB(10行) | 1670 万色 |

**选 16 的理由**：RAM 减半 + 你的 LCD 本身就是 16 位接口。32 位只在高端屏用。

### 9.3 LVGL 堆大小（LV_MEM_SIZE）—— 控件内存来源

**所有控件/动画/样式都从 LVGL 堆分配**。太小 → 控件创建失败（卡死/花屏）；太大 → 挤占其他内存。

```
LV_MEM_SIZE 的选择：
  控件多（几十个）→ 20~30KB
  控件少（几个）  → 10~16KB
  不足 → vApplicationMallocFailedHook 触发 / LVGL 断言失败
```

**查看堆剩余（调试）**：

```c
// v7 写法：
lv_mem_monitor_t mon;
lv_mem_monitor(&mon);
printf("LVGL堆剩余: %d\r\n", (int)mon.free_size);

// v8 写法：
printf("LVGL堆剩余: %d\r\n", (int)lv_mem_get_free());
```

### 9.4 控件裁剪（LV_USE_XXX）—— 代码体积

lv_conf.h 里每个控件都有开关，**不用就关**：

```c
#define LV_USE_BTN          1   // 按钮：用
#define LV_USE_LABEL        1   // 标签：用
#define LV_USE_SLIDER       1   // 滑块：用
#define LV_USE_BAR          1   // 进度条：用
#define LV_USE_DROPDOWN     1   // 下拉框：用
#define LV_USE_SWITCH       0   // 开关：不用 → 关！
#define LV_USE_CHECKBOX     0   // 复选框：不用 → 关！
#define LV_USE_CHART        0   // 图表：不用 → 关！
#define LV_USE_TABLE        0   // 表格：不用 → 关！
#define LV_USE_CALENDAR     0   // 日历：不用 → 关！
#define LV_USE_MSGBOX       0   // 消息框：不用 → 关！
```

**每个关闭的控件 = 省几 KB 代码**。比如日历控件一个就占好几 KB Flash。

### 9.5 功能裁剪（LV_USE_XXX 全局功能）

```c
#define LV_USE_ANIMATION    1   // 动画：进度条过渡要用 → 开
#define LV_USE_GROUP        1   // 分组（按键导航）：触摸屏不用 → 可关
#define LV_USE_GPU          0   // GPU 加速：没有 GPU → 关！
#define LV_USE_LABEL_LONG_TXT_DOT  1   // 超长文字省略号：可关
#define LV_USE_THEME        1   // 主题：默认主题，要开
#define LV_USE_FONT_...     0   // 不用的字体全关（内置字体也占空间）
```

### 9.6 字体裁剪（LV_FONT_XXX）—— 重要

**LVGL 内置字体（montserrat）只有 ASCII**，中文必须自己加字库：

```c
#define LV_FONT_MONTSERRAT_12    1   // 12号：小字
#define LV_FONT_MONTSERRAT_14    1   // 14号：常用
#define LV_FONT_MONTSERRAT_16    0   // 16号：不用 → 关！
#define LV_FONT_MONTSERRAT_20    0   // 20号：不用 → 关！
#define LV_FONT_MONTSERRAT_28    0   // 28号：不用 → 关！
```

**每个内置字体 ≈ 5~10KB Flash**，全开很浪费。

### 9.7 中文字库方案（两种，项目都接触过）

**方案 A：程序内嵌（Myfont_24.c）**

```c
// Lvgl Font Tool 生成的字模数组，编译进程序
lv_font_t Myfont_24 = { ... };   // 内嵌，不占运行 RAM，但占 Flash
```

**方案 B：外部 XBF 字库（W25Q64 存）** ← 空气质量检测仪做法

```c
// 字库 bin 文件烧进 W25Q64，LVGL 按需从 Flash 读字模
#define FONT_24_Addr  0x0EF000    // 24号字库起始地址
static uint8_t *__user_font_getdata(int offset, int size) {
    sFLASH_ReadBuffer(__g_font_buf, FONT_24_Addr + offset, size);
    return __g_font_buf;
}
```

**选哪个**：
- 字少（几个屏幕）→ 内嵌，简单
- 字多/界面多 → 外部字库省 Flash（中文字库几百 KB 内嵌放不下）

### 9.8 裁剪前后对比（经验值）

| 项 | 全开 | 裁剪后 |
|----|------|--------|
| 代码体积 | 300+KB | ~120KB |
| LVGL 堆 | 需 30KB | 20KB 够 |
| 显示缓冲 | 16KB | 16KB（不变）|

> **面试点**：能讲"为什么关掉不用的控件/字体" = 理解嵌入式资源意识。这是嵌入式工程师和"会调 API"的程序员的区别。

---

## 十、内存管理与性能优化

### 10.1 LVGL 内存模型

```
LVGL 堆（LV_MEM_SIZE，20KB）
  ├── 控件对象（lv_obj_t，几十~几百字节/个）
  ├── 动画（lv_anim_t）
  ├── 样式
  └── 文本缓冲
```

### 10.2 内存泄漏（常见 bug）

**控件反复创建不删除 = 泄漏**：

```c
// ❌ 错误：每次收帧都创建新控件
void update(void) {
    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);  // 每次都新建！
    lv_label_set_text(label, "数据");
}
// 问题：创建了几百个 label，堆耗尽 → 卡死

// ✅ 正确：创建一次，只改属性
static lv_obj_t *label = NULL;
void update(void) {
    if (label == NULL) label = lv_label_create(lv_scr_act(), NULL);  // 只建一次
    lv_label_set_text(label, "数据");   // 之后只改内容
}
```

**项目教训**：AutoDisp 模块早期就是反复创建控件导致 LVGL 堆耗尽（14780→14664 递减）——后来改成"创建一次 + 防重复判断"解决。

### 10.3 性能优化技巧

| 技巧 | 效果 |
|------|------|
| 减少控件数 | 渲染快（每控件都要遍历）|
| 用局部刷新 | LVGL 自动只刷脏区域（不用管）|
| 双缓冲 + DMA | 渲染/刷新并行 |
| 减少大图 | 图片解码耗内存耗时间 |
| 少用动画 | 动画耗 CPU |
| 提高任务优先级 | LVGL 任务不被饿死 |

### 10.4 常见崩溃排查

| 现象 | 原因 | 排查 |
|------|------|------|
| 白屏 | LV_MEM_SIZE 太小 / 控件创建失败 | 调大堆 + 检查 malloc 钩子 |
| 卡死 | 缺少 flush_ready / 线程不安全 | 查 disp_flush / 集中 LVGL 操作 |
| 花屏 | 缓冲/刷屏时序 | 逐行刷 / 查排线 |
| 断言失败 | 传了 NULL 对象 | 查指针是否有效（屏幕被删）|
| 内存递减 | 控件反复创建 | 创建一次 + 复用 |

---

## 十一、v7 vs v8 差异对比

### 11.1 样式系统（最大差异）

```c
// v7：part + state 两个参数
lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

// v8：selector 合并（位或）
lv_obj_set_style_bg_color(obj, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
// 简写：lv_obj_set_style_bg_color(obj, color, 0);  ← 0 = 默认 part+state
```

### 11.2 常用 API 差异表

| 功能 | v7 | v8 |
|------|----|----|
| 隐藏 | `lv_obj_set_hidden(obj, true)` | `lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN)` |
| 显示 | `lv_obj_set_hidden(obj, false)` | `lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN)` |
| 对齐 | `lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0)` | `lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0)` |
| 对齐枚举 | `LV_ALIGN_IN_TOP_LEFT` | `LV_ALIGN_TOP_LEFT` |
| 堆剩余 | `lv_mem_monitor(&mon)` | `lv_mem_get_free()` |
| 清除样式 | `lv_obj_reset_style_list(obj, part)` | `lv_obj_remove_style_all(obj)` |
| 事件回调 | `lv_obj_set_event_cb(obj, cb)` | `lv_obj_add_event_cb(obj, cb, LV_EVENT_ALL, NULL)` |
| 坐标 | `lv_obj_get_x(obj)` | `lv_obj_get_x_aligned(obj)` |

### 11.3 屏幕切换差异

```c
// v7：
lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 500, 0, false);
// v8：没有 autodel 参数了
lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 500, 0, false);
```

> **判断版本的方法**：看编译报错，或查 `lv_conf.h` 顶部的版本号。

---

## 十二、踩坑记录（项目真实经验）

| # | 坑 | 原因 | 解决 |
|---|----|------|------|
| 1 | 全屏写入花屏 | 排线信号不稳定（大量连续写）| disp_flush 逐行写 |
| 2 | 界面卡死 | 缺 `lv_disp_flush_ready` | 补上 |
| 3 | 浮点显示乱码 | `LV_SPRINTF_DISABLE_FLOAT=1` | snprintf 先格式化 |
| 4 | 中文乱码 | 默认字体无中文 | 加中文字库（内嵌/XBF）|
| 5 | 反复进出页面卡死 | `autodel=true` 删屏后指针野指针 | autodel=false 或置 NULL |
| 6 | LVGL 堆耗尽 | 控件反复创建 | 创建一次 + 防重复 |
| 7 | 点击位置错 | 触摸坐标没按屏幕方向映射 | touchpad_read 里转换 |
| 8 | 动画不转 | tick 没喂 / 喂的不对 | vApplicationTickHook 喂 lv_tick_inc |
| 9 | 跨任务调 LVGL 花屏 | 线程不安全 | 集中到 LVGL 任务 + 标志位 |
| 10 | 按钮回调读错控件 | 回调里 obj 是按钮自己 | 用 guider_ui 全局引用 |

---

## 十三、问答

### Q1：LVGL 怎么移植的？

> 三步：① 初始化显示（LCD 初始化）；② 分配显示缓冲（部分缓冲，RAM 限制）；③ 实现 disp_flush（把 LVGL 渲染的像素刷到屏幕）。输入侧实现 touchpad_read（读触摸坐标）。再加 tick 时钟和周期调用 lv_task_handler。

### Q2：为什么用部分缓冲不用全屏缓冲？

> 全屏 800×480×2 = 768KB，STM32 只有 64KB RAM 放不下。部分缓冲（如 10 行）让 LVGL 分块渲染，代价是慢一点但能跑。

### Q3：disp_flush 里必须做什么？

> 把 color_p 的像素按 area 区域写到屏幕，然后**必须调用 lv_disp_flush_ready** 通知 LVGL 缓冲可复用。不调用会卡死。

### Q4：LVGL 线程安全怎么处理？

> LVGL 不是线程安全的。做法：所有 LVGL 操作集中在单独任务（LVGL 任务），其他任务只置标志位请求更新，LVGL 任务消费标志。

### Q5：中文字库怎么解决？

> 默认字体只有 ASCII。方案：① GUI Guider/工具生成中文字体内嵌（占 Flash）；② XBF 外部字库存 SPI Flash，LVGL 回调按需读取（省 Flash）。按字量选。

### Q6：怎么裁剪 LVGL 减小体积？

> lv_conf.h 里关掉不用的控件（LV_USE_XXX=0）、不用的字体（LV_FONT_XXX=0）、不用的功能（GPU/动画等）。每个控件/字体几 KB，全关能省一半以上。

### Q7：v7 和 v8 有什么区别？

> 最大差异是样式系统：v7 用 `lv_obj_set_style_local_xxx(obj, part, state, val)`，v8 合并成 selector。其他还有隐藏控件、对齐、事件 API 的差异。

### Q8：LVGL 内存不够怎么排查？

> 用 lv_mem_monitor 看堆剩余。如果递减说明泄漏（控件反复创建）。调大 LV_MEM_SIZE 或优化代码。

### Q9：GUI Guider 和手写代码什么关系？

> GUI Guider 负责静态界面布局（拖拽生成 setup_scr_screen_x.c），动态逻辑（事件回调、定时刷新）手写 events_init.c。生成的 guider_ui 结构体保存所有控件指针，代码通过它访问控件。

### Q10：有没有踩过什么坑？

> ① 花屏——逐行刷解决；② 浮点显示——snprintf 替代；③ autodel 删屏野指针——改 false；④ 控件反复创建泄漏——创建一次复用。这些都是真实调试经历。

*文档整理日期: 2026-08-07*