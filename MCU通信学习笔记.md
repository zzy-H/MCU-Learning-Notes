# MCU通信学习笔记

## 一、UART\USART

### 1、UART是什么

**UART**：Universal Asynchronous Receiver/Transmitter，通用异步收发器。

- **异步**：没有独立的时钟线，通信双方必须提前约定好相同的波特率。
- **全双工**：有独立的发送线 TX 和接收线 RX，可以同时收发。
- **点对点**：通常是一个发送端对一个接收端，不能像 I2C 那样挂多个设备。
- **帧传输**：数据以“帧”为单位一位一位地发送。

MCU 里的“串口”通常就是指 UART。很多芯片的串口外设叫 **USART**，多了一个“S”（同步），但实际使用时绝大多数情况只用异步功能，所以可以等同理解。

### 2、硬件连接

UART 是点对点通信，接线必须 **交叉连接**：

```c
MCU_A  TX  ────────>  RX  MCU_B
MCU_A  RX  <────────  TX  MCU_B
```

### 3、概念分层

我们经常听说RS-232,RS-485,RS-422,TTL，这些是什么？，可以往下看

我们可以把串口通信分成两层来看：

| 层次                | 作用                                       | 例子                        |
| :------------------ | :----------------------------------------- | :-------------------------- |
| **协议层/控制器**   | 负责数据的打包、解包、时序控制             | UART、USART                 |
| **物理层/电气标准** | 规定信号电平、传输介质、连接器、传输距离等 | TTL、RS-232、RS-485、RS-422 |

**UART** 属于协议层，它定义了数据帧格式（起始位、数据位、停止位等）和异步传输机制。
**RS-232、RS-485、RS-422** 属于物理层标准，它们规定了用什么样的电压表示逻辑 0 和 1，以及如何连接设备。

通常，UART 控制器输出的信号是 **TTL/CMOS 电平**（如 0~3.3V）。如果要把这个信号传输得更远、更抗干扰，就需要通过电平转换芯片（如 MAX232、MAX485 等）把它变成 RS-232、RS-485 等标准的电平。

所以，实际应用中常见的是：

- **UART over RS-232**：UART 数据帧通过 RS-232 电气标准传输。
- **UART over RS-485**：UART 数据帧通过 RS-485 电气标准传输。
- **UART over RS-422**：UART 数据帧通过 RS-422 电气标准传输。

也就是说，RS-232/485/422 是“运输工具”，UART 是“货物”。货物可以用不同的运输工具运输。

**对比表**

| 特性         | TTL/CMOS     | RS-232                         | RS-422              | RS-485                          |
| :----------- | :----------- | :----------------------------- | :------------------ | :------------------------------ |
| 信号类型     | 单端         | 单端                           | 差分                | 差分                            |
| 逻辑电平     | 0~3.3V/5V    | 逻辑1：-3~-15V，逻辑0：+3~+15V | 差分电压            | 差分电压                        |
| 最大传输距离 | 很短（板内） | 约15m                          | 约1200m             | 约1200m                         |
| 最大速率     | 很高         | 约1Mbps                        | 约10Mbps            | 约10Mbps（距离远会降低）        |
| 通信方式     | 全双工       | 全双工                         | 全双工              | 半双工（典型）或全双工（需4线） |
| 拓扑结构     | 点对点       | 点对点                         | 点对多点（1发多收） | 多机总线（最多32/256节点）      |
| 典型应用     | 板内模块通信 | 老式PC串口、工业设备           | 工业控制、视频监控  | 工业现场总线、Modbus            |
| 最少信号线数 | 2（TX/RX）   | 2（TX/RX）                     | 4（差分收发）       | 2（A/B）                        |
| 抗干扰能力   | 差           | 一般                           | 强                  | 强                              |
| 布线成本     | 极低         | 低                             | 较高                | 高                              |

下面分别给出四种接口的标准接线示意图，均以点对点通信为基础，RS-485 额外补充多点总线组网接法。

**1.TTL/CMOS UART（3 线制・全双工・点对点）**

单片机原生串口的最基础接法，收发交叉、共地连接，芯片直连无需额外转换芯片。

```c
MCU_A                     MCU_B
  TX  ───────────────────►  RX
  RX  ◄───────────────────  TX
 GND  ───────────────────  GND
```

最少 3 根线即可通信，必须共地才能保证电平识别正常

3.3V 与 5V 器件互联时需做电平匹配，避免烧损引脚

**2. RS-232（3 线基础制・全双工・点对点）**

商用设备最常用的最简接法，逻辑与 TTL 一致，仅电平标准不同；完整 DB9 接口可扩展 RTS/CTS 等流控握手线。

```c
设备A                     设备B
 TXD  ───────────────────►  RXD
 RXD  ◄───────────────────  TXD
 GND  ───────────────────  GND
```

对应 DB9 接口典型引脚：2 脚 = RXD、3 脚 = TXD、5 脚 = GND

同为单端共地传输，接线方式与 TTL 串口一致，仅电平幅值不同

**3.RS-422（4 线制・全双工・差分传输）**

```c
设备A                     设备B
 TX+  ───────────────────►  RX+
 TX-  ───────────────────►  RX-
 RX+  ◄───────────────────  TX+
 RX-  ◄───────────────────  TX-
 GND  ───────────────────  GND
```

差分线正接正、负接负，无需交叉

GND 为参考地，长距离、强电磁干扰环境建议连接以抑制共模偏移

**4.RS-485（2 线制・半双工・差分总线）**

工业现场主流接法，仅用一对差分线实现双向通信，核心优势是支持多设备总线组网。

```c
设备A                     设备B
  A   ───────────────────   A
  B   ───────────────────   B
 GND  ───────────────────  GND
```

A（D+）与 A 直连、B（D-）与 B 直连，无交叉

半双工模式，同一时间只能一端发送，需通过 DE/RE 引脚控制收发方向

**多点总线接法（RS-485 核心组网形态）**

```
  ┌───────┐    ┌───────┐    ┌───────┐    ┌───────┐
  │ 主机  │     │从机1  │     │从机2  │    │从机N  │
  │  A/B  │────│  A/B  │────│  A/B  │────│  A/B  │
  └───────┘    └───────┘    └───────┘    └───────┘
     │                                            │
    GND  ───────────────────────────────────────  GND
```

所有设备并联在同一条差分总线上，支持一主多从通信

总线首尾两端建议接 120Ω 终端匹配电阻，抑制信号反射

单总线最多可挂载 32~256 个设备，具体数量由接口芯片的负载能力决定

**注意**：MCU 的串口引脚是 TTL 电平，不能直接接 RS 接口，否则会损坏芯片。

### 4、UART帧格式

UART 以“帧”为单位传输数据。一帧包含：

```c
空闲 ──┐     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┐      ┌──────
       │     │ D0│ D1│ D2│ D3│ D4│ D5│ D6│ D7│校验│停止位│ 空闲
       └─────┘   └───┴───┴───┴───┴───┴───┴───┴───┘     └──────
       起始位(低)      数据位(LSB先发)         可选校验位   高电平
```

- **空闲状态**：总线保持高电平。

- **起始位**：1 位，低电平，表示一帧开始。

- **数据位**：5~8 位可选，通常用 8 位。先发送最低位 LSB。

- **校验位**：可选，用于简单错误检测。

  - 无校验（None）--没有校验位--常用

  - 奇校验（Odd）--**数据位 + 校验位 里面 1 的总数 = 奇数**

    如果数据中 1 的个数已经是奇数 → 校验位 = 0

    如果数据中 1 的个数是偶数 → 校验位 = 1

  - 偶校验（Even）--**数据位 + 校验位 里面 1 的总数 = 偶数**

- **停止位**：1 / 1.5 / 2 位，高电平，表示一帧结束。

**常用格式**：`8N1` —— 8 位数据位、无校验位、1 位停止位。

### 5、波特率

**波特率**：每秒传输的码元（位）数，单位 Baud。

- 常见波特率：**`9600`**、`19200`、`38400`、`57600`、**`115200`**。
- 双方波特率必须一致，否则会出现乱码。
- 实际允许一定误差，一般要求误差 < 3%

**举例**：波特率 9600，发送一个字节（10 位：起始 + 8 数据 + 停止）大约需要 `10 / 9600 ≈ 1.04 ms`。

### 6、MCU处理串口数据的三种方式

| 方式 | 原理                                    | 优点                     | 缺点                   | 适用场景             |
| :--- | :-------------------------------------- | :----------------------- | :--------------------- | :------------------- |
| 轮询 | 主循环不断查询接收/发送标志位           | 实现简单                 | 占用 CPU，容易漏数据   | 简单测试、低数据量   |
| 中断 | 收到数据或发送完成时触发中断            | 响应快，CPU 利用率高     | 中断频繁时影响主循环   | 常规应用，数据量中等 |
| DMA  | 数据自动搬运到内存，无需 CPU 逐字节处理 | 效率最高，CPU 几乎不参与 | 配置复杂，需要理解 DMA | 大数据量、高速通信   |

初学阶段建议先掌握 **轮询** 和 **中断**，DMA 可以后续再学。

### 7、UART 配置步骤（以 STM32 为例）

一般 MCU 的 UART 配置流程如下：

1. **使能时钟**：使能 UART 外设时钟和对应 GPIO 端口时钟。
2. **配置 GPIO**：将 TX 引脚配置为复用推挽输出，RX 引脚配置为浮空输入或上拉输入。
3. **配置 UART 参数**：
   - 波特率（如 115200）
   - 数据位（8 位）
   - 停止位（1 位）
   - 校验位（无）
   - 硬件流控（通常关闭）
4. **使能 UART**：使能发送器、接收器。
5. **使能中断/DMA**（可选）：如果需要中断接收，使能接收中断并配置 NVIC。
6. **编写中断服务函数**：在接收中断中读取数据，或使用 HAL 库的回调函数。

以STM32F1xx系列标准库为例，初始化代码大致如下：

**PS：建议贴合原理图去看，更好理解**

```c
void USART3_Init(void)
{
    //开时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //使能USART3时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//使能GPIOB时钟

    //PB10 TX 复用推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;//定义GPIO结构体变量
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //PB11 RX 输入浮空
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//输入浮空
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //串口3基础配置
    USART_InitTypeDef USART3_InitStructure;//定义USART结构体变量
    USART3_InitStructure.USART_BaudRate = 115200;//波特率
    USART3_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件流控制
    USART3_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
    USART3_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验
    USART3_InitStructure.USART_StopBits = USART_StopBits_1;//1位停止位
    USART3_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据位
    USART_Init(USART3, &USART3_InitStructure);//初始化串口3

    //开启接收中断和空闲中断
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//接收中断使能
    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//空闲中断使能

    //NVIC中断配置
    NVIC_InitTypeDef NVIC_InitStructure;//定义NVIC结构体变量
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//中断通道
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//子优先级
    NVIC_Init(&NVIC_InitStructure);//初始化NVIC

    USART_Cmd(USART3, ENABLE);//使能串口3
}
```

发送单个字节

```c
void USART3_SendByte(uint8_t byte)
{
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);//等待发送缓冲区为空
    //将数据写入发送缓冲区，触发发送
    USART_SendData(USART3, byte);//发送数据
}
```

发送指定长度到缓冲区的数据(利用循环多次发送单个字节)

```c
void USART3_SendBuff(uint8_t *buff, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        USART3_SendByte(*buff++);//发送数据
    }
}
```

发送字符串

```c
void USART3_SendString(char *str)
{
    while (*str)
    {
        USART3_SendByte(*str++);//发送数据
    }
}
```

**※中断服务函数**(这里不过多讲解)

