# JSON 与 cJSON 学习笔记（STM32 实战版）

> 本笔记结合空气质量检测仪项目（ESP8266 + OneNET 上云），讲解 JSON 语法、为什么用 cJSON、如何在 STM32 工程中移植 cJSON（FreeRTOS 堆重定向）、生成/解析实战。
> 配套源码：`cJSON源码/cJSON-1.7.18/`（GitHub DaveGamble/cJSON v1.7.18，含 LICENSE）

---

## 目录

1. [JSON 是什么](#一json-是什么)
2. [JSON 语法规则](#二json-语法规则)
3. [为什么用 cJSON（对比手写解析）](#三为什么用-cjson对比手写解析)
4. [cJSON 源码结构](#四cjson-源码结构)
5. [在 STM32 工程中移植 cJSON](#五在-stm32-工程中移植-cjson)
6. [cJSON 核心 API](#六cjson-核心-api)
7. [cJSON 生成 JSON（项目实战）](#七cjson-生成-json项目实战)
8. [cJSON 解析 JSON（项目实战）](#八cjson-解析-json项目实战)
9. [cJSON 内存管理（重点）](#九cjson-内存管理重点)
10. [手写解析 vs cJSON 对比](#十手写解析-vs-cjson-对比)
11. [踩坑记录](#十一踩坑记录)
12. [问答](#十二问答)

---

## 一、JSON 是什么

**JSON（JavaScript Object Notation）** = 比 XML 更简单的一种**数据交换格式**，用完全独立于编程语言的文本格式来存储和表示数据。

**为什么物联网/嵌入式用 JSON**：
- **人可读**：串口打印出来直接能看懂
- **平台标准**：OneNET 物模型接口定义字段，设备按格式上报，平台自动解析
- **语言无关**：C/Python/JS 都有现成解析库

**JSON 能表示的数据类型**：字符串、数字、对象、数组、布尔、null。

---

## 二、JSON 语法规则

### 规则 1：键值对（key:value）

```json
{
  "student_num": 99
}
```

### 规则 2：逗号分隔多条数据

```json
{
  "student_count": 99,
  "student_age": 18
}
```

### 规则 3：花括号 {} 表示对象（可嵌套）

```json
{
  "value1": 1111,
  "value2": "hello",
  "value3": {
    "value4": "asdasd",
    "value5": "asdsadsa"
  }
}
```

### 规则 4：方括号 [] 表示数组

```json
{
  "name": "XiaoHong",
  "age": [1, 2, 3, 4, 5, 6, 7, 8, 9]
}
```

### 项目实际例子（OneNET 物模型上报）

```json
{
  "id": "2",
  "version": "1.0",
  "params": {
    "temp":  {"value": 25.3},
    "hum":   {"value": 60},
    "VOC":   {"value": 0.12},
    "CO2":   {"value": 800},
    "Switch": {"value": 1}
  }
}
```

---

## 三、为什么用 cJSON（对比手写解析）

### 3.1 手写解析的问题（cjson.txt 原话提炼）

```
之前我去解析 JSON，完全不靠任何外部库，纯 C 语言自己写！
p -> json 字符串
```

手写解析的三大问题：

| 问题 | 说明 |
|------|------|
| **不专业** | 自己写的死循环解析，别人看着"心里不舒服" |
| **有危险** | 字符串边界、嵌套层级、转义处理容易出 bug |
| **无通用性** | 换一种 JSON 格式就要重写解析逻辑 |

### 3.2 cJSON 的优势

| 优势 | 说明 |
|------|------|
| **效率高** | 比自己写的死循环解析效率高 |
| **代码稳定** | 全球广泛使用的成熟库（MIT 协议）|
| **通用性强** | 适用于很多不同格式的 JSON 解析 |
| **零依赖** | 只有 cJSON.c + cJSON.h 两个文件，直接编译进工程 |

---

## 四、cJSON 源码结构

```
cJSON源码/cJSON-1.7.18/
├── cJSON.c      # 实现文件（约 79KB，2000+ 行）
├── cJSON.h      # 头文件（API 声明 + cJSON 结构体定义）
├── LICENSE      # MIT 协议
└── README.md    # 官方说明
```

> **注意**：cJSON_Utils.c/h（JSON 指针/补丁功能）是可选扩展，一般用不到，STM32 工程**只加 cJSON.c/cJSON.h** 即可。

### cJSON 的核心结构体（cJSON.h 里）

```c
typedef struct cJSON {
    struct cJSON *next;          /* 链表：下一个兄弟节点 */
    struct cJSON *prev;          /* 链表：上一个兄弟节点 */
    struct cJSON *child;         /* 子节点（对象/数组的第一个元素）*/

    int type;                    /* 类型：对象/数组/字符串/数字/布尔/null */

    char *valuestring;           /* 字符串值 */
    int valueint;                /* 整数值（旧字段，推荐用 valuedouble）*/
    double valuedouble;          /* 双精度浮点值 */

    char *string;                /* 键名（对象的字段名）*/
} cJSON;
```

**核心理解：cJSON 用"双向链表 + 树"结构存储 JSON**：

```
root（对象）
 ├── child → "id" → next → "version" → next → "params"
 │                                          └── child → "temp" → next → "hum" ...
 └── （链表横向 = 兄弟字段，child 纵向 = 嵌套层级）
```

---

## 五、在 STM32 工程中移植 cJSON

### 5.1 步骤 1：把文件加入工程

1. 把 `cJSON.c`、`cJSON.h` 复制到工程目录（如 `user/API/cjson/`）
2. Keil 里把 `cJSON.c` 加入工程（App 分组）
3. 头文件路径加 `user/API/cjson/`

### 5.2 步骤 2：内存重定向到 FreeRTOS 堆（关键！）

**问题**：cJSON 默认用 C 库的 `malloc/free`。在 STM32 上：
- 裸机：C 库堆由启动文件 `Heap_Size` 决定（通常 512 字节，**不够用**！）
- FreeRTOS：建议用 `pvPortMalloc/pvPortFree`（用 FreeRTOS 的堆）

**解决方案**：`cJSON_InitHooks` 重定向：

```c
#include "cJSON.h"
#include "FreeRTOS.h"      // pvPortMalloc / pvPortFree

/* 在 main() 里、使用 cJSON 之前调用一次 */
void CJSON_Init(void)
{
    cJSON_Hooks hooks;
    hooks.malloc_fn = pvPortMalloc;   /* 用 FreeRTOS 堆 */
    hooks.free_fn  = pvPortFree;
    cJSON_InitHooks(&hooks);
}
```

**注意**：
1. **必须在创建任何 cJSON 对象之前调用**（否则之前的对象还是 C 库堆，后面释放会混）
2. `pvPortMalloc/pvPortFree` 必须在调度器启动后使用（或确认 heap_4 可用）
3. FreeRTOS 的 `configTOTAL_HEAP_SIZE` 要留足（见内存管理章节）

### 5.3 步骤 3：验证移植

```c
/* 简单测试：解析一段 JSON */
cJSON *root = cJSON_Parse("{\"a\": 123}");
if (root != NULL) {
    cJSON *a = cJSON_GetObjectItem(root, "a");
    printf("a = %d\r\n", (int)a->valuedouble);
    cJSON_Delete(root);   /* 必须释放！ */
}
```

---

## 六、cJSON 核心 API

### 6.1 创建与挂载（生成 JSON）

| 函数 | 作用 |
|------|------|
| `cJSON_CreateObject()` | 创建对象 `{}` |
| `cJSON_CreateArray()` | 创建数组 `[]` |
| `cJSON_CreateString(s)` | 创建字符串节点 |
| `cJSON_CreateNumber(d)` | 创建数字节点 |
| `cJSON_CreateBool(b)` | 创建布尔节点 |
| `cJSON_AddItemToObject(obj, "name", child)` | 子节点挂到对象 |
| `cJSON_AddItemToArray(arr, child)` | 子节点追加到数组 |
| `cJSON_AddStringToObject(obj, "name", s)` | 快捷：建字符串+挂载 |
| `cJSON_AddNumberToObject(obj, "name", d)` | 快捷：建数字+挂载 |
| `cJSON_AddBoolToObject(obj, "name", b)` | 快捷：建布尔+挂载 |

### 6.2 序列化（树 → 字符串）

| 函数 | 作用 | 说明 |
|------|------|------|
| `cJSON_Print(root)` | 带缩进格式化 | 可读性好，字节多 |
| `cJSON_PrintUnformatted(root)` | 无空格紧凑 | **上报用这个**，省字节 |
| `cJSON_PrintBuffered(root, len, fmt)` | 输出到指定缓冲 | 可控内存（大 RAM 限制时用）|

> **返回值是 malloc 出来的字符串，用完必须 free()！**

### 6.3 解析（字符串 → 树）

| 函数 | 作用 |
|------|------|
| `cJSON_Parse(str)` | 解析 JSON 字符串为树，失败返回 NULL |
| `cJSON_GetObjectItem(obj, "name")` | 取对象里的字段节点 |
| `cJSON_GetArraySize(arr)` | 取数组长度 |
| `cJSON_GetArrayItem(arr, i)` | 取数组第 i 个元素 |
| `cJSON_Delete(root)` | 释放整棵树（递归）|

### 6.4 取值（v1.7+ 推荐用 IsXXX 判断）

| 函数 | 作用 |
|------|------|
| `cJSON_IsNumber(item)` | 判断是否为数字 |
| `cJSON_IsString(item)` | 判断是否为字符串 |
| `cJSON_IsObject(item)` | 判断是否为对象 |
| `cJSON_IsArray(item)` | 判断是否为数组 |
| `cJSON_IsBool(item)` | 判断是否为布尔 |
| `item->valuedouble` | 数字值 |
| `item->valuestring` | 字符串值 |
| `cJSON_IsTrue(item)` / `cJSON_IsFalse(item)` | 布尔判断 |

---

## 七、cJSON 生成 JSON（项目实战）

### 7.1 目标：生成 OneNET 物模型上报数据

```json
{
  "id": "2",
  "version": "1.0",
  "params": {
    "temp":  {"value": 25.3},
    "hum":   {"value": 60},
    "VOC":   {"value": 0.12},
    "CHO2":  {"value": 0.05},
    "CO2":   {"value": 800},
    "temp_max": {"value": 30.0},
    "Switch":   {"value": 1}
  }
}
```

### 7.2 完整实现（替换项目里的 snprintf 手拼）

```c
/**
 * @brief 使用 cJSON 构建 MQTT 物模型上报数据
 * @param buf   输出缓冲区（接收 JSON 字符串）
 * @param size  缓冲区大小（建议 ≥ 300 字节）
 * @return int  JSON 字符串长度；失败返回 -1
 */
int MQTT_BuildPayload_cJSON(char *buf, uint16_t size)
{
    cJSON *root   = NULL;   /* 根对象 */
    cJSON *params = NULL;   /* params 子对象 */
    char *str     = NULL;
    int len       = -1;

    /* 1. 创建根对象 */
    root = cJSON_CreateObject();
    if (root == NULL) return -1;           /* 内存不足 */

    /* 2. 添加字符串字段 */
    cJSON_AddStringToObject(root, "id", "2");
    cJSON_AddStringToObject(root, "version", "1.0");

    /* 3. 创建 params 子对象并挂载 */
    params = cJSON_CreateObject();
    if (params == NULL) {
        cJSON_Delete(root);                /* 失败要释放已创建的 */
        return -1;
    }
    cJSON_AddItemToObject(root, "params", params);
    /* AddItemToObject 之后 params 归 root 管，不要再单独 Delete */

    /* 4. 物模型字段：每个属性是 {"value": x} 对象 */
    #define ADD_PROP(obj, name, val)               \
        do {                                       \
            cJSON *o = cJSON_CreateObject();       \
            cJSON_AddNumberToObject(o, "value", (val)); \
            cJSON_AddItemToObject(obj, (name), o); \
        } while (0)

    ADD_PROP(params, "temp",     Dht.Tem);
    ADD_PROP(params, "hum",      Dht.Hum);
    ADD_PROP(params, "VOC",      Sensor.VOC);
    ADD_PROP(params, "CHO2",     Sensor.CH2O);
    ADD_PROP(params, "CO2",      Sensor.CO2);
    ADD_PROP(params, "temp_max", temp_max);
    ADD_PROP(params, "Switch",   1);

    /* 5. 序列化成紧凑字符串 */
    str = cJSON_PrintUnformatted(root);
    if (str == NULL) {
        cJSON_Delete(root);
        return -1;
    }

    /* 6. 拷贝到调用者缓冲区（带长度检查） */
    len = strlen(str);
    if (len < size) {
        memcpy(buf, str, len + 1);   /* +1 带上结尾 '\0' */
    } else {
        len = -1;                    /* 缓冲区不够 */
    }

    /* 7. 释放：字符串 + 整棵树（顺序不能反） */
    free(str);          /* PrintUnformatted 申请的内存 */
    cJSON_Delete(root); /* 递归释放整棵树 */

    return len;
}
```

### 7.3 在 MQTT_Publish_Post 里使用

```c
void MQTT_Publish_Post(void)
{
    int len;
    int payloadlen;
    char payload[300] = {0};
    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = PUB_Topic_Post;

    /* 用 cJSON 构建 payload（替换原来的 snprintf） */
    payloadlen = MQTT_BuildPayload_cJSON(payload, sizeof(payload));
    if (payloadlen <= 0) {
        printf("payload 构建失败\r\n");
        return;
    }

    len = MQTTSerialize_publish(Esp.S_Buff, ESP_S_Buff_Length, 0, 0, 0, 0,
                                topicString, (unsigned char*)payload, payloadlen);
    if (len <= 0) {
        printf("发布报文打包失败\r\n");
        return;
    }

    ESP_Clear_R_Buff();
    UART1_SendBuff(Esp.S_Buff, len);   /* 调试用 */
    ESP_SendBuff(Esp.S_Buff, len);     /* 发给 ESP8266 转发 */
}
```

---

## 八、cJSON 解析 JSON（项目实战）

### 8.1 场景：解析平台下发的 property/set 指令

平台下发（MQTT 报文里的 JSON）：

```json
{"id":"6","version":"1.0","params":{"temp_max":69.4,"Switch_LED":0}}
```

### 8.2 用 cJSON 解析（替换项目里的 strstr + sscanf）

```c
/**
 * @brief 解析 OneNET 下行指令（property/set 的 JSON 数据）
 * @param json_str 收到的 JSON 字符串（注意要跳过 MQTT 报文头！）
 */
void Parse_Platform_Command(char *json_str)
{
    cJSON *root = NULL;
    cJSON *params = NULL;
    cJSON *item = NULL;

    root = cJSON_Parse(json_str);
    if (root == NULL) {
        printf("JSON 解析失败\r\n");
        return;
    }

    /* 1. 取 params 对象 */
    params = cJSON_GetObjectItem(root, "params");
    if (params == NULL) {
        cJSON_Delete(root);
        return;
    }

    /* 2. 解析温度阈值 temp_max */
    item = cJSON_GetObjectItem(params, "temp_max");
    if (cJSON_IsNumber(item)) {
        temp_max = item->valuedouble;         /* 直接取数值！ */
        printf("温度阈值 = %.1f\r\n", temp_max);
    }

    /* 3. 解析开关指令 Switch_LED */
    item = cJSON_GetObjectItem(params, "Switch_LED");
    if (cJSON_IsNumber(item)) {
        if (item->valuedouble == 0) {
            LED1_OFF(); LED2_OFF(); LED3_OFF(); LED4_OFF();   /* 关灯 */
        } else {
            LED1_ON();  LED2_ON();  LED3_ON();  LED4_ON();    /* 开灯 */
        }
    }

    cJSON_Delete(root);   /* 必须释放！ */
}
```

### 8.3 在 vTaskNETCode 里接入

```c
/* main.c vTaskNETCode 里，收到平台下行数据后： */
if (Esp.R_Buff[0] == 0x30) {   /* MQTT PUBLISH 报文 */
    /* 偏移 3 字节跳过 MQTT 报文头（主题长度2字节+主题名长度1字节） */
    Parse_Platform_Command((char *)&Esp.R_Buff[3]);
}
```

> **注意**：JSON 字符串里不能有 0x00 截断问题——cJSON_Parse 要求传入的字符串以 '\0' 结尾，MQTT 报文头的 0x00 字节要跳过（偏移 3），和之前 strstr 的处理一样。

---

## 九、cJSON 内存管理（重点）

### 9.1 内存从哪来

| 环境 | 内存来源 | 说明 |
|------|---------|------|
| 裸机 | C 库堆（启动文件 Heap_Size）| 默认 512B~1KB，**通常不够** |
| FreeRTOS | pvPortMalloc（configTOTAL_HEAP_SIZE）| 需用 cJSON_InitHooks 重定向 |

### 9.2 内存占用估算

| 内容 | 占用 |
|------|------|
| 每个 cJSON 节点结构体 | ~40 字节 |
| 每个字段名（string）| 名字长度 + 1 |
| 每个字符串值（valuestring）| 值长度 + 1 |
| 7 字段上报的整棵树 | **约 300~500 字节** |

**经验**：临时创建→用完删除，峰值内存 ≈ 树大小 + 序列化字符串，控制在 1KB 内。

### 9.3 必须成对释放（泄漏源头）

```c
/* ✅ 正确：成对释放 */
char *str = cJSON_PrintUnformatted(root);
// ... 使用 str ...
free(str);            /* ① 释放序列化字符串 */
cJSON_Delete(root);   /* ② 释放整棵树 */

/* ❌ 错误：只释放树，忘了 str → 泄漏 */
cJSON_Delete(root);
// str 没释放 → 每次上报泄漏几百字节 → 跑几天内存耗尽死机
```

### 9.4 内存碎片

频繁创建/删除大 JSON 树会产生碎片（FreeRTOS heap_4 有合并机制，但碎片仍会累积）。建议：
- **上报频率别太高**（几秒一次没问题）
- 必要时用 `cJSON_PrintBuffered` 复用固定缓冲
- 定期打印 `xPortGetFreeHeapSize()` 监控

---

## 十、手写解析 vs cJSON 对比

| 对比项 | 手写（snprintf/strstr）| cJSON |
|--------|----------------------|-------|
| 代码量 | 少（不用加文件）| +2 个文件（~80KB 源码）|
| RAM 占用 | 少（无动态分配）| 多（每个节点 malloc）|
| 格式正确性 | 手动拼引号/逗号，易错 | 自动处理，不会错 |
| 嵌套结构 | 很难处理 | 天然支持 |
| 数组支持 | 手动循环 | GetArrayItem 直接取 |
| 通用性 | 换格式重写 | 换格式不用改库 |
| 适合场景 | 字段固定、RAM 极紧张 | 字段多变、结构复杂 |

**项目选型建议**：
- 上行固定 7 字段 + RAM 紧张 → **手拼够用**（现状）
- 下行解析 + 字段可能增加 → **cJSON 更稳**
- 若 FreeRTOS 堆充足（>5KB 空闲）→ 全换 cJSON

---

## 十一、踩坑记录

| # | 坑 | 原因 | 解决 |
|---|----|------|------|
| 1 | 解析返回 NULL | JSON 字符串被 0x00 截断（MQTT 报文头）| 偏移 3 字节再 Parse |
| 2 | 内存耗尽死机 | Print 的字符串没 free | str 和 root 成对释放 |
| 3 | 崩溃/HardFault | malloc/free 混用（部分重定向）| 所有 cJSON 操作前统一 InitHooks |
| 4 | 值取出来不对 | 用了废弃的 valueint（大数溢出）| 用 valuedouble |
| 5 | 调度器启动前用 pvPortMalloc | FreeRTOS 堆未初始化 | 等调度器启动后再用 cJSON |
| 6 | 字段取不到 | 没先取父对象（直接取嵌套字段）| 先 GetObjectItem 父，再取子 |

---

## 十二、问答

### Q1：JSON 是什么？为什么物联网用 JSON？

> JSON 是轻量级文本数据交换格式，键值对 + 对象 + 数组。物联网平台（OneNET 物模型）定义了 JSON 字段格式，设备按格式上报，平台自动解析。人可读、语言无关。

### Q2：为什么用 cJSON 而不是手写解析？

> 手写解析不通用（换格式要重写）、有边界/转义 bug 风险、别人看不懂。cJSON 成熟稳定、通用性强、效率高、只有两个文件零依赖。

### Q3：cJSON 怎么在 STM32 上做内存管理？

> cJSON 默认用 C 库 malloc/free。STM32 建议用 cJSON_InitHooks 重定向到 FreeRTOS 的 pvPortMalloc/pvPortFree，统一走 FreeRTOS 堆。必须在第一次使用前调用。

### Q4：cJSON 怎么生成/解析？

> 生成：CreateObject/CreateNumber → AddItemToObject 挂载 → PrintUnformatted 序列化 → free + Delete。解析：Parse → GetObjectItem 逐层取 → 用 IsNumber/IsString 判断 → valuedouble/valuestring 取值 → Delete。

### Q5：cJSON 内存泄漏怎么避免？

> 两条铁律：① Print 出来的字符串必须 free；② 整棵树用 cJSON_Delete 递归释放。创建和释放必须成对。

### Q6：手拼 JSON 和 cJSON 怎么选？

> 字段固定、RAM 紧张、单次上报 → 手拼 snprintf 够用（项目现状）。字段多变、嵌套深、要解析下行数据 → cJSON。本质是"代码简单 vs 内存开销"的权衡。

---

*文档整理日期: 2026-08-16*