```c
void USART3_IRQHandler(void)
{
    uint8_t data;//定义接收数据变量
    //接收中断
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//接收中断标志位被置位
    {
        data = USART_ReceiveData(USART3);//读取接收到的数据
        //处理接收到的数据,将接收到的数据存入缓冲区（数组）
        if (USART3_ReceiveBuffIndex < USART3_RECEIVE_BUFF_SIZE-1)//判断缓冲区是否已满
        {
            USART3_ReceiveBuff[USART3_ReceiveBuffIndex++] = data;//将接收到的数据存入缓冲区
        }
        else
        {
            //缓冲区已满，处理溢出情况，例如丢弃数据或重置索引
            USART3_ReceiveBuffIndex = 0;//重置索引
        }
        
    }

    //空闲中断
    if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
    {
        USART_ClearITPendingBit(USART3, USART_IT_IDLE);//清除空闲中断标志位
        data = USART_ReceiveData(USART3);//读取接收到的数据，清除中断标志位

        USART3_ReceiveBuffFlag = 1;//设置接收缓冲区标志位，表示接收到一帧数据
    }
}
```

## 二、IIC(I2C)

### 1、I2C 是什么？

**I2C**：Inter-Integrated Circuit，集成电路间总线。也常写作 **IIC** 或 **I²C**。

- **同步通信**：有独立的时钟线 SCL，数据与时钟同步，不需要双方约定波特率。
- **半双工**：只有一根数据线 SDA，收发不能同时进行。
- **两线制**：只需要两根线——SCL（时钟）和 SDA（数据）。
- **多主多从**：一条总线上可以挂多个主机和多个从机，每个从机有唯一地址。
- **开漏输出 + 上拉电阻**：总线空闲时为高电平，器件只能拉低，不能主动拉高。

I2C 最初由飞利浦（现恩智浦）开发，常用于连接 MCU 与传感器、EEPROM、RTC、OLED 显示屏等外围器件。

其中GT911 电容触摸屏驱动、光照传感器bh1750都是IIC通信

### 2、硬件连接

```c
VCC ──┬── 上拉电阻 ── SDA
      │
      ├── 上拉电阻 ── SCL
      │
      ├── 设备1: SDA, SCL, GND
      ├── 设备2: SDA, SCL, GND
      └── GND
```

- **SDA**：数据线，双向。
- **SCL**：时钟线，由主机控制。
- **上拉电阻**：通常 4.7kΩ ~ 10kΩ，接 VCC（3.3V 或 5V）。
- **共地**：所有设备必须共地。

**为什么需要上拉电阻？**

I2C 的引脚是**开漏（或开集）**结构，设备只能把总线拉低，不能主动输出高电平。总线要回到高电平，必须依赖上拉电阻。这样做的好处是：

- 允许多个设备同时驱动同一根线而不会烧毁。
- 可以实现“线与”逻辑，用于仲裁和时钟同步。

### 3、I2C 的速率模式

| 模式      | 速率       | 说明                         |
| :-------- | :--------- | :--------------------------- |
| 标准模式  | 100 kbit/s | 最常用，默认                 |
| 快速模式  | 400 kbit/s | 许多传感器支持               |
| 快速模式+ | 1 Mbit/s   | 较少见                       |
| 高速模式  | 3.4 Mbit/s | 需要特殊处理，MCU 一般不常用 |

实际使用中，**100kHz** 和 **400kHz** 最常见。

### 4、I2C 协议与时序

**起始条件和停止条件**

- **起始条件（Start）**：SCL 为高电平时，SDA 从高电平跳变到低电平。
- **停止条件（Stop）**：SCL 为高电平时，SDA 从低电平跳变到高电平。

- 起始和停止条件由主机产生。
- 总线空闲时，SCL 和 SDA 均为高电平。

**数据有效性**

在 SCL 为**高电平**期间，SDA 上的数据必须保持稳定。只有在 SCL 为**低电平**时，SDA 才允许改变。

**应答信号 ACK / NACK**

每传输完一个字节（8 位），接收方需要发送一个应答位：

- **ACK（应答）**：第 9 个时钟周期，SDA 被拉低。
- **NACK（非应答）**：第 9 个时钟周期，SDA 保持高电平。

如果接收方是主机，NACK 可能表示“不要再发了”；如果接收方是从机，NACK 可能表示“没有收到”或“地址不存在”。

### 5、I2C 的读写流程

**地址格式**

- 从机地址通常是 7 位（也有 10 位，较少见）。
- 7 位地址后面跟着 1 位读写标志：
  - **0**：主机向从机写数据
  - **1**：主机向从机读数据
- 因此一个完整的地址字节是：`7位地址 + 1位读写位`。

例如：某传感器地址为 `0x50`（7位），写操作时发送 `0xA0`，读操作时发送 `0xA1`。

0xA0 = 0x50<<1 | 0;

**写数据流程**

*例：EEPROM（带电可擦可编程只读存储器）如 AT24C02*

AT24C02 是一个 I2C 接口的 EEPROM，容量 256 字节，内部有 256 个存储单元，每个单元有一个 8 位地址（0x00 ~ 0xFF）。

如果你想往第 0x10 个存储单元写一个字节 0x55，I2C 流程是：

```c
Start → 设备地址+写 → ACK → 存储单元地址(0x10) → ACK → 数据(0x55) → ACK → Stop
```

start:主机在SCL高电平时拉低SDA

- **ACK（应答）**：第 9 个时钟周期，SDA 被拉低。

- **NACK（非应答）**：第 9 个时钟周期，SDA 保持高电平。

接收方是从机，NACK 可能表示“没有收到”或“地址不存在”。

**读数据流程**

通常需要先写寄存器地址，再重新启动读操作：

*例：MPU6050 六轴传感器*

MPU6050 有几十个寄存器，例如：

- 0x3B~0x48：加速度和陀螺仪数据寄存器
- 0x6B：电源管理寄存器
- 0x1C：加速度计配置寄存器

你想读取加速度数据，必须先写寄存器地址 0x3B，然后再发起读操作。流程：

```c
Start → 设备地址+写 → ACK → 寄存器地址(0x3B) → ACK → Restart → 设备地址+读 → ACK → 读取数据...
```

这里 **ReStart** 是在不产生停止条件的情况下再次发送起始条件，常用于读操作前先指定寄存器地址。

*注：是不是所有 I2C 设备都需要寄存器地址？*

答：**不是所有设备都需要** 

### 6、I2C 配置示例（以 STM32 标准库库为例）

以常见的 STM32 + I2C 读写 GT911 为例，简要说明配置步骤：

**初始化**

```c
/**
 * @brief I2C 引脚初始化
 *
 * 将 PB1(SCL) 和 PF9(SDA) 配置为开漏输出，
 * 初始化后释放总线（拉高 = 空闲状态）。
 */
void iic_init(void)
{
    // PF9 → SDA, PB1 → SCL
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOF, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Mode  = GPIO_Mode_Out_OD;   // 开漏输出（双向通信）
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    // PB1: SCL
    gpio.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOB, &gpio);

    // PF9: SDA
    gpio.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOF, &gpio);

    // 释放总线（拉高 = 空闲状态）
    GPIO_SetBits(GPIOB, GPIO_Pin_1);   // SCL = 1
    GPIO_SetBits(GPIOF, GPIO_Pin_9);   // SDA = 1
}
```

**起始信号**

```c
/**
 * @brief I2C 起始信号（START）
 *
 * 时序：SCL 高电平时，SDA 由高变低。
 * 结束后 SCL 拉低，为后续数据传输做准备。
 *
 */
void iic_start(void)
{
    // 1. 确保 SCL、SDA 都是高电平（空闲状态）
    GPIO_SetBits(GPIOB, GPIO_Pin_1);   // SCL = 1
    GPIO_SetBits(GPIOF, GPIO_Pin_9);   // SDA = 1
    Delay_us(2);

    // 2. SDA 拉低 → 产生 START 条件（SCL 高时 SDA 下降沿）
    GPIO_ResetBits(GPIOF, GPIO_Pin_9); // SDA = 0
    Delay_us(5);

    // 3. SCL 拉低，释放总线，准备传输数据
    GPIO_ResetBits(GPIOB, GPIO_Pin_1); // SCL = 0
    Delay_us(2);
}

```

**停止信号**

```c
/**
 * @brief I2C 停止信号（STOP）
 *
 * 时序：SCL 高电平时，SDA 由低变高。
 * 结束后 SCL、SDA 均为高电平（总线空闲）。
 *
 */
void iic_end(void)
{
    // 1. 先确保 SDA 为低
    GPIO_ResetBits(GPIOF, GPIO_Pin_9); // SDA = 0
    Delay_us(2);

    // 2. SCL 拉高
    GPIO_SetBits(GPIOB, GPIO_Pin_1);   // SCL = 1
    Delay_us(5);

    // 3. SDA 拉高 → 产生 STOP 条件（SCL 高时 SDA 上升沿）
    GPIO_SetBits(GPIOF, GPIO_Pin_9);   // SDA = 1
    Delay_us(5);
}
```

**等待从机应答（ACK）**

```c
/**
 * @brief 等待从机应答（ACK）
 *
 * 发送完 8 位数据后，主设备释放 SDA 总线，
 * 从设备拉低 SDA 表示应答（ACK）。
 *
 * @return uint8_t 0=收到应答(ACK)，1=未收到应答(NACK/超时)
 *
 * 时序：
 *  SDA  ──────────────────___
 *  SCL  ───────────────────────
 *        ↑ 第 8 位  ↑ SCL 高时从机拉低 SDA = ACK
 */
uint8_t iic_waitack(void)
{
    uint8_t waittime = 0;    // 超时计数器
    uint8_t rack = 0;        // 返回值：0=ACK, 1=NACK

    // 1. 释放 SDA 总线（开漏输出写 1 = 高阻，由上拉拉高）
    GPIO_SetBits(GPIOF, GPIO_Pin_9);   // SDA = 1（释放）
    Delay_us(2);

    // 2. SCL 拉高，让从机在 SDA 上输出应答
    GPIO_SetBits(GPIOB, GPIO_Pin_1);   // SCL = 1
    Delay_us(2);

    // 3. 读取 SDA 电平，等待从机拉低
    //    SDA=0 → 从机应答(ACK)
    //    SDA=1 → 从机无应答(NACK)
    while (GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_9))
    {
        waittime++;
        if (waittime > 250)            // 超时，认为 NACK
        {
            iic_end();                  // 发送 STOP
            rack = 1;                   // 标记 NACK
            break;
        }
    }

    // 4. SCL 拉低，结束应答位
    GPIO_ResetBits(GPIOB, GPIO_Pin_1); // SCL = 0
    Delay_us(5);

    return rack;    // 0=收到 ACK, 1=未收到
}
```

**发送应答信号（ACK）**

```c
/**
 * @brief 发送应答信号（ACK）
 *
 * 主设备接收完一个字节后，拉低 SDA 告诉从机继续发送。
 * （仅在主设备还想继续读数据时调用）
 *
 */
void iic_ack(void)
{
    GPIO_ResetBits(GPIOF, GPIO_Pin_9); // SDA = 0
    Delay_us(2);
    GPIO_SetBits(GPIOB, GPIO_Pin_1);   // SCL = 1（从机检测到 ACK）
    Delay_us(5);
    GPIO_ResetBits(GPIOB, GPIO_Pin_1); // SCL = 0
    Delay_us(2);
    GPIO_SetBits(GPIOF, GPIO_Pin_9);   // SDA = 1（释放）
    Delay_us(2);
}
```

**发送非应答信号（NACK）**

```c
/**
 * @brief 发送非应答信号（NACK）
 *
 * 主设备接收完最后一个字节后，不拉低 SDA，
 * 告诉从机不再继续发送。
 */
void iic_nack(void)
{
    GPIO_SetBits(GPIOF, GPIO_Pin_9);   // SDA = 1（不拉低 = NACK）
    Delay_us(2);
    GPIO_SetBits(GPIOB, GPIO_Pin_1);   // SCL = 1（从机检测到 NACK）
    Delay_us(2);
    GPIO_ResetBits(GPIOB, GPIO_Pin_1); // SCL = 0
    Delay_us(2);
}
```

**发送一个字节（高位先发）**

有人会疑惑：

“我把一个 16 位数据通过 I2C 连续发两个字节，要不要考虑大小端？”

答：

- **单个字节内部：永远遵循 I2C MSB 先行（协议规定，不用管）**
- **多个字节之间谁先发谁后发：这才是大小端问题，由你自己 / 外设寄存器规范定义**

例子：要发送 `uint16_t val = 0x1234`

如果你先发高字节 `0x12`、再发低字节 `0x34` → 大端排布

如果你先发低字节 `0x34`、再发高字节 `0x12` → 小端排布

```c
/**
 * @brief 发送一个字节（高位先发）
 *
 * I2C 规定：数据在 SCL 低电平时改变，SCL 高电平时被采样。
 * 每发送完 8 位后，需调用 iic_waitack() 等待从机应答。
 *
 * @param byte 要发送的 8 位数据
 *
 */
void iic_sendbyte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_1); // SCL = 0（允许改变 SDA）
        Delay_us(2);

        // 取当前最高位输出到 SDA
        if (byte & 0x80)
            GPIO_SetBits(GPIOF, GPIO_Pin_9);   // 发送 1
        else
            GPIO_ResetBits(GPIOF, GPIO_Pin_9);  // 发送 0

        Delay_us(2);
        GPIO_SetBits(GPIOB, GPIO_Pin_1);   // SCL = 1（从机采样）
        Delay_us(5);

        byte <<= 1;                         // 左移，处理下一位
        GPIO_ResetBits(GPIOB, GPIO_Pin_1); // SCL = 0
    }

    // 发送完保持 SCL 低电平，等待外部调用 iic_waitack()
    GPIO_ResetBits(GPIOB, GPIO_Pin_1); // SCL = 0
}

```

**接收一个字节**

```c
/**
 * @brief 接收一个字节
 *
 * SCL 高电平时从 SDA 读取一位，SCL 低电平时移位。
 * 读取完成后根据 ack 参数发送 ACK 或 NACK。
 *
 * @param ack 0=接收完成后发 NACK（最后一字节）
 *            非 0=接收完成后发 ACK（还要继续读）
 * @return uint8_t 接收到的 8 位数据
 */
uint8_t iic_recvbyte(uint8_t ack)
{
    uint8_t byte = 0;

    // 先释放 SDA 总线，让从机控制
    GPIO_SetBits(GPIOF, GPIO_Pin_9);

    for (uint8_t i = 0; i < 8; i++)
    {
        byte <<= 1;                         // 左移，为新位腾出空间
        GPIO_SetBits(GPIOB, GPIO_Pin_1);   // SCL = 1（从机输出数据）
        Delay_us(2);

        // 读取 SDA 电平
        if (GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_9))
        {
            byte++;                         // SDA=1 → 该位为 1
        }

        GPIO_ResetBits(GPIOB, GPIO_Pin_1); // SCL = 0
        Delay_us(2);
    }

    // 发应答/非应答
    if (ack == 0)
        iic_nack();     // 最后一字节，通知从机停止发送
    else
        iic_ack();      // 还要继续读，通知从机继续发

    return byte;
}
```

### 7、实际应用

*以GT911触摸控制芯片为例子*

#### 7.1、触摸引脚初始化（短时复位）

```c
/**
 * @brief 触摸引脚初始化（短时复位）
 *
 * PF10(INT) = 浮空输入（等待 INT 触发）
 * PF11(RST) = 推挽输出，执行一次 1ms 低脉冲复位
 */
void touch_init(void)
{
    // PF10 = INT（浮空输入），PF11 = RST（推挽输出）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    // PF10: INT（中断输入）
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio.GPIO_Pin  = GPIO_Pin_10;
    GPIO_Init(GPIOF, &gpio);

    // PF11: RST（复位输出）
    gpio.GPIO_Pin  = GPIO_Pin_11;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOF, &gpio);

    // 短复位脉冲
    GPIO_ResetBits(GPIOF, GPIO_Pin_11);   // RST = 0
    Delay_SoftMs(1);
    GPIO_SetBits(GPIOF, GPIO_Pin_11);     // RST = 1
    Delay_SoftMs(5);
}
```

**GT911 内部有很多寄存器，必须指定地址**

GT911 是一颗电容触摸控制芯片，内部有几十个寄存器，每个寄存器都有独立地址，功能不同。例如：

| 寄存器地址    | 功能                                          | 读写属性 |
| :------------ | :-------------------------------------------- | :------- |
| 0x8040        | 软件复位寄存器                                | 可读写   |
| 0x8140~0x8143 | 产品 ID（存 "911" 和版本号）                  | 只读     |
| 0x814E        | 触摸状态寄存器（bit7=是否有触摸，低4位=点数） | 可读写   |
| 0x814F~0x8156 | 触摸点 1 的坐标数据（8 字节）                 | 只读     |
| 0x8157~0x815E | 触摸点 2 的坐标数据（8 字节）                 | 只读     |

**如果不告诉芯片“我要访问哪个地址”，芯片就不知道你想读状态还是读坐标，也不知道要写什么数据到哪个功能单元。**

**代码中如何体现寄存器地址**

#### 7.2、 写寄存器：

**`touch_write(uint16_t reg, uint8_t *buf, uint8_t len)`**

```c
/**
 * @brief 通过 I2C 写 GT911 寄存器
 *
 * @param reg 寄存器地址（16 位）
 * @param buf 要写入的数据缓冲区
 * @param len 数据长度（字节）
 * @return uint8_t 0=成功，1=失败（I2C 无应答）
 */
uint8_t touch_write(uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    uint8_t ret = 0;

    iic_start();
    iic_sendbyte((0x14 << 1) | 0);   // 写地址 0x28 设备地址 + 写
    iic_waitack();
    iic_sendbyte((uint8_t)(reg >> 8) & 0xFF);  // 寄存器高字节
    iic_waitack();
    iic_sendbyte((uint8_t)reg & 0xFF);          // 寄存器低字节
    iic_waitack();

    for (i = 0; i < len; i++)
    {
        iic_sendbyte(buf[i]);
        ret = iic_waitack();
        if (ret != 0)
        {
            break;    // 从机无应答，退出
        }
    }
    iic_end();
    
    return (ret != 0) ? 1 : 0;
}
```

**调用例子：**

```c
uint8_t dat = 0x02;
touch_write(0x8040, &dat, 1);   // 向 0x8040 寄存器写 0x02，执行软件复位
```

这里 **0x8040** 就是寄存器地址，告诉 GT911：“我要写的是软件复位寄存器，不是别的寄存器”。

#### 7.2、读寄存器：

`touch_read(uint16_t reg, uint8_t *buf, uint8_t len)`

```c
void touch_read(uint16_t reg, uint8_t *buf, uint8_t len)
{
    // 第一阶段：写入寄存器地址
    iic_start();
    iic_sendbyte((0x14 << 1) | 0);              // 设备地址 + 写
    iic_waitack();
    iic_sendbyte((uint8_t)(reg >> 8) & 0xFF);    // 寄存器地址高字节
    iic_waitack();
    iic_sendbyte((uint8_t)reg & 0xFF);           // 寄存器地址低字节
    iic_waitack();

    // 第二阶段：重复起始，读取数据
    iic_start();
    iic_sendbyte((0x14 << 1) | 1);              // 设备地址 + 读
    iic_waitack();

    for (i = 0; i < len - 1; i++)
        buf[i] = iic_recvbyte(1);                // 读数据，回 ACK
    buf[i] = iic_recvbyte(0);                    // 最后一个字节回 NACK
    iic_end();
}
```

**调用例子：**

```c
uint8_t tp_info;
touch_read(0x814E, &tp_info, 1);   // 读 0x814E 状态寄存器
```

**为什么读取也要先发寄存器地址？**
因为 GT911 内部有多个可读寄存器（状态、坐标、ID 等），必须先指定“我想读哪个寄存器”，它才会把对应数据放到 I2C 总线上。

## 三、SPI

### 1、SPI是什么

**SPI**：Serial Peripheral Interface，串行外设接口，由摩托罗拉（Motorola）推出。

- **同步通信**：有独立时钟线 SCK，主设备产生时钟，从设备根据时钟收发数据，不需要约定波特率。
- **全双工**：有独立的数据发送线 MOSI 和数据接收线 MISO，主从之间可以同时收发。
- **主从架构**：一个主机，一个或多个从机。
- **四线制**：通常需要 4 根线（SCK、MOSI、MISO、CS/SS）。
- **无应答机制**：不像 I2C 有 ACK/NACK，SPI 不确认数据是否收到。
- **无设备地址**：通过片选线 CS 来选择从机。
- **高速**：时钟频率通常可达几 MHz 到几十 MHz（取决于器件），比 I2C 快很多。

SPI 常用于连接 Flash 存储器、SD 卡、显示屏、ADC/DAC、无线模块等需要高速数据交换的设备。

### 2、硬件连接

 **基本四线连接（单从机）**

```c
主机                    从机
SCLK  ──────────────>  SCK   （时钟）
MOSI  ──────────────>  MOSI  （主机输出，从机输入）
MISO  <──────────────  MISO  （从机输出，主机输入）
SS/CS ──────────────>  CS    （片选，低电平有效）
GND   ───────────────  GND
```

- **SCLK/SCK**：串行时钟，由主机产生，控制数据传输节奏。
- **MOSI**：Master Out Slave In，主机输出、从机输入数据线。
- **MISO**：Master In Slave Out，从机输出、主机输入数据线。
- **CS/SS**：Chip Select / Slave Select，片选信号，通常低电平有效。主机拉低 CS 选中从机，通信结束后拉高释放。

**多从机连接方式**

**方式一：独立片选（最常用）**

每个从机都共用 SCK、MOSI、MISO，但每个从机有独立的 CS 引脚，由主机 GPIO 控制。同一时刻主机只拉低一个 CS，选中对应从机。

```c
主机 CS0 ──> 从机0 CS
主机 CS1 ──> 从机1 CS
主机 CS2 ──> 从机2 CS
共用 SCK/MOSI/MISO
```

**方式二：菊花链**

所有从机串联，前一个从机的数据输出接到下一个从机的数据输入，所有从机共用一个 CS。数据通过移位依次传到最远的从机。这种接法节省 CS 引脚，但速度慢，应用较少。

```c
主机 MOSI ──> 从机A MOSI
从机A MISO ──> 从机B MOSI
从机B MISO ──> 从机C MOSI
从机C MISO ──> 主机 MISO
共用 SCK 和 CS
```

### 3、SPI 协议与时序

**1. 数据移位原理**

SPI 的核心是两个 **移位寄存器**：主机的移位寄存器和从机的移位寄存器通过 MOSI/MISO 连接成一个环形。

```c
主机移位寄存器 <--- MISO <--- 从机移位寄存器
       |                          ^
       └------ MOSI ------------->┘
```

每次时钟脉冲，主机和从机同时移出一位数据：

- 主机通过 MOSI 移出一位，同时通过 MISO 移入一位。
- 从机通过 MISO 移出一位，同时通过 MOSI 移入一位。

经过 8 个时钟周期，主机和从机就交换了一个字节。所以 SPI 的读写是同时进行的：**发送一个字节的同时也会收到一个字节**，即使你只想读或只想写。

**2. 数据有效性**

SPI 在时钟的边沿采样数据，具体是上升沿还是下降沿采样，由 **时钟极性（CPOL）** 和 **时钟相位（CPHA）** 决定。

**3. 四种工作模式**

SPI 有四种工作模式，由 CPOL 和 CPHA 两个参数决定：

| 模式   | CPOL | CPHA | 说明                                   |
| :----- | :--- | :--- | :------------------------------------- |
| Mode 0 | 0    | 0    | 时钟空闲低电平，上升沿采样，下降沿移位 |
| Mode 1 | 0    | 1    | 时钟空闲低电平，下降沿采样，上升沿移位 |
| Mode 2 | 1    | 0    | 时钟空闲高电平，下降沿采样，上升沿移位 |
| Mode 3 | 1    | 1    | 时钟空闲高电平，上升沿采样，下降沿移位 |

**CPOL（Clock Polarity，时钟极性）**

- CPOL = 0：SCK 空闲时为低电平。
- CPOL = 1：SCK 空闲时为高电平。

**CPHA（Clock Phase，时钟相位）**

- CPHA = 0：在第一个时钟边沿采样数据（前沿）。
- CPHA = 1：在第二个时钟边沿采样数据（后沿）。

**常用的是 Mode 0 和 Mode 3**，很多 SPI Flash 和 SD 卡默认使用 Mode 0。具体使用哪种模式必须参考从机芯片的数据手册。

### 4、SPI 通信流程

以一个典型的主机向从机发送数据并同时读取响应为例：

1. 主机拉低对应从机的 CS 引脚，选中从机。
2. 主机产生时钟，同时通过 MOSI 发送数据，通过 MISO 接收数据。
3. 传输完成后，主机拉高 CS，释放从机。

例如向 SPI Flash 发送一个读命令（简化）：

```c
CS 拉低
主机发送 0x03（读命令）      → 从机可能返回无效数据
主机发送地址高字节          → 从机返回无效数据
主机发送地址中字节          → 从机返回无效数据
主机发送地址低字节          → 从机返回无效数据
主机发送 0xFF（dummy）      → 从机返回第一个有效数据字节
主机发送 0xFF               → 从机返回第二个有效数据字节
...
CS 拉高
```

**注意**：因为 SPI 是全双工，主机发送数据的同时也在接收数据。即使从机暂时没有有效数据可返回，主机也要发送任意字节（通常 0xFF）来产生时钟，从机才有机会把数据放到 MISO 上。

### 5、SPI 配置示例（以 STM32 标准库库为例）

**SPI2 初始化（主机模式）**

```c
/**
 * @brief SPI2 初始化（主机模式）
 *
 * 配置步骤：
 *   1. 开启 GPIOB 时钟
 *   2. 配置 PB12(CS)/PB13(SCK)/PB14(MISO)/PB15(MOSI) 引脚
 *   3. 开启 SPI2 外设时钟
 *   4. 配置 SPI2 为：主机、8位数据、模式3、MSB先发、软件NSS
 *   5. 使能 SPI2，片选拉高（空闲状态）
 */
void SPI_Config(void)
{
    /* ---------- 1. 开启 GPIOB 时钟 ---------- */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* ---------- 2. 配置 4 个引脚 ---------- */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // PB12: CS（片选）→ 通用推挽输出，软件控制高低电平
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // PB13(SCK) + PB15(MOSI) → 复用推挽输出
    // 注意：F1 的引脚复用不需要设置 GPIO_Pin_AF（那是 F4 才有的成员），
    //       配置成 GPIO_Mode_AF_PP 后引脚自动连接到 SPI2 的对应功能
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // PB14(MISO) → 浮空输入（SPI 主机接收从机数据）
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_14;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ---------- 3. 开启 SPI2 外设时钟 ---------- */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    /* ---------- 4. 配置 SPI2 工作参数 ---------- */
    SPI_InitTypeDef SPI_InitStruct = {0};

    // 波特率预分频：2 分频 → 36MHz/2 = 18MHz（APB1 最高就是 18MHz，取最大）
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;

    // 时钟相位：第 2 个边沿采样（配合 CPOL=High 即模式 3）
    // W25Q64 数据手册：支持模式 0 和模式 3，这里选模式 3
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;

    // 时钟极性：空闲时为高电平（模式 3）
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;

    // 数据宽度：8 位
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;

    // 工作模式：双线双向全工（F1 标准库的宏名，F4 才叫 DualLine）
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;

    // 位顺序：最高位先发（W25Q64 要求 MSB first）
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;

    // 主从模式：主机
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;

    // NSS 管理：软件控制（片选用普通 GPIO 手动拉高拉低）
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;

    /* ---------- 5. 初始化并使能 ---------- */
    SPI_Init(SPI2, &SPI_InitStruct);
    SPI_Cmd(SPI2, ENABLE);

    // 片选拉高 = 未选中任何从机（W25Q64 片选低电平有效）
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
}
```

**SPI 单字节收发（全双工）**

```c
/**
 * @brief SPI 单字节收发（全双工）
 *
 * SPI 是全双工协议：发送一个字节的同时，也会接收从机返回的一个字节。
 * 所以读数据时发送任意字节（如 0xFF 哑元），把从机数据"顶"回来。
 *
 * @param Byte 要发送的字节
 * @return uint8_t 接收到的字节
 */
uint8_t SPI_Send_Rec_Byte(uint8_t Byte)
{
    uint8_t Data = 0;

    // 1. 等待发送缓冲区为空（上一次数据已移出）
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET)
    {
    }

    // 2. 写入要发送的数据
    SPI_I2S_SendData(SPI2, Byte);// 写 DR → 自动清 TXE

    // 3. 等待接收缓冲区非空（从机数据已移入）
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) != SET)
    {
    }

    // 4. 读取接收到的数据
    Data = SPI_I2S_ReceiveData(SPI2);// 读 DR → 自动清 RXNE
	
    // 5. 返回
    return Data;
}
```

### 6、完整的 SPI 通信流程（以 W25Q64 读取 ID 为例）

假设我们要读取 W25Q64 的制造商 ID（命令 0x90），流程如下：

1. **拉低 CS**：`GPIO_ResetBits(GPIOB, GPIO_Pin_12);` 选中从机。
2. **发送命令**：`SPI_Send_Rec_Byte(0x90);` 发送读取 ID 命令，接收到的数据是无效的（可能是 0xFF 或任意值），忽略即可。
3. **发送地址**：发送 3 字节地址（0x000000），同样接收到的数据无效。
4. **发送哑元**：`SPI_Send_Rec_Byte(0xFF);` 此时从机会在 MISO 上返回制造商 ID，接收到的数据就是有效数据。
5. **继续发送哑元**：读取更多字节（如设备 ID）。
6. **拉高 CS**：`GPIO_SetBits(GPIOB, GPIO_Pin_12);` 结束通信。

> **为什么需要哑元？**
> SPI 从机无法主动提供数据，必须由主机产生时钟。发送 0xFF 只是为了让 SCK 时钟线产生 8 个脉冲，从机才能把数据放到 MISO 上。

## 四、CAN和FDCAN

### 1、CAN是什么

**CAN**：Controller Area Network，控制器局域网络，由博世（Bosch）开发，最初用于汽车电子，现在广泛用于工业自动化、机器人、医疗设备等。

- **多主总线**：总线上任意节点都可以主动发送数据，没有固定主机。
- **差分信号**：使用 CAN_H 和 CAN_L 两根线，抗干扰能力极强。
- **异步通信**：没有独立时钟线，靠位时序同步（但比 UART 的波特率同步更复杂）。
- **非破坏性仲裁**：多个节点同时发送时，通过报文 ID 仲裁，优先级高的继续发送，低的自动退出。
- **报文广播**：发送的报文没有特定接收地址，所有节点都能收到，由接收节点通过 ID 过滤决定是否处理。
- **可靠性高**：有 CRC 校验、错误检测、自动重发等机制。

**典型应用**：汽车（发动机、变速箱、车身电子）、工业现场总线（CANopen、DeviceNet）、机器人关节通信。

### 2、CAN物理层

**差分信号**

CAN 使用两根线：CAN_H（高）和 CAN_L（低），信号为差分电压。

| 逻辑状态  | CAN_H | CAN_L | 差分电压（CAN_H - CAN_L） |
| :-------- | :---- | :---- | :------------------------ |
| 显性（0） | 3.5V  | 1.5V  | ~2V                       |
| 隐性（1） | 2.5V  | 2.5V  | ~0V                       |

- **显性**（逻辑 0）优先级更高，总线上只要有一个节点发显性，总线就是显性。
- **隐性**（逻辑 1）是总线空闲状态，所有节点都不发送时总线为隐性。

**终端电阻**

CAN 总线两端必须各接一个 **120Ω 终端电阻**，用于阻抗匹配，消除信号反射。否则高速通信会出错。

**拓扑结构**

- 总线型拓扑，所有节点并联在 CAN_H 和 CAN_L 上。
- 最大节点数：标准 CAN 收发器通常支持 32~128 个节点。
- 通信距离与速率成反比：

| 波特率   | 最大传输距离（典型值） |
| :------- | :--------------------- |
| 1 Mbps   | 40 m                   |
| 500 kbps | 100 m                  |
| 250 kbps | 250 m                  |
| 125 kbps | 500 m                  |
| 50 kbps  | 1000 m                 |

### 3、CAN 硬件连接（收发器的作用）

**完整链路**

```c
STM32G431 FDCAN1
  PA11 ──→ FDCAN1_RX（接收）
  PA12 ──→ FDCAN1_TX（发送）
    ↓
CAN 收发器（L9616/TJA1050 等）── 差分信号 CANH/CANL ──→ 总线
    ↓
120Ω 终端电阻（总线两端各一个，防信号反射）
```

 **为什么必须要有收发器**

STM32 的 FDCAN 是**控制器**（只管协议逻辑），引脚输出 3.3V 单端信号。CAN 总线是**差分信号**（CANH - CANL），需要收发器芯片转换：

- **电平转换**：TXD 的 0/1 → CANH/CANL 差分信号（发送）；差分 → RXD 单端（接收）
- **总线驱动**：CAN 总线要推 120Ω 终端电阻，驱动电流几十 mA，GPIO 推不动
- **电气保护**：总线可能被干扰/短路，收发器起隔离保护作用

**差分信号为什么抗干扰**

干扰（电磁噪声）同时作用在 CANH 和 CANL 两根线上，**差值不变** → 接收端看的是差值，干扰被抵消。这是 CAN 抗干扰的根本原因。

### 4、CAN 协议特点

**报文 ID 与仲裁**

CAN 报文没有“地址”概念，而是用 **标识符（ID）** 来区分报文内容或优先级。ID 数值越小，优先级越高。

当多个节点同时发送时，它们会逐位比较 ID：

- 总线是显性（0）优先。
- 如果一个节点发送隐性（1），但检测到总线上是显性（0），说明有更高优先级节点在发送，该节点立即停止发送并转为接收。

这就是 **非破坏性仲裁**：优先级高的报文不会被打断，低优先级的自动退出，等总线空闲后再重试。

**仲裁场在哪里**

CAN 数据帧开始于一个显性的帧起始位（SOF），紧接着就是 **仲裁场**，里面包含标识符 ID 和 RTR 位。

- 标准帧（11 位 ID）：仲裁场 = 11 位 ID + RTR 位
- 扩展帧（29 位 ID）：仲裁场 = 11 位基础 ID + SRR + IDE + 18 位扩展 ID + RTR

**ID 的发送顺序是从最高位（MSB）到最低位（LSB）**。也就是说，最先发送的是 ID 的最高位。

**仲裁过程：逐位比较，显性优先**

当多个节点同时认为总线空闲并开始发送时，它们会同时发送各自的 SOF，然后进入仲裁场逐位发送 ID。

**每个节点在发送每一位的同时，也会读取总线上的实际电平**：

1. 如果自己发送的是 **显性（0）**，那么总线一定是显性，因为显性覆盖一切。该节点继续发送下一位。
2. 如果自己发送的是 **隐性（1）**，但总线上出现的是 **显性（0）**，说明有至少一个其他节点正在发送显性。由于显性覆盖隐性，该节点立即检测到自己发送的位与总线不一致（位错误），于是 **主动放弃发送，转为接收模式**，不再参与后续仲裁。
3. 如果所有参与仲裁的节点发送的位都相同，则总线呈现该相同电平，所有节点继续发送下一位。

**最终，只有一个节点能完整发送完整个仲裁场，这个节点就是 ID 数值最小的节点**。因为 ID 数值越小，其二进制表示中从高位到低位比较时，第一个出现差异的位上，数值小的节点发送的是 0（显性），而数值大的节点发送的是 1（隐性），于是数值大的节点退出。

**为什么 ID 数值越小，优先级越高？**

这个结论可以从二进制数值比较的特性推导出来。

**对于两个无符号二进制数，从最高位到最低位逐位比较，第一个出现差异的位上，数值较小的那个数该位为 0，数值较大的那个数该位为 1**（因为二进制表示是标准的权重编码）。

在 CAN 仲裁中，显性（0）优先于隐性（1）。所以当两个节点发送的 ID 在某一位上不同时：

- 发送 0 的节点（对应数值较小的 ID）继续发送。
- 发送 1 的节点（对应数值较大的 ID）失去仲裁。

因此，ID 数值越小的报文，越容易在仲裁中胜出，即优先级越高。

**例如**：0x100 和 0x200 在 bit9 处出现差异（0 vs 1），0x100 胜出。

**非破坏性仲裁的意义**

与以太网的 CSMA/CD（冲突检测后随机退避，可能丢失已发送的数据）不同，CAN 的仲裁是 **非破坏性** 的：

- 高优先级报文（ID 小）在仲裁过程中不会受到任何破坏，它继续正常发送。
- 低优先级节点在检测到冲突后立即停止发送，**不会干扰高优先级报文的传输**。
- 低优先级节点会等待当前报文传输完成、总线空闲后，再次尝试发送。

这种机制保证了在总线负载很重的情况下，关键的控制报文（例如紧急制动、安全相关信号）能够以确定的延迟发送出去，这也是 CAN 在汽车和工业控制中得到广泛应用的重要原因。

**总结**

| 要点                        | 说明                                     |
| :-------------------------- | :--------------------------------------- |
| 显性（0）覆盖隐性（1）      | 总线上只要有一个节点发 0，总线就是 0     |
| ID 从最高位开始发送         | 逐位比较，先比较高位                     |
| 节点发送 1 但检测到总线为 0 | 立即失去仲裁，停止发送，转为接收         |
| ID 数值越小，优先级越高     | 二进制比较中，数值小的数在高位更早出现 0 |
| 仲裁是非破坏性的            | 高优先级报文不受影响，低优先级自动退避   |

**广播与过滤**

所有节点都能收到总线上的报文，每个节点通过 **验收滤波器** 设置自己关心的 ID 范围，只接收匹配的报文，其他直接丢弃。这减少了 CPU 负担。

### 5、CAN 报文结构（帧格式）

CAN 有多种帧类型，最常用的是 **数据帧**。数据帧有两种格式：标准帧（11 位 ID）和扩展帧（29 位 ID）

**1.标准数据帧结构（CAN 2.0A）**

| 字段          | 长度     | 说明                                             |
| :------------ | :------- | :----------------------------------------------- |
| SOF（帧起始） | 1 bit    | 显性（0），表示帧开始                            |
| 仲裁场        | 12 bit   | 11 位 ID + RTR（远程请求位）                     |
| 控制场        | 6 bit    | IDE（0=标准帧）+ 保留位 + DLC（数据长度，4 bit） |
| 数据场        | 0~8 字节 | 实际数据                                         |
| CRC 场        | 15+1 bit | 15 位 CRC + 定界符                               |
| ACK 场        | 2 bit    | 发送器发送隐性，接收器回显性表示收到             |
| EOF（帧结束） | 7 bit    | 连续 7 个隐性位                                  |
| 帧间空间      | 3 bit    | 两帧之间的间隔                                   |

**2. 扩展数据帧结构（CAN 2.0B）**

与标准帧的区别：仲裁场变为 29 位 ID，IDE 位为 1（表示扩展帧），其余类似。扩展帧的 ID 范围更大，但优先级相对标准帧稍低（因为 IDE 位出现在仲裁场中，标准帧 IDE=0，显性优先）。

**3. 远程帧（Remote Frame）**

用于请求数据，与数据帧结构类似，但 RTR 位为隐性（1），且没有数据场。接收节点收到远程帧后，发送相同 ID 的数据帧。

**4. 错误帧和过载帧**

- **错误帧**：检测到错误时，节点发送错误标志，通知所有节点。
- **过载帧**：当节点处理不及时时发送，用于延迟下一帧。

**收发流程（广播 + 过滤）**

```c
发送节点：ID + 数据 广播到总线
    ↓
所有节点都收到 → 硬件滤波器按 ID 判断
    ├── 匹配 → 存入 FIFO → 中断通知 CPU
    └── 不匹配 → 硬件直接丢弃（不占 CPU）
```

### 6、CAN 位时序与同步

CAN 的位时间由几个时间段组成，用于解决异步通信中的同步问题。

**1. 位时间组成**

每个位时间分为 4 个段：

| 段名                       | 作用                | 可配置 |
| :------------------------- | :------------------ | :----- |
| 同步段（SYNC_SEG）         | 用于同步，固定 1 Tq | 固定   |
| 传播时间段（PROP_SEG）     | 补偿物理延迟        | 可配置 |
| 相位缓冲段 1（PHASE_SEG1） | 调整相位            | 可配置 |
| 相位缓冲段 2（PHASE_SEG2） | 调整相位            | 可配置 |

### 7、FDCAN\CAN 配置逐行解析

**初始化配置**

```c
hfdcan1.Instance = FDCAN1;
hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;      // 时钟不分频
hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;    // ★ 经典 CAN 帧（不是 FD！）
hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;             // 正常模式
hfdcan1.Init.NominalPrescaler = 20;                // 预分频 20
hfdcan1.Init.NominalSyncJumpWidth = 1;             // 同步跳转宽度
hfdcan1.Init.NominalTimeSeg1 = 10;                 // 时间段1
hfdcan1.Init.NominalTimeSeg2 = 5;                  // 时间段2
hfdcan1.Init.StdFiltersNbr = 1;                    // 标准帧滤波器数量
hfdcan1.Init.ExtFiltersNbr = 1;                    // 扩展帧滤波器数量
```

**波特率计算（重点）**

CAN 位时间 = **同步段 + 时间段 1 + 时间段 2**
$$
\(T_{bit}=T_{SYNC}+T_{BS1}+T_{BS2}\)
$$
所有段单位：**tq（时间量子）**

- SYNC_SEG：固定 **1 tq**
- BS1：1~16 tq
- BS2：1~8 tq

$$
\(T_{bit} = 1 + BS1 + BS2 \quad(\text{tq})\)
$$

时间量子 tq 由 CAN 外设时钟分频得到：
$$
\(T_{tq}=\frac{1}{f_{CAN}/Prescaler}\)
$$

$$
\(波特率 = \frac{f_{CAN}}{Prescaler \times (1+BS1+BS2)}\)
$$

$$
\(f_{CAN}\)：CAN 外设输入时钟（STM32 大部分型号 CAN 挂载 APB1）
$$

Prescaler：预分频器 BRP（1~1024）

以该初始化为例

```c
FDCAN 时钟 = PCLK1 = 170MHz（G431，APB1 不分频）
波特率 = 时钟 / 预分频 / (1 + TSEG1 + TSEG2)
       = 170MHz / 20 / (1 + 10 + 5)
       = 170MHz / 20 / 16
       = 531.25 kHz
```

***采样点（非常重要！调试必看）***
$$
\(采样点 = \frac{1+BS1}{1+BS1+BS2} \times 100\%\)
$$
行业推荐：

- **低速 ≤125k：75%~80%**

- 高速 ≥250k/500k/1M：70%~75%

  采样点不合适极易出现总线报错、丢包。

以该初始化为例子

```c
采样点 = (1 + TSEG1) / (1 + TSEG1 + TSEG2)
       = 11 / 16
       = 68.75%
```

**滤波器配置（详细解析在后面）**

```c
void Can_Message_Init(void)
{
// 配置滤波器：接收 ID 0x000~0x1FF 的所有报文
	FDCAN_FilterTypeDef Filter={0};
	
	Filter.FilterConfig=FDCAN_FILTER_TO_RXFIFO0;// 滤波器配置：使能/禁用过滤或分配给某些 FIFO
	
	Filter.FilterID1= 0x000;// 第一个过滤器标识符
	
	Filter.FilterID2= 0x7FF;// 第二个过滤器标识符（范围过滤或掩码过滤使用）
	
	//Filter.FilterID1= 0x00000000;// 第一个过滤器标识符
	//Filter.FilterID2=0x01ffffff;// 第二个过滤器标识符（范围过滤或掩码过滤使用）
	
	Filter.FilterIndex= 0;// 滤波器索引，表示当前配置的是哪个滤波器
	
	Filter.FilterType= FDCAN_FILTER_RANGE;//滤波器类型：范围过滤或掩码过滤
	
	Filter.IdType=FDCAN_STANDARD_ID;//标识符类型：标准标识符或扩展标识符
	
	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &Filter) != HAL_OK) {
		Error_Handler();//错误处理
	}
	
//配置发送的数据帧类型
	TxHeader.BitRateSwitch=FDCAN_BRS_OFF;//是否启用速率切换（仅在 CAN FD 模式下有效）
	
	TxHeader.DataLength=FDCAN_DLC_BYTES_8;//数据长度代码 (DLC)，指定数据字段的长度
	
	//TxHeader.DataLength=FDCAN_DLC_BYTES_16;
	
	TxHeader.ErrorStateIndicator=FDCAN_ESI_ACTIVE;//错误状态指示器，指定是否使用活动错误状态指示。
	
	TxHeader.FDFormat=FDCAN_CLASSIC_CAN;// 是否使用 CAN FD 格式← 这个是经典 CAN，只能用 8 字节
	
	TxHeader.Identifier=0x125;//消息标识符，用于标识 CAN 帧。
	
	TxHeader.IdType=FDCAN_STANDARD_ID;//标识符类型，指定是标准还是扩展标识符。
	
	TxHeader.MessageMarker=0x02;//消息标记，用于区分传输消息。
	
	TxHeader.TxEventFifoControl=FDCAN_NO_TX_EVENTS;//是否将传输事件存储到 Tx 事件 FIFO。
	
	TxHeader.TxFrameType=FDCAN_DATA_FRAME;//帧类型，指定是数据帧还是远程帧。
	
	if(HAL_FDCAN_ActivateNotification(&hfdcan1,FDCAN_IT_RX_FIFO0_NEW_MESSAGE,FDCAN_RX_FIFO0)!=HAL_OK)// 激活 FDCAN1 的接收 FIFO
	{
		Error_Handler();
	}

//开启 CAN 通信
	HAL_FDCAN_Start(&hfdcan1);//开启 FDCAN 通信
}
```

**滤波器的作用**：CAN 是广播总线，**硬件级"只收我关心的"**——不匹配 ID 的报文直接被硬件丢弃，不占 CPU。

**详细解析**

先明确核心：**FDCAN_FILTER_RANGE 范围过滤模式**，标准 ID，接收 `0x000 ~ 0x7FF` 全部标准 CAN 报文。

```c
void Can_Message_Init(void)
{
	FDCAN_FilterTypeDef Filter={0};
	
	Filter.FilterConfig=FDCAN_FILTER_TO_RXFIFO0;
	Filter.FilterID1= 0x000;
	Filter.FilterID2= 0x7FF;
	Filter.FilterIndex= 0;
	Filter.FilterType= FDCAN_FILTER_RANGE;
	Filter.IdType=FDCAN_STANDARD_ID;
	
	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &Filter) != HAL_OK) {
		Error_Handler();
	}
    // ...后面发送帧头、开启中断、启动FDCAN
}
```

**每个结构体成员详细解释**

① `FilterIndex = 0`

FDCAN 拥有多个滤波器（FDCAN1 一般有**28 个滤波器**，编号 0~27）

这里使用**第 0 号滤波器**，一个滤波器只能做一组规则；想要多组过滤规则就要配置多个 Index。

② `Filter.IdType = FDCAN_STANDARD_ID`

`FDCAN_STANDARD_ID`：标准 ID（11bit，范围 `0x000 ~ 0x7FF`）

`FDCAN_EXTENDED_ID`：扩展 ID（29bit）

PS:如果总线上来了扩展帧，这条滤波器**不会接收**！

③ `Filter.FilterType = FDCAN_FILTER_RANGE`

**FDCAN 滤波器一共两种工作模式：**

1.FDCAN_FILTER_RANGE 范围模式【当前使用】

规则：

> 接收满足：`FilterID1 ≤ 报文ID ≤ FilterID2` 的帧
>
> 你配置：ID1=0x000，ID2=0x7FF
>
> → **所有标准 ID 报文全部放行**，相当于关闭过滤、直通接收。

2.FDCAN_FILTER_MASK 掩码模式（匹配模式）

ID1 = 基准 ID，ID2 = 掩码

公式：`(接收ID & 掩码) == (基准ID & 掩码)`

适合：接收一组 ID 前缀相同的报文。

旧 bxCAN（STM32F1/F4 经典 CAN）只有掩码模式，**FDCAN 新增了范围模式，非常好用**。

④ `Filter.FilterID1`、`Filter.FilterID2`

含义取决于 `FilterType`：

- 范围模式：ID1 = 下限，ID2 = 上限
- 掩码模式：ID1 = 目标 ID，ID2 = 屏蔽掩码

这里：`0x000 ~ 0x7FF`，刚好覆盖全部 11 位标准 ID。

⑤ `Filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0`

匹配成功的报文投递去向：

- `FDCAN_FILTER_TO_RXFIFO0` → 存入 **RX FIFO0**（你代码里中断也是开启 FIFO0 新消息）
- `FDCAN_FILTER_TO_RXFIFO1` → 存入 RX FIFO1
- `FDCAN_FILTER_REJECT` → 匹配到直接丢弃

和中断代码对应

```c
HAL_FDCAN_ActivateNotification(&hfdcan1,FDCAN_IT_RX_FIFO0_NEW_MESSAGE,FDCAN_RX_FIFO0);
```

滤波器匹配报文送入 FIFO0，FIFO0 收到新帧产生中断，逻辑闭环。

**如果想要：同时接收【所有标准帧 + 所有扩展帧】**

PS:没有应用过

需要**配置两个独立滤波器**：滤波器 0 负责标准 ID，滤波器 1 负责扩展 ID。

```c
// ========== 滤波器0：接收全部标准ID（0~0x7FF）==========
Filter.FilterIndex= 0;
Filter.IdType=FDCAN_STANDARD_ID;
Filter.FilterType= FDCAN_FILTER_RANGE;
Filter.FilterID1= 0x000;
Filter.FilterID2= 0x7FF;
Filter.FilterConfig=FDCAN_FILTER_TO_RXFIFO0;
HAL_FDCAN_ConfigFilter(&hfdcan1, &Filter);

// ========== 滤波器1：接收全部扩展ID（0 ~ 0x1FFFFFFF）==========
Filter.FilterIndex= 1;
Filter.IdType=FDCAN_EXTENDED_ID;
Filter.FilterType= FDCAN_FILTER_RANGE;
Filter.FilterID1= 0x00000000;
Filter.FilterID2= 0x1FFFFFFF;
Filter.FilterConfig=FDCAN_FILTER_TO_RXFIFO0;
HAL_FDCAN_ConfigFilter(&hfdcan1, &Filter);
```

两个滤波器都投递到 RXFIFO0，中断统一从 FIFO0 读取。

**读取中断时怎么区分收到的是标准 / 扩展帧？**

```c
FDCAN_RxHeaderTypeDef RxHeader;
uint8_t rx_data[8];
HAL_FDCAN_GetRxMessage(&hfdcan1,FDCAN_RX_FIFO0,&RxHeader,rx_data);

if(RxHeader.IdType == FDCAN_STANDARD_ID)
{
    //标准帧 RxHeader.Identifier 有效范围0~0x7FF
}
else if(RxHeader.IdType == FDCAN_EXTENDED_ID)
{
    //扩展帧 RxHeader.Identifier 有效范围0~0x1FFFFFFF
}
```

**常见误区提醒**

很多人误以为：

> 标准 ID 就是扩展 ID 高 11 位，把滤波器设成扩展 ID 就能顺带收下标准帧

**硬件层面不成立**。CAN 帧仲裁段里有 IDE 位，硬件先识别 IDE 位，再选择对应 ID 滤波器组匹配。

IDE=0（标准帧）→ 只去匹配 STANDARD_ID 类型滤波器

IDE=1（扩展帧）→ 只去匹配 EXTENDED_ID 类型滤波器

**※※※高频踩坑点**

**1标准 ID / 扩展 ID 不要混用**

滤波器 IdType 必须和总线上帧格式一致；标准滤波器收不到扩展帧。

**2多滤波器优先级**

小 Index 优先匹配；一旦被某个滤波器匹配，不会再走后面滤波器。

**3范围模式 ID1 必须 ≤ ID2，否则过滤失效**

**4FDCAN 滤波器默认全部关闭，不配置滤波器，任何报文都收不到！**

**5滤波器只作用接收，发送不受滤波器影响。**

PS如果要使用 CAN FD（支持 64 字节），记得改成：

```c
TxHeader.FDFormat=FDCAN_FD_CAN;
```

#### 7.1接收流程（中断 + 回调）

**中断服务函数**（stm32g4xx_it.c）：

```c
void FDCAN1_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&hfdcan1);   // HAL 统一处理中断
}
```

**接收回调**

```c
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)   // 新消息
    {
        if(hfdcan->Instance == FDCAN1)
        {
            // 从 FIFO0 取报文
            HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, FDCan_RxData);
            FDCan_NewFrame = 1;   // 置标志，主循环处理
            HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);   // LED 指示
        }
    }
}
```

**接收完整链路**：

```c
CAN 总线 → 收发器 → FDCAN 硬件 → 滤波器（ID匹配?）→ FIFO0
    → 中断 FDCAN1_IT0 → HAL_FDCAN_IRQHandler → RxFifo0Callback
    → HAL_FDCAN_GetRxMessage 取数据 → FDCan_NewFrame=1
    → 主循环 FDCAN_Control() 解析 → 控制电机
```

#### 7.2发送（main.c）

```c
if (HAL_GetTick() - last_can_tick >= 100) {
    last_can_tick = HAL_GetTick();
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, FDCan_RxData);
}
```

`HAL_FDCAN_AddMessageToTxFifoQ`：加入**发送 FIFO 队列**，硬件自动发送，非阻塞。

### 8、FDCAN 与 CAN-FD 的关系（概念澄清）

> **核心澄清：FDCAN 是外设名，CAN-FD 是帧格式名，两者不是一回事！**

#### 8.1FDCAN 外设兼容两种帧格式

STM32G431 的 CAN 控制器外设统一叫 **FDCAN**（G431 没有老式 bxCAN），它**兼容两种帧格式**：

| 外设   | 帧格式配置            | 实际跑的协议                       |
| ------ | --------------------- | ---------------------------------- |
| FDCAN1 | `FDCAN_FRAME_CLASSIC` | **经典 CAN 2.0**（8 字节，单速率） |
| FDCAN1 | `FDCAN_FRAME_FD`      | CAN-FD（64 字节，双速率）          |

**项目用的是 FDCAN 外设，跑的是经典 CAN 协议**——两者不冲突。

#### 8.2 经典 CAN vs CAN-FD

| 特性       | CAN 2.0（经典）     | CAN-FD                             |
| ---------- | ------------------- | ---------------------------------- |
| 数据长度   | 8 字节              | **64 字节**                        |
| 速率       | 单一速率（≤1Mbps）  | 仲裁段 ≤1Mbps + **数据段 2~5Mbps** |
| 帧格式     | 标准/扩展           | 标准/扩展 + FD 帧                  |
| 收发器要求 | 经典收发器（L9616） | **FD 收发器**（TJA1044 等）        |
| 应用       | 传统汽车/工控       | 新车型、OTA 升级等大数据量场景     |

#### 8.3 为什么 CAN-FD 需要不同收发器

数据段 5Mbps 时每个位只有 200ns，要求收发器**上升/下降沿足够陡**（信号质量好）。L9616 这类经典收发器按 1Mbps 优化，跑高速数据段波形失真 → 误码。

#### 8.4 项目为什么用经典模式

1. 上位机（CAN 调试工具）是经典 CAN 设备
2. 控制指令 8 字节够用（启停+方向+速度才 4 字节）
3. 531.25kHz 速率下 L9616 工作稳定
4. 兼容性最好

### 9、CAN 收发器详解

只用过L9616，故以L9616为示例介绍

**收发器在链路中的位置**

```c
STM32G431 FDCAN1（控制器，纯逻辑）
  PA11 RX / PA12 TX
       ↓
┌──────────────┐
│    L9616     │ ← CAN 收发器（5V 供电）
└──────┬───────┘
       ↓
 CANH/CANL 差分信号 → 总线（120Ω 终端电阻）
```

**收发器干什么（两个方向）**

- **发送**：TXD 的 0/1 → 驱动 CANH/CANL 输出差分信号（显性位：CANH 高 CANL 低；隐性位：两线约 2.5V）
- **接收**：总线差分信号 → 解调成单端 0/1 → 给 RXD

**为什么 MCU 不能直接接总线**

1. **电平不匹配**：MCU 是 3.3V 单端，总线是差分
2. **驱动能力不够**：总线要推 120Ω 终端电阻，需要几十 mA，GPIO 推不动
3. **电气保护**：总线可能被干扰/短路，收发器隔离保护 MCU

**L9616 关键特性**

| 特性 | 说明                                            |
| ---- | ----------------------------------------------- |
| 标准 | 符合 **ISO 11898**（CAN 物理层标准）            |
| 速率 | 支持 **1 Mbps**（经典 CAN 最高速）              |
| 供电 | **5V**（典型 CAN 收发器）                       |
| 模式 | 正常/待机/静默等                                |
| 引脚 | TXD/RXD（接 MCU）+ CANH/CANL（接总线）+ VCC/GND |

**收发器选型对比**

| 芯片       | 厂商 | 特点                       |
| ---------- | ---- | -------------------------- |
| **L9616**  | ST   | 低成本经典收发器，1Mbps    |
| TJA1050    | NXP  | 最经典 CAN 收发器          |
| TJA1042    | NXP  | 带待机模式，低功耗         |
| TJA1051    | NXP  | **支持 CAN-FD**，可升级    |
| SN65HVD230 | TI   | 3.3V 供电（适合 3.3V MCU） |

**收发器 vs 帧格式的关系（重点）**

| 选择                          | L9616 能用吗                     |
| ----------------------------- | -------------------------------- |
| FDCAN 外设 + 经典模式（项目） | ✅ 完全没问题                     |
| FDCAN 外设 + CAN-FD 模式      | ❌ 需要换 FD 收发器（TJA1051 等） |
| 老 bxCAN 外设 + 经典模式      | ✅ 没问题                         |

> **结论**：L9616 限制的是 **CAN-FD 高速数据段**，不是 FDCAN 外设本身。项目用 FDCAN 外设 + 经典模式 + L9616，组合完全正确。

### 10、调试经验与踩坑

| 坑                              | 现象                 | 解决                              |
| ------------------------------- | -------------------- | --------------------------------- |
| **ActivateNotification 参数错** | 中断触发但回调不执行 | 第三个参数必须传 `FDCAN_RX_FIFO0` |
| 收不到数据                      | 滤波器没配对         | 检查 FilterConfig + ID 范围       |
| 波特率不对                      | 通信失败/乱码        | 按公式：170M/20/16=531.25k        |
| 忘记 HAL_FDCAN_Start            | 完全不工作           | 配置完必须 Start                  |
| 总线没接终端电阻                | 距离稍远就通信不稳   | 总线两端各接 120Ω                 |

## 五、扩展TCP/IP

### 1、TCP/IP是什么？

**TCP/IP** 是一组网络通信协议的集合，以其中两个核心协议 **TCP**（传输控制协议）和 **IP**（网际协议）命名。它是互联网的基础，定义了数据如何在网络中打包、寻址、传输和接收。

###  2、分层模型

通常用四层或五层模型来描述 TCP/IP 协议栈：

| 层次                 | 功能                                 | 常见协议             |
| :------------------- | :----------------------------------- | :------------------- |
| 应用层               | 应用程序之间交换数据                 | HTTP、MQTT、FTP、DNS |
| 传输层               | 端到端的数据传输，提供可靠性或实时性 | TCP、UDP             |
| 网络层               | 数据包的路由和转发，逻辑寻址         | IP、ICMP、ARP        |
| 链路层（网络接口层） | 物理介质上的数据传输，如以太网、WiFi | Ethernet、WiFi       |

**数据封装过程**：应用层数据 → 加上 TCP/UDP 头 → 加上 IP 头 → 加上以太网帧头 → 发送到物理介质。接收时逐层解封装。

**TCP 和 IP 各自干什么**：

- **IP**：负责"把数据送到哪台机器"（寻址，靠 IP 地址）
- **TCP**：负责"在这台机器上可靠地传数据"（三次握手建立连接、确认重传、按序到达）

> 比喻：IP 是"快递公司送件地址"，TCP 是"快递单号跟踪——丢了补发、乱了重排"。

### 3. TCP 与 UDP 对比

| 特性     | TCP                      | UDP                   |
| :------- | :----------------------- | :-------------------- |
| 连接方式 | 面向连接（三次握手）     | 无连接                |
| 可靠性   | 可靠，有确认、重传、排序 | 不可靠，不保证送达    |
| 速度     | 较慢                     | 较快                  |
| 数据流   | 字节流                   | 数据报                |
| 适用场景 | 文件传输、网页、MQTT     | 视频流、实时游戏、DNS |

**MQTT 基于 TCP**，因为物联网数据通常要求可靠传输，即使牺牲一些实时性。

 **IP 地址与端口**

- **IP 地址**：标识网络中的主机，如 `192.168.1.100`。
- **端口号**：标识主机上的应用程序，如 HTTP 默认端口 80，MQTT 默认端口 1883（明文）或 8883（TLS 加密）。

一个 TCP 连接由四元组唯一确定：`源 IP + 源端口 + 目的 IP + 目的端口`。

### 4、嵌入式中的 TCP/IP 实现

MCU 本身资源有限，不能直接运行完整的 Linux 网络栈。通常有两种方式：

#### 1. 使用外部网络模块

- **WiFi 模块**：如 ESP8266、ESP32，内部已经集成 TCP/IP 协议栈，MCU 通过 UART/SPI 发送 AT 指令或使用 SDK 进行通信。
- **以太网模块**：如 W5500，内部硬件实现 TCP/IP 协议栈，MCU 通过 SPI 读写寄存器即可。

这种方式下，MCU 不需要运行协议栈，只需处理应用层数据，简化开发。

#### 2. 在 MCU 上运行轻量级协议栈

- **lwIP**：最常用的开源 TCP/IP 协议栈，专为嵌入式设计，占用资源少，支持 TCP、UDP、DHCP、DNS 等。
- **FreeRTOS + TCP**：另一种选择。

STM32 等 MCU 通常通过 **以太网 MAC 控制器** + 外部 PHY 芯片（如 LAN8720）连接有线网络，配合 lwIP 实现网络通信。

| 层     | 项目中的实体                                   |
| ------ | ---------------------------------------------- |
| 应用层 | MQTT 报文（cloud.c 打包）+ JSON（snprintf 拼） |
| 传输层 | TCP（ESP8266 固件内部实现）                    |
| 网络层 | IP（ESP8266 固件内部实现）                     |
| 链路层 | WiFi（ESP8266 模块）                           |
| 物理层 | 无线电波                                       |

> **关键理解**：ESP8266 把 TCP/IP 协议栈**整个封装在模块内部**（AT 指令屏蔽细节）。STM32 不需要懂 TCP，只需要"往串口写数据"——TCP 的握手、重传、分包全是 ESP8266 干的。

### 5、详细解析TCP核心机制

**三次握手（建立连接）**

**四次挥手（断开连接）**

#### 5.1三次握手（建立连接）

```c
客户端                         服务器
  │  SYN (seq=x) ────────────→  │
  │  ←──────────── SYN+ACK     │
  │  ACK ────────────────────→  │
  └── 连接建立，开始传数据 ──────┘
```

**为什么是三次不是两次**：防止"失效的连接请求"突然到达服务器，导致服务器白等。第三次 ACK 是客户端确认"我知道你准备好了"。

- **seq（序号）**：本报文第一个字节的编号（从随机数开始）
- **ack（确认号）**：**期望对方下一个发来的 seq**（= 已收到对方 seq-1 之前的所有字节）
- **SYN**：请求建立连接；**ACK**：确认；**FIN**：请求关闭连接

**核心规则**：`ack = 对方上一个 seq + 数据长度`（SYN/FIN 各占 1 个序号）。

**示例：**

假设 ESP8266（客户端）连接 OneNET 服务器（1883 端口），初始序号随机：客户端 ISN=1000，服务器 ISN=5000：

```c
客户端 (ESP8266)                    服务器 (OneNET)
    │                                  │
    │  ① SYN, seq=1000                 │
    │ ────────────────────────────────→│
    │      客户端: 我想连接，我的序号从1000开始
    │                                  │
    │  ② SYN+ACK, seq=5000, ack=1001   │
    │ ←────────────────────────────────│
    │      服务器: 好，我的序号从5000开始，我确认收到你的1000
    │                                  │
    │  ③ ACK, seq=1001, ack=5001       │
    │ ────────────────────────────────→│
    │      客户端: 我确认收到你的5000，连接建立！
    │                                  │
    │  ④ 开始传数据 (seq=1001, 数据...)│
    │ ────────────────────────────────→│
```

**TCP 报文头总是有序号和确认号**

TCP 报文头**固定包含**两个 32 位字段：**序号（seq）** 和 **确认号（ack）**。无论这个报文是否携带数据，这两个字段都必须填写。

- **seq**：表示这个报文段中第一个数据字节的序号。如果报文不携带数据（比如纯 ACK），seq 仍然要填，它表示当前连接中“我下一个要发送的数据字节的序号”。
- **ack**：表示“我期望收到对方的下一个字节的序号”。

所以第三次握手的 ACK 报文虽然不携带任何应用数据，但它仍然需要填写 seq 和 ack 字段。

**提问：为什么不能是两次**

**场景**：客户端发了 SYN，但**在网络中滞留**（堵车），客户端等不到回复，超时重发 SYN 并成功建立连接、传完数据、关闭连接。

此时**第一个滞留的 SYN 才到达服务器**：

- **两次握手**：服务器收到"过期 SYN"，以为客户端要建新连接 → 分配资源、回 SYN+ACK → **服务器白白浪费资源**（客户端根本不想要）
- **三次握手**：客户端收到服务器回的 SYN+ACK，发现序号对不上 → **不回 ACK** → 服务器超时释放资源 

> **一句话**：第三次 ACK 是客户端给服务器吃的"定心丸"——防止服务器为过期请求白等。

**提问：为什么不是四次**

第三次 ACK 已经让双方都确认了状态，第四次没有新信息。三次是**信息完备的最小值**

**注意：在使用外部网络模块时候，内置可能有三次握手，例如ESP8266**

```c
// esp.c：这一条 AT 指令背后，ESP8266 固件完成了整个三次握手
AT+CIPSTART="TCP","mqtts.heclouds.com",1883
```

时序上：

1. ESP8266 DNS 解析域名 → 得到 OneNET 服务器 IP
2. 发 SYN（对应代码里等 "OK" 回复的那段时间）
3. 服务器回 SYN+ACK
4. ESP8266 回 ACK → 连接建立 → 返回 "OK"

**调试技巧**：这条指令一直超时（等不到 OK）= 握手失败——服务器 IP/端口不对、网络不通、或服务器拒绝。

#### 5.2四次挥手

假设客户端（ESP8266）主动断开，seq 接着之前的数据流：客户端 seq=2000，服务器 seq=7000：

```c
客户端                             服务器
    │                                  │
    │  ① FIN, seq=2000                 │
    │ ────────────────────────────────→│
    │      客户端: 我的数据发完了，我要关了
    │                                  │
    │  ② ACK, ack=2001                 │
    │ ←────────────────────────────────│
    │      服务器: 收到，我确认你的FIN（但我还有数据要发）
    │                                  │
    │  ③ （服务器继续发剩余数据...）     │
    │ ←────────────────────────────────│
    │      服务器: 我的数据也发完了      │
    │                                  │
    │  ④ FIN, seq=7000                 │
    │ ←────────────────────────────────│
    │      服务器: 我也要关了            │
    │                                  │
    │  ⑤ ACK, ack=7001                 │
    │ ────────────────────────────────→│
    │      客户端: 确认，关闭            │
    │                                  │
    │  ⑥ 客户端进入 TIME_WAIT（2MSL）   │
```

**为什么是四次（重点）**

TCP 是**全双工**——两个方向的数据流**互相独立**，各自要单独关闭：

```c
方向 A：客户端 → 服务器（客户端主动关，发 FIN）
方向 B：服务器 → 客户端（服务器关，也要发自己的 FIN）
```

**为什么不能合并**：

- 客户端发 FIN 时，**服务器可能还有数据没发完**
- 服务器收到 FIN 只能先回 ACK（"我收到了"），**等自己数据发完**再发自己的 FIN
- 所以中间必然隔一段时间 → 拆成两步（ACK + 稍后的 FIN）→ 总共 4 步

> **对比**：握手时双方都没数据要传，SYN+ACK 可以合并成一步；挥手时服务器可能还有数据，ACK 和 FIN 不能合并。

**TIME_WAIT 是干嘛的（进阶）**

主动关闭的一方（客户端）发完最后的 ACK 后，**不立即关闭**，进入 TIME_WAIT 等 2MSL（约 1~4 分钟）：

1. **确保最后的 ACK 到达**：万一 ACK 丢了，服务器会重发 FIN，客户端还能再回 ACK（否则服务器永远等不到确认）
2. **让旧连接的数据包在网络上消失**：防止"上一个连接的残留数据包"被新连接误收

> **面试点**：TIME_WAIT 是面试官爱追问的——"四次挥手后为什么还要等？"

在MQTT中挥手在哪里？

```c
AT+CIPCLOSE        // 正常关闭（四次挥手）
// 或 AT+RST         // 直接重启（相当于物理断开）
```

**但实际场景**：设备上电后一直保持连接（心跳保活），一般不主动挥手。挥手通常发生在：

- 服务器主动断开（设备长时间无心跳，服务器踢掉连接）
- 设备重启 / WiFi 断开（此时不是标准挥手，而是连接中断）

**中断 vs 挥手**：

- **四次挥手**：双方协商，优雅关闭（数据都传完）
- **连接中断**（断电/断网）：没有挥手，直接消失 → 对方靠**超时**（TCP keepalive 或 MQTT 心跳超时）才发现连接死了

### 6、MQTT 协议详解

#### 6.1 层次关系

```c
MQTT（应用层：发布/订阅消息）
  ↓ 封装成 MQTT 报文
TCP（传输层：可靠传输）← ESP8266 内部
  ↓
IP（网络层）
  ↓
WiFi（链路层）
```

**为什么 MQTT 需要 TCP**：MQTT 报文要求**不丢不重**（QoS 保证），TCP 的可靠传输是它的地基。

#### 6.2 核心概念（发布/订阅）

```c
          ┌──────────┐
          │ OneNET    │ ← Broker（消息代理）
          └──────────┘
          ↑发布         ↓订阅
  设备（Publisher）      手机 App（Subscriber）
```

- **Broker（代理）**：中转站，负责收消息、按主题转发
- **主题（Topic）**：消息的"频道名"，如 `$sys/a8QXjFofql/Demo_TEST/thing/property/post`
- **发布/订阅解耦**：发送方不需要知道接收方是谁，只管发到主题

#### 6.3 MQTT 报文类型

| 报文      | 方向        | 作用             |
| --------- | ----------- | ---------------- |
| CONNECT   | 设备→服务器 | 连接 + 身份鉴权  |
| CONNACK   | 服务器→设备 | 连接确认（0x20） |
| PUBLISH   | 双向        | 发布消息（0x30） |
| SUBSCRIBE | 设备→服务器 | 订阅主题         |
| SUBACK    | 服务器→设备 | 订阅确认（0x90） |
| PINGREQ   | 设备→服务器 | 心跳请求         |
| PINGRESP  | 服务器→设备 | 心跳回复（0xD0） |

#### 6.4 MQTT 四步（cloud.c 真实代码）

**① CONNECT（连接 + 鉴权）**：

```c
MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
data.clientID.cstring = ClientID;      // "Demo_TEST"
data.username.cstring = UserName;      // "a8QXjFofql"
data.password.cstring = PassWord;      // 签名（md5 鉴权）
len = MQTTSerialize_connect(Esp.S_Buff, ESP_S_Buff_Length, &data);
```

**② PUBLISH（上行数据上报）**：

```c
// 传感器数据 → JSON 字符串 → MQTT 发布报文 → TCP → 服务器
len = MQTTSerialize_publish(Esp.S_Buff, ESP_S_Buff_Length, 0,0,0,0,
                            topicString, (unsigned char*)payload, payloadlen);
```

**③ SUBSCRIBE（订阅下行指令）**：告诉 Broker"我要听 property/set 主题"

**④ PINGREQ（心跳保活）**：

```
if (PING_Period[0] >= PING_Period[1]) {
    PING_Period[0] = 0;
    MQTT_Ping();   // 周期发 PINGREQ，防止服务器断开连接
}
```

MQTT 连接状态靠**心跳维持**——TCP 连接建立后长时间不发数据，服务器会认为设备死了并断开。PINGREQ/PINGRESP 就是"我还活着"的确认。

## 补充

### 1.为什么RS-485总线首尾要接 120Ω 终端电阻

核心目的是**消除传输线的信号反射**，保证通信波形稳定，底层基于传输线理论。

**1. 信号反射是怎么来的**

当 RS-485 总线通信速率较高、线缆较长时，数字信号的上升沿 / 下降沿很陡，此时导线不再是 “理想导线”，而是一条**传输线**，存在固定的**特性阻抗**。

工业现场常用的 RS-485 屏蔽双绞线，特性阻抗标称值就是 **120Ω**。

如果总线末端开路（接收器输入阻抗高达几十 kΩ，和 120Ω 严重不匹配），信号传到末端就会像 “声音撞墙产生回声” 一样，沿总线反射回去。反射波和原始信号叠加后，会出现**振铃、过冲、电平台阶**，导致芯片无法正确识别高低电平，出现丢包、误码，严重时完全无法通信。

**2. 终端电阻的作用**

在总线两端并联和线缆特性阻抗相等的 120Ω 电阻，可以让信号到达末端时被电阻完全吸收，不再产生反射，从而保证整条总线上的信号波形干净、稳定。

**3. 为什么是 120Ω**

这个数值不是随意定的：工业标准 RS-485 线缆（如 RVSP 屏蔽双绞线）的特性阻抗标称值就是 120Ω，电阻值和线缆特性阻抗一致时，匹配效果最好，反射最小。

**4.怎么接线**

核心规则

只接在**整条总线的两个最远端端点**（首端、尾端各 1 个），中间所有节点绝对不能接。

电阻直接**跨接在差分线 A（D+）和 B（D-）之间**，不需要接地。

一条总线上只能有 **2 个** 120Ω 终端电阻。

### 2、GT911 的 I2C 地址选择机制

GT911 的 I2C 从机地址不是固定死的，而是可以在上电/复位时通过 **INT 引脚的电平** 来配置。它支持两个常用的 7 位地址：

| INT 电平（复位释放时） | 7 位地址 | 8 位写地址 | 8 位读地址 |
| :--------------------- | :------- | :--------- | :--------- |
| 高电平（1）            | 0x14     | 0x28       | 0x29       |
| 低电平（0）            | 0x5D     | 0xBA       | 0xBB       |

这样设计是为了避免 I2C 总线上地址冲突。例如，当总线上有两个 GT911 或与其他设备地址冲突时，可以通过硬件连接选择不同地址。

**地址选择的原理**

GT911 在 **复位释放的瞬间**（RST 从低变为高）采样 INT 引脚的电平，并锁存到内部寄存器，决定 I2C 地址。此后 INT 引脚恢复为触摸中断输出功能，外部电平不再影响地址。

**关键点：采样时刻是 RST 上升沿，而不是复位期间任意时刻。**

**代码中的复位时序**

```c
void touch_reset(void)
{
    // 临时将 INT(PF10) 改为推挽输出
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOF, &gpio);

    GPIO_ResetBits(GPIOF, GPIO_Pin_11);   // RST = 0
    GPIO_SetBits(GPIOF, GPIO_Pin_10);     // INT = 1
    Delay_SoftMs(20);

    GPIO_SetBits(GPIOF, GPIO_Pin_11);     // RST = 1（上升沿）
    Delay_SoftMs(300);

    // INT 恢复为浮空输入
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOF, &gpio);
    Delay_SoftMs(100);
}
```

### 3、GT911 坐标解析细节

**1. 触摸状态寄存器（0x814E）**

每次读取触摸数据前，都需要先读状态寄存器，判断是否有触摸以及触摸点数。

```c
uint8_t tp_info;
touch_read(0x814E, &tp_info, 1);

if ((tp_info & 0x80) == 0x80)   // bit7 = 1 表示有触摸
{
    tp_cnt = tp_info & 0x0F;    // 低 4 位 = 触摸点数（1~5）
}
```

- **bit7**：缓冲区状态，1 表示有触摸数据待读取。
- **bit6**：大点标志（某些版本使用）。
- **bit5**：保留。
- **bit4**：是否有按键（某些版本）。
- **bit3~0**：触摸点数量（1~5）。

处理完触摸数据后，需要向 0x814E 写 0 来清除标志，否则芯片不会更新新的触摸数据。

**2. 单个触摸点数据结构（8 字节）**

GT911 的每个触摸点占用 8 个字节，地址从 0x814F 开始，每点间隔 8 字节。

代码中定义的数组：

```c
static const uint16_t touch_tp[5] = {
    0x814F,  // 触摸点 1
    0x8157,  // 触摸点 2
    0x815F,  // 触摸点 3
    0x8167,  // 触摸点 4
    0x816F,  // 触摸点 5
};
```

每个 8 字节的数据格式如下（以索引 0~7 表示）：

| 字节索引 | 含义            | 说明                               |
| :------- | :-------------- | :--------------------------------- |
| 0        | Track ID        | 触摸点标识，0~5，用于跟踪手指      |
| 1        | X 坐标低 8 位   | X 坐标 = (byte[2] << 8) \| byte[1] |
| 2        | X 坐标高 8 位   | 大端组合，高字节在后               |
| 3        | Y 坐标低 8 位   | Y 坐标 = (byte[4] << 8) \| byte[3] |
| 4        | Y 坐标高 8 位   | 大端组合                           |
| 5        | 触摸面积低 8 位 | Size = (byte[6] << 8) \| byte[5]   |
| 6        | 触摸面积高 8 位 |                                    |
| 7        | 保留            | 通常为 0                           |

**3. 代码中的解析**

```c
touch_read(touch_tp[i], tpn_info, 8);

raw.x    = ((uint16_t)tpn_info[2] << 8) | tpn_info[1];   // X = 高字节<<8 | 低字节
raw.y    = ((uint16_t)tpn_info[4] << 8) | tpn_info[3];   // Y
raw.size = ((uint16_t)tpn_info[6] << 8) | tpn_info[5];   // Size
```

这完全符合上述格式。注意 `tpn_info[2]` 是高字节，`tpn_info[1]` 是低字节，组合成 16 位坐标。

**坐标范围**：GT911 默认坐标分辨率通常为 **0~4095**（12 位）或 **0~2047**，取决于固件配置。具体需要参考数据手册或实际测试。如果你的屏幕分辨率是 800x480，需要进行缩放映射。

**4. 屏幕方向坐标变换**

代码中根据 `lcd_dir` 进行了四个方向的坐标变换：

```c
switch (dir)
{
    case dir_0:
        point[i].x = w - raw.y;
        point[i].y = raw.x;
        break;
    case dir_90:
        point[i].x = raw.x;
        point[i].y = raw.y;
        break;
    case dir_180:
        point[i].x = raw.y;
        point[i].y = h - raw.x;
        break;
    case dir_270:
        point[i].x = w - raw.x;
        point[i].y = h - raw.y;
        break;
}
```

这些变换的作用是将触摸芯片输出的原始坐标（基于触摸屏物理坐标系）映射到 LCD 显示坐标系，以适应屏幕旋转。

- **dir_0**：屏幕正常竖屏（0°）
- **dir_90**：屏幕旋转 90°，此时触摸坐标与显示坐标一致，直接使用。
- **dir_180**：屏幕旋转 180°，`x = raw.y`，`y = h - raw.x`。
- **dir_270**：屏幕旋转 270°，`x = w - raw.x`，`y = h - raw.y`。

**注意**：这些变换是否准确取决于你的 LCD 与触摸屏的物理安装关系，实际调试时需要根据触摸位置是否正确来调整。

**坐标缩放**

GT911 输出的坐标值范围可能大于屏幕分辨率，例如 4096x4096。在使用前通常需要进行比例缩放：

```c
uint16_t x_display = (uint32_t)raw.x * screen_width / 4096;
uint16_t y_display = (uint32_t)raw.y * screen_height / 4096;
```

**6. 清除触摸标志**

读取完所有触摸点并处理后，必须写 0 到 0x814E：

```c
tp_info = 0;
touch_write(0x814E, &tp_info, 1);
```

这告诉 GT911 数据已读取完毕，可以更新下一次触摸数据。如果不写，GT911 会一直认为缓冲区满，不再上报新触摸。

- **地址选择**：复位时 RST 上升沿采样 INT 电平，高电平 → 0x14，低电平 → 0x5D。代码中应确保时序与使用的地址一致。
- **坐标解析**：每点 8 字节，X/Y 坐标由两个字节组成（高字节在后），组合后可能需旋转、缩放才能对应屏幕。
- **状态清除**：每次读完必须写 0 到状态寄存器，否则触摸数据不会更新。

这两个细节都体现了 GT911 作为复杂 I2C 设备的特点：**通过寄存器进行功能配置和数据交换**，理解它对以后调试其他触摸芯片或传感器会很有帮助。

