# IAP升级学习笔记

## 一、基础概念区分

### 1. HEX / BIN 固件文件

**HEX**：自带地址信息，体积更大；ST-Link 下载直接识别，无需手动填起始地址；不适合串口 IAP 分包传输。

**BIN**：纯二进制代码，无地址，体积小；**IAP 升级标准文件**，串口 / 无线升级只用 bin。

### 2.Keil 自动生成 BIN 配置

**1.打开keil安装路径**

**2.切换到路径：MDK 安装目录`ARM\ARMCC\bin\fromelf.exe`**

找到**fromelf.exe文件并把所在路径复制出来**

例如：E:\2.MDK5\ARM\ARMCC\bin

**3.在魔法棒页面output页面下把Name of Execulabe 后面的名字复制出来**

例如："Demo"(一般是工程名字)

**4.把下面这个脚本格式按要求修改**

**E:\2.MDK5\ARM\ARMCC\bin**\fromelf.exe --bin -o .\Objects\`weather`.bin .\Objects\`weather`.axf

黑色部分：换成自己第 2 步中复制路径

灰色部分：换成自己第 3 步中复制的名字

例如：

**E:\2.MDK5\ARM\ARMCC\bin**\fromelf.exe --bin -o .\Objects\`Demo`.bin .\Objects\`Demo`.axf

**5.按照一下操作设置**

<魔法棒--user--勾选Run#1--填入脚本格式--点击OK>

**6.编译查看是否报错**

**7.查看工程路径下的 bin 文件**

路径为Demo\project\Objects\Demo.bin

## 二、什么是 IAP

IAP = In-Application Programming，**在应用程序中编程**。

不用 ST-Link、不用拆开板子，设备上电运行原有程序时，通过串口 / USB/WiFi 等通道接收新固件，自己擦写 Flash 完成程序升级。

对比三种烧录方式

1.**ISP**：ST 出厂固化系统存储器，BOOT0=1、BOOT1=0，串口烧录，需改硬件跳线，工厂烧录使用。

2.**IAP（在应用编程）**：本地升级（串口 / SD 卡 / U 盘 / W25Q64），无需改动 BOOT 引脚，产品现场升级。

3.**OTA**：空中下载，基于 WiFi/4G 网络从服务器获取固件，底层逻辑和 IAP 完全一致，只是数据来源为网络。

## 三、IAP 分区架构（F103 Flash 划分核心）

**以下以STM32F103为例子**

STM32F103ZET6 Flash 总容量 512KB

总 Flash 地址：`0x08000000 ~ 0x0807FFFF`，分 3 块防升级变砖

| 分区              | 地址范围              | 大小 | 功能                                                         |
| ----------------- | --------------------- | ---- | ------------------------------------------------------------ |
| BootLoader 引导区 | 0x08000000~0x08002FFF | 12K  | 上电优先运行；检测备份区固件、拷贝、跳转 APP；**禁止擦写自身** |
| APP1 运行区       | 0x08003000~0x080417FF | 250K | 设备正常运行的机械臂业务程序                                 |
| APP2 备份缓存区   | 0x08041800~0x0807FFFF | 250K | 临时存放新固件，校验通过再覆盖 APP1；升级中途断电不破坏可用程序 |

## 四、整体运行流程

### 1.上电流程（BootLoader 执行逻辑）

1.上电自动进入 BootLoader；

2.读取 APP2 备份区末尾 4 字节标志位（`0x0807FFFC`，Flash 最后 4 字节）：值为`0xAAAA`（半字写两次，4 字节内容为 AA AA AA AA）代表存在完整新固件；

3.有升级标志：把 APP2 全部拷贝至 APP1 → 清除升级标志 → 跳转 APP1（实现上未显式擦 APP1，靠写入函数自动“检测非空页 → 整页擦 → 重写”，见第六章第 2 节）；

4.无升级标志：直接校验 APP1 向量表合法性，合法则跳转 APP1；

5.APP1 校验失败则打印错误、停留在 BootLoader 的 LED 闪烁循环（**本代码 Boot 不含串口接收逻辑，无法原地等升级**；产品级应加“按键/串口指令进入强制升级模式”，见第七章第 2 节）。

### 2.APP1 正常运行 + 升级流程

1.APP1 启动第一件事：**修改中断向量表偏移`SCB->VTOR = 0x08003000`**，否则所有中断失效；

2.正常执行舵机、屏幕、电机控制业务；

3.收到串口升级指令：分包接收 bin 固件，写入 APP2 备份区；

4.串口空闲 **100ms** 判定接收完成（`Flash_APP1/user/api/usart.c:14`），在 APP2 末尾（`0x0807FFFC`）写入升级标志`0xAAAA`；

5.软件复位，重启进入 BootLoader 执行更新（本代码是打印提示后**手动按复位键**；产品级可自动调用`NVIC_SystemReset()`，见第七章第 4 节）。

在 APP 程序的编译时，我们需要修改地址属性

在魔法棒页面Target页面内勾选IROM1

在IROM1框中Start:写入**0x08003000**（APP1起始地址）

在IROM1框中Size:写入0x3E800（分配给这个工程的 Flash 总字节长度（十六进制））

`0x3E800` 转十进制 = **256000 字节**------256000 ÷ 1024 = 250 KB

### 3.APP2代码编写

APP2 的代码跟 APP1 的代码功能基本一致，只是细微处修改 BUG

注意要生成bin文件（参考Keil 自动生成 BIN 配置）

​	**通过这一步设置，我们就可以在 MDK 编译成功之后，调用 fromelf.exe（注意，我的 MDK 是安装在 E盘文件夹下，如果你是安装在其他目录，请根据你自己的目录修改 fromelf.exe 的路径），根据当前工程的 weather.axf（如果是其他的名字，请记住修改，这个文件存放在 Objects 目录下面，格式为 xxx.axf），生成一个 weather.bin 的文件。并存放在 axf 文件相同的目录下，即工程的 Objects 文件夹里面。在得到.bin 文件之后，我们只需要将这个 bin 文件传送给单片机，即可执行 IAP 升级。(我们也可以将 bin 文件无线发送，存放在 SD 卡内，存放在外部 FLASH 内等等方式进行代码升级，其中无线发送的形式叫 OTA)。**

​	我们把 APP2 生成的 bin 文件，通过串口，发送到 APP1 的运行设备上，就会自动的保存 APP2 的代码数据到对应的 Flash 地址下，那么按下复位按键后（也可以软件复位），再次运行 bootloader 代码，就会加载 APP2的数据到 APP1 的地址下，并运行新的程序。

## 五、核心底层原理：中断向量表偏移

芯片复位固定从`0x08000000`取向量表（**BOOT0=0 主闪存启动时**，BootLoader 的向量表就在这，对应《STM32上电启动方式学习笔记》的主闪存启动模式）；

APP1 起始地址偏移 0x3000，自身向量表在`0x08003000`；

**向量表前两个字决定程序能不能跑**：

- 第 1 个字（+0）：栈顶地址 `__initial_sp`，必须是 SRAM 地址（0x20000000 段）
- 第 2 个字（+4）：复位函数 `Reset_Handler` 入口，必须是 Flash 地址（0x08000000 段）

> 芯片复位后硬件固定从 0x08000000 取向量表，而 APP 在 0x08003000。**必须把 VTOR 改到 0x08003000，否则所有中断向量仍指向 Boot 区**，定时器、串口、PWM 中断一触发就跳错地方，甚至 HardFault。
>
> 注意配置时机：本套代码里 Boot 跳转前**不配 VTOR**，是 APP 的 main() 第一行自己配（`Flash_APP1/user/main.c:15`）；更稳妥的写法是在跳转函数里直接切（见第七章第 3 节修正版）。

VTOR 的两种写法（二选一）：

```c
SCB->VTOR = FLASH_BASE | 0x3000;   // ① 寄存器写法（本套代码使用，FLASH_BASE=0x08000000）
// SCB->VTOR = 0x08003000;         // 等价写法：直接给绝对地址

// ② 库函数写法（旧版标准外设库 misc.c）
// void NVIC_SetVectorTable(uint32_t NVIC_VectTab, uint32_t Offset);
NVIC_SetVectorTable(FLASH_BASE, 0x3000);
```

跳转 APP 的标准三步（**含合法性校验**，对照源码 `iap_load_app` 见第六章第 3 节）：

```c
#define APP1_ADDR  FLASH_APP1_ADDR   // 0x08003000（本套代码里的宏叫 FLASH_APP1_ADDR）

// 1. 校验栈顶在 SRAM、复位向量在 Flash（防止空白/损坏的 Flash 被当成程序执行）
uint32_t app_sp    = *(volatile uint32_t*)APP1_ADDR;
uint32_t app_reset = *(volatile uint32_t*)(APP1_ADDR+4);
if((app_sp & 0x2FFE0000) != 0x20000000) return;     // 栈顶必须是 SRAM 地址
if((app_reset & 0xFF000000) != 0x08000000) return;  // 复位向量必须是 Flash 地址

// 2. 设置 APP 的栈指针
__set_MSP(app_sp);

// 3. 跳到 APP 的复位函数（不返回）
((void(*)(void))app_reset)();
```

> 严格写法还要在跳转前**关全局中断 + 清 NVIC pending**（`__set_PRIMASK(1)` + 清 ICPR），否则跳转瞬间向量表还没切过去，来一个中断就跑错地址——完整可抄版本见第七章第 3 节。

## 六、源码实战解读：三份 IAP 工程的完整剖析

> 本节对应《15.IAP代码升级资料整理》里 `OTA_250K以内代码升级` 目录下的三份工程：
> `Flash_boot2`（Bootloader）、`Flash_APP1`（当前业务程序）、`Flash_APP2`（FreeRTOS+LVGL 新固件）。
> 前几节讲"该怎么做"，这一节对照**真实源码**逐段看"代码是怎么写的"，并核对前文几处说法。

### 1. 三个工程的职责与链接地址（map 实测）

| 工程 | 职责 | 关键源码 | 链接地址（map 实测） |
| --- | --- | --- | --- |
| Flash_boot2 | 判标志 → 搬运 → 跳转 | `user/main.c` | 0x08000000 |
| Flash_APP1 | 业务 + 串口接收新固件 | `user/main.c` + `user/api/usart.c` | 0x08003000 |
| Flash_APP2 | 新固件（FreeRTOS+LVGL） | `user/main.c`（同样内嵌接收逻辑） | 0x08003000 |

分区宏定义（`Flash_boot2/user/main.c:44-49`）与第三节表格一一对应：

```c
#define FLASH_APP1_ADDR  0x08003000      // APP1 起始地址
#define FLASH_APP2_ADDR  0x08041800      // APP2 起始地址（备份区）
#define APP2_FLAG_ADDR   (0x08080000-4)  // 升级标志位 = Flash 末尾 4 字节 0x0807FFFC
#define APP_SIZE  (0x3E800/STM_SECTOR_SIZE) // 125 个扇区（每页 2K）
#define APP_MAX_SIZE  0x3E800            // 250K
```

> **最重要的一点（和直觉相反）：APP2 备份区里的固件，链接地址是 0x08003000，不是它物理存放的 0x08041800。**
> 看两个工程的 map 文件：`Flash_APP1/project/Listings/weather.map:1415` 和 `Flash_APP2/project/Listings/weather.map:13861` 都是
> `Load Region LR_IROM1 (Base: 0x08003000, ...)`；Boot 才是 0x08000000（`Flash_boot2/project/Listings/weather.map:899`）。
> 因为 APP2 的固件最终要被**整体拷贝到 APP1 的位置去运行**，链接地址必须等于运行时地址。
> 这就解释了第四节"编译时修改地址属性"的为什么：两个 App 工程的 IROM1 都填 Start=0x08003000、Size=0x3E800。

### 2. Bootloader 上电执行流程（main.c 源码级）

`Flash_boot2/user/main.c:96`：

```c
int main(void)
{
    NVIC_SetPriorityGrouping(4);
    Led_Config(); Key_Config(); Beep_Config();
    Usart1_Config();
    printf("Bootloader程序执行\r\n");

    // 1. 读 APP2 末尾的标志位，判断是否有待更新的固件
    STMFLASH_Read(APP2_FLAG_ADDR, app2FlagBuff, 2);
    if(app2FlagBuff[0] == 0xAAAA && app2FlagBuff[1] == 0xAAAA) {
        printf("有更新程序,正在执行代码升级\r\n");
        UpdateFun();          // 2. 有：把 APP2 整体搬到 APP1
    } else {
        printf("没有新的程序,执行原有APP\r\n");
        UserFlashAppRun();    // 3. 没有：直接跳转执行 APP1
    }
    while (1) {               // 4. 走到这说明 APP 校验失败：LED 闪烁兜底
        LedToggle(Led1Port, Led1Pin); ... Delay_ms(200);
    }
}
```

- Boot 不干业务、**不接收数据**（它的 usart.c 里没有接收中断处理），只有三件事：**判标志 → 搬运 → 跳转**。
- 判断依据不是"APP1 坏没坏"，而是"APP2 里有没有新固件"——新固件先完整落进 APP2，再一次性切换，**升级中途断电也不影响 APP1 里正在跑的旧程序**。

`UpdateFun()`（`main.c:81`）逐扇区搬运 250K：

```c
for(uint16_t i=0; i<APP_SIZE; i++) {              // 125 个扇区
    STMFLASH_Read (FLASH_APP2_ADDR+i*STM_SECTOR_SIZE, readBuff, STM_SECTOR_SIZE/2);
    STMFLASH_Write(FLASH_APP1_ADDR+i*STM_SECTOR_SIZE, readBuff, STM_SECTOR_SIZE/2);
}
STMFLASH_Write(APP2_FLAG_ADDR, app2FlagBuff, 2);  // 写 0xFFFF 清除升级标志
UserFlashAppRun();                                // 搬运完立刻跳转
```

> 清标志这行的细节：Flash 写入只能把 1 改成 0，直接写 0xFFFF 盖不掉原来的 0xAAAA。`STMFLASH_Write` 内部会先读整页，发现不是 0xFFFF 就**整页擦除再写**，0xAAAA 才被真正清掉（等价于第四节说的"擦除 APP2 清除标志"，只是实现靠的是"检测非空页 → 整页擦"）。

### 3. 跳转 APP 三步曲（iap_load_app 核心）

`Flash_boot2/user/main.c:55`——比第五节的两步写法多了"栈顶合法性校验"，且栈指针用汇编直接改：

```c
void iap_load_app(u32 appxaddr)
{
    if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)  // ① 校验栈顶地址在 SRAM 范围
    {
        jump2app=(iapfun)*(vu32*)(appxaddr+4);       // ② 取复位向量（第 2 个字）
        MSR_MSP(*(vu32*)appxaddr);                   // ③ 设置 APP 栈指针（第 1 个字）
        jump2app();                                  // ④ 跳到 APP 的复位函数
    }
}
```

一个完整 ARM 固件的**向量表前两个字**：`+0` 是栈顶 `__initial_sp`（必须在 SRAM 0x20000000 段），`+4` 是复位函数 `Reset_Handler`（必须在 Flash 0x08000000 段）。所以"校验 + 跳转"等价于检查这个固件"像不像能跑的程序"：

- ① `&0x2FFE0000 == 0x20000000`：掩掉 SRAM 内部偏移，只认 0x20000000 段 → 栈顶合法
- ③ `MSR_MSP` 是手写汇编（`main.c:28`）：`MSR MSP, r0` 直接改写主栈指针，不能用普通 C 赋值替代
- ④ 跳转后 CPU 从 APP 的 Reset_Handler 开始执行，APP 内部自己重新初始化时钟/外设/中断

跳转前 `UserFlashAppRun()`（`main.c:67`）还会再校验一次复位向量，双重保险：

```c
if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)  // 复位地址高字节是 0x08
    iap_load_app(FLASH_APP1_ADDR);
else
    printf("APP程序加载失败!\r\n");   // 校验不过 → main 里 while(1) LED 闪烁兜底
```

### 4. APP 侧三件事：VTOR、擦备份区、串口边收边写

**（1）第一行改向量表偏移** —— `Flash_APP1/user/main.c:15`，即第五节原理的落地代码：

```c
SCB->VTOR = FLASH_BASE | 0x3000;   // 向量表从 0x08003000 开始
```

**（2）启动后先擦干净 APP2 区** —— `Flash_APP1/user/main.c:26`：

```c
if(STMFLASH_Erase(FLASH_APP2_ADDR, APP_MAX_SIZE) == 0)  // 擦掉整个备份区
```

Flash 写前必须擦除（擦完是 0xFFFF），所以每次升级前先把备份区清空，接收中断才能放心逐半字写入。

**（3）串口中断边收边写** —— `Flash_APP1/user/api/usart.c:41`：

```c
void USART1_IRQHandler(void)
{
    data = USART1->DR;                          // 读收到的字节
    recvBuff[recvNum%2] = data;                 // 两个字节轮流存
    if((recvNum%2) == 1) {                      // 凑齐 2 字节 → 拼成 16 位半字
        temp = ((u16)recvBuff[1]<<8) + recvBuff[0];   // 小端：低字节在前
        STMFLASH_WriteHalfWord(addr, temp);     // 直接写进 APP2 区
        addr += 2;
    }
    recvNum++;
    recvTime = 1;                               // 重置超时计时
}
```

- F1 的 Flash 编程粒度是**半字（16 位）**，固件按 2 字节一组收进来就拼好写掉，不需要大缓冲
- `addr` 从 `FLASH_APP2_ADDR`（0x08041800）连续递增——bin 里数据按链接地址连续排列，顺序写进去就是完整固件
- 收完判定靠"无新数据超时"：`RecvTimeOut()`（`usart.c:14`）由 SysTick 1ms 调用一次，`recvTime>=100` 置 `recvOver=1`；主循环 `RecvOverFun()`（`usart.c:25`）检测到后把 0xAAAA 写到标志位，提示按复位

### 5. 一次完整升级的操作时序

```text
电脑串口助手(发送 APP2 编译的 weather.bin，115200)
   │  ← 裸二进制流，无需协议头，直接发
   ▼
APP1 运行中: USART1 中断边收边写 → APP2 区(0x08041800 起)
   │  100ms 无新数据 → 判定接收完成 → 写 0xAAAA 到 0x0807FFFC → 提示按复位
   ▼
按复位键 / 重新上电
   ▼
Bootloader: 读 0x0807FFFC == 0xAAAA ?
   ├─ 是 → 搬运 APP2(250K) → APP1 → 清标志 → 跳转 APP1
   └─ 否 → 直接跳转 APP1
   ▼
新固件运行: 第一行改 VTOR → 擦除 APP2 区(为下次升级清场) → 正常业务
```

### 6. 前文说法 vs 源码实际（核对表）

| 前文说法 | 源码实际 | 说明 |
| --- | --- | --- |
| 标志位 `0xAAAAAAAA` | `app2FlagBuff[2] = {0xAAAA, 0xAAAA}`，半字分两次读写 | 4 字节内容相同（AA AA AA AA），只是实现是两次半字操作 |
| "串口空闲 200ms 判定接收完成" | `recvTime >= 100`，1ms 计数一次 → **100ms** | `Flash_APP1/user/api/usart.c:14` |
| "APP1 损坏则停留在 BootLoader 等待串口升级" | Boot 没有串口接收逻辑，损坏时打印"APP程序加载失败"后进 `while(1)` **LED 闪烁** | `Flash_boot2/user/main.c:119` |
| "有升级标志：擦除 APP1 → 拷贝 → 擦除 APP2 清标志" | 未显式擦 APP1；靠 `STMFLASH_Write` 内部"检测非空页 → 整页擦 → 重写"；清标志同理 | `Flash_boot2/user/main.c:81` |

### 7. 这套代码的坑与改进空间

| # | 问题 | 位置 | 改进建议 |
| --- | --- | --- | --- |
| 1 | **跳转前没有关中断**：`INTX_DISABLE/WFI_SET` 定义了但没调用，map 里显示被链接器移除（`Flash_boot2/project/Listings/weather.map:376`）。若 Boot 使能过外设中断，跳转瞬间向量表还没切到 APP，可能误入错误中断 | `main.c:17` | 跳转前调 `INTX_DISABLE()`，有条件再清 NVIC pending 位 |
| 2 | 接收完成靠 100ms 超时，**没有显式结束帧/应答/校验** | `usart.c:14` | 丢字节会导致坏固件 + 标志照写；产品级应加 CRC + 应答重传 |
| 3 | 中断里直接写 Flash，半字编程会阻塞；115200 波特率实测没问题，更高要评估 | `usart.c:41` | 改为中断收缓冲 + 主循环整包写入 |
| 4 | 升级中"写完标志、搬运前"断电 → 下次开机搬运坏固件 | — | 固件头加版本号/长度/CRC，搬运前先校验 |
| 5 | 升级成功与否无回显，无法确认新固件版本 | `main.c:81` | 固件头放版本号，跳转前打印 |

### 8. 升级失败速查（对应源码校验点）

| 现象 | 源码校验点 | 原因 |
| --- | --- | --- |
| 升级后 Boot 里 LED 反复闪烁 | `iap_load_app` 栈顶校验 / `UserFlashAppRun` 复位向量校验不过 | APP 的 IROM1 链接地址不是 0x08003000，或 bin 没烧对位置 |
| 程序能跑但串口/定时器中断卡死 | `SCB->VTOR = FLASH_BASE \| 0x3000` 缺失或写错 | 向量表还指向 0x08000000 的 Boot 区 |
| 跳转后偶发 HardFault | 跳转前未关中断（见第 7 节坑 1） | 中断向量表切换瞬间执行了错误中断 |

## 七、实战补充：从能跑到能落地

### 1. 出厂首次烧录（第一次怎么把程序弄进板子）

IAP 工程首次烧录**不是只烧一个文件**——Boot 和 APP1 是两个独立工程、烧两个地址：

| 烧录对象 | 工程 | 烧录地址 | 方法 |
| --- | --- | --- | --- |
| BootLoader | Flash_boot2 | 0x08000000（IROM1 Start 填 0x08000000） | ST-Link 直接下载（Keil 默认配置） |
| 当前运行程序 | Flash_APP1 | 0x08003000（IROM1 Start 填 0x08003000） | ST-Link 直接下载，Keil 按工程配置的地址烧 |

操作要点：

1. 先烧 Boot：打开 Flash_boot2 工程，编译后直接 Download（它的 IROM1 是 0x08000000，默认不用改）；
2. 再烧 APP1：打开 Flash_APP1 工程，**确认 IROM1 Start=0x08003000** 再下载——Keil 会把代码烧到 0x08003000，而不是覆盖 Boot；
3. 两段烧完复位即可运行（Boot 无升级标志 → 直接跳 APP1）；
4. **千万别用 APP1 的工程去烧 Boot 区**，会把 Boot 覆盖掉；Boot 工程下载也只写 0x08000000 起的一段；
5. 验证：读 Flash，0x08000000 和 0x08003000 处都应有合法向量表（前 4 字节是 0x2000xxxx 栈顶）。

> 串口 IAP 升级只是“换 APP1”，Boot 用 ST-Link 烧一次就不用再动（除非改 Boot 本身）。

### 2. 升级失败与“变砖”恢复

这套代码的局限：**Boot 不接收串口、没有强制升级模式**。两个风险场景：

- APP1 区数据损坏 + 无升级标志 → Boot 校验失败 → `while(1)` LED 闪烁，此时**无法用串口救回来**，只能 ST-Link 重烧；
- “写完标志、搬运前”断电 → 下次开机搬运的是坏固件。

产品级恢复方案（可组合）：

1. **Boot 内置强制升级模式**：上电检测按键（如按住 KEY 再复位）→ 进入 Boot 的串口接收程序，直接把新固件写进 APP1 区——最通用；
2. **Boot 也接收串口**：在 Boot 里加一段和 APP1 一样的串口写入逻辑，平时不启用，仅在 APP 校验失败时启用；
3. **固件头校验**：在 bin 头部放“魔数 + 版本号 + 长度 + CRC”，Boot 搬运前后校验，不过就丢弃 APP2 并清标志，保证永远只执行完整固件；
4. **看门狗兜底**：Boot/APP 开 IWDG，程序跑飞（含升级中断电后状态错乱）自动复位重新走 Boot 流程。

### 3. 标准跳转函数（修正版，可直接抄）

对照第六章第 7 节“坑 1”，跳转前应**关中断 + 清 pending**，APP 侧在 main 最开头重设 VTOR：

```c
// —— Boot 侧：跳转 APP（修正版）——
typedef void (*iapfun)(void);

void iap_load_app(u32 appxaddr)
{
    if(((*(vu32*)appxaddr) & 0x2FFE0000) != 0x20000000) return;     // ① 栈顶必须在 SRAM
    if(((*(vu32*)(appxaddr+4)) & 0xFF000000) != 0x08000000) return; // ② 复位向量必须在 Flash

    __set_PRIMASK(1);              // ③ 关全局中断（等价 INTX_DISABLE）
    SCB->VTOR = appxaddr;          // ④ 直接切向量表（也可以留给 APP 自己设）

    // 清掉所有挂起的中断，防止跳转瞬间执行错误中断
    for(uint32_t i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;   // 关闭全部外设中断
        NVIC->ICPR[i] = 0xFFFFFFFF;   // 清全部 pending
    }

    __set_MSP(*(vu32*)appxaddr);         // ⑤ 设 APP 栈指针
    ((iapfun)(*(vu32*)(appxaddr+4)))();  // ⑥ 跳复位函数（不返回）
}
```

```c
// —— APP 侧：main 第一行（推荐写法）——
SCB->VTOR = 0x08003000;   // 直接给绝对地址，不依赖 FLASH_BASE 宏
```

> 对比原代码：原版少了 ③④ 两步（关中断、清 pending）。SysTick、串口等外设中断如果在跳转瞬间触发，向量表还没切过去，就会跑到 Boot 的中断处理（或错误地址）——偶发 HardFault 的根源。

### 4. 软件复位入口（升级完不用手动按复位）

```c
// 写完升级标志后，直接软件复位进 Boot，无需人按按键
NVIC_SystemReset();   // CMSIS 提供的函数：触发系统复位
```

> 本代码是串口收完、写标志后打印“核对数据无误后，请按下复位按键进行数据更新”，让用户手动复位；产品上一般直接 `NVIC_SystemReset()` 自动完成。注意复位前确保串口发送完成（TC 标志），否则最后一句提示可能发不出去。

### 5. OTA 无线升级扩展（APP2 里的 ESP8266 代码）

APP2（FreeRTOS+LVGL 工程）里其实带了 ESP8266 的配网代码（`Flash_APP2/user/main.c` 的 `Device_SetMode()`，main 里被注释）：

- 按 KEY4 进入设置模式 → ESP8266 配成 AP（`AT+CWMODE=2`，热点 `XYD_WifiWeather/12345678`）→ 手机连上 → APP 通过 8080 端口下发配置；
- 无线升级的底层逻辑和串口 IAP **完全一致**：只是把“数据来源”从串口换成 WiFi 收包，收到 bin 后一样写 APP2 区 → 置标志 → 复位 → Boot 搬运。

OTA 与 IAP 的关系（呼应第二节）：**数据通道不同，Flash 操作相同**。做 OTA 只需：

1. 把串口接收中断换成 WiFi/TCP 收包逻辑（esp8266.c 的 +IPD 数据解析）；
2. 加一个“下载到一半失败 → 放弃并清 APP2”的容错；
3. 其余（写 APP2、置标志、Boot 搬运、跳转）原样复用。

### 6. 升级过程调试验证方法

| 步骤 | 做法 | 预期结果 |
| --- | --- | --- |
| 1 | 串口助手 115200，发 APP2 的 weather.bin（选“发送文件”直接发） | 设备打印“APP数据接收完成:xxxxx” |
| 2 | 等 100ms 超时 | 出现“核对数据无误后，请按下复位按键进行数据更新” |
| 3 | 按复位 | Boot 打印“有更新程序,正在执行代码升级”→“固件更新完成!”→“开始执行FLASH用户代码!!” |
| 4 | 看新程序打印（APP2 的 printf 内容变了） | 确认升级成功 |
| 5 | 读 Flash 0x0807FFFC | 值应为 0xFFFF（标志已被清掉） |
| 失败排查 | Boot 打印“APP程序加载失败” | 核对 APP1 区 0x08003000 向量表，或对照第六章第 8 节速查表 |

> 注意：发 bin 时不要勾“发送新行”（\r\n 会被当成固件数据写进 Flash）；串口助手按字节原样发送。

## 八、产品级 IAP 源码模板（模块化）

> 前面各章用的是老师例程：裸数据流、边收边写、无校验、无应答。本节给一套**产品级模板**：帧协议 + CRC + 应答重传、固件头校验、整包 CRC32、超时保护、强制升级模式、跳转前关中断。
> 6 个模块相互独立，可直接拷进 Keil 工程（STM32F103 + 标准外设库环境）。

### 1. 模块总览与设计

| 文件 | 职责 | 谁在用 |
| --- | --- | --- |
| `iap_config.h` | 分区/协议/固件头/底层接口，**全工程唯一配置点** | 所有模块 |
| `iap_crc.c/h` | CRC16（帧校验）+ CRC32（固件校验） | protocol / core / boot |
| `iap_flash.c/h` | Flash 分区擦/写/读/回读校验，**Boot 区写保护** | core / boot |
| `iap_protocol.c/h` | 帧解析状态机（串口中断逐字节喂入）+ 应答 + 组帧 | APP 与 Boot 共用 |
| `iap_core.c/h` | 升级状态机：START→DATA→END，页缓存整页写 Flash | APP 与 Boot 强制模式 |
| `iap_boot.c/h` | Boot 引导：查标志 → 校验固件头 → 搬运 → 跳转 | 仅 Boot |

与老师例程的对比（对应第六章）：

| 老师例程 | 产品级模板 |
| --- | --- |
| 裸数据流，无协议 | 帧头 + 命令 + 长度 + CRC16，每帧 ACK/NACK |
| 中断里边收边写，无校验 | 中断只收字节，主循环整页写；整包 CRC32 + 回读 |
| 100ms 静默判定结束 | 帧级超时 + 断点重传（上位机重发当前帧） |
| 无固件头 | 16B 固件头：魔数 / 版本 / 长度 / CRC32 |
| 升级失败只能 ST-Link 重烧 | Boot 强制升级模式（按键救砖） |
| 跳转不关中断 | 关全局中断 + 清 pending + 切 VTOR |

执行架构（**中断只收，主循环干活**——避免在中断里擦写 Flash）：

```text
串口中断 ──逐字节──▶ iap_protocol(解析) ──完整帧──▶ 主循环 IAP_Task
                                                        ├─ START: 校验固件头 → 整区擦除
                                                        ├─ DATA : 攒入 2K 页缓存 → 满页写 Flash
                                                        └─ END  : 尾页落盘 → 全量 CRC32 → 写头 → 置标志
```

### 2. 升级协议设计

帧格式（大端命令，小端长度/CRC）：

```text
[AA 55] [CMD:1] [LEN_L:1] [LEN_H:1] [DATA:0~1024] [CRC_L:1] [CRC_H:1]
```

| 命令 | 值 | 帧数据 | 作用 |
| --- | --- | --- | --- |
| START | 0x01 | 16B 固件头 | 校验头 → 整区擦除 → 写固件头 → 进入接收态 |
| DATA | 0x02 | ≤1024B bin 数据 | 顺序发送，页满即写 Flash |
| END | 0x03 | 无 | 尾页落盘 → 全量 CRC32 → 写固件头 → 置升级标志 |
| ABORT | 0x04 | 无 | 放弃本次升级，清状态 |

- **CRC16**（CCITT，覆盖 CMD+LEN+DATA，不含帧头）由 MCU 校验，错则回 NACK；
- **应答**：每帧处理完回 `ACK(0x06)+状态码` 或 `NACK(0x15)+错误码`，上位机等到应答才发下一帧，超时（如 500ms）重发当前帧——这就是断点重传；
- **固件头 16B**：`magic(0x31504149 "IAP1") + version + length + crc32`，MCU 收到 END 后对 `length` 字节数据全量算 CRC32 比对。

### 3. iap_config.h（配置 + 底层接口，全工程唯一改这里）

```c
#ifndef __IAP_CONFIG_H
#define __IAP_CONFIG_H

#include "stm32f10x.h"
#include <string.h>

/* ========== Flash 分区（512K F103：Boot 12K + APP1 250K + APP2 250K） ========== */
#define IAP_BOOT_ADDR       0x08000000UL   /* BootLoader 区（驱动层禁止擦写） */
#define IAP_APP1_ADDR       0x08003000UL   /* APP1 运行区 */
#define IAP_APP2_ADDR       0x08041800UL   /* APP2 备份区（新固件暂存） */
#define IAP_APP_SIZE        0x3E800UL      /* 每个 APP 区 250K */
#define IAP_SECTOR_SIZE     2048UL         /* F103 大容量 Flash 每页 2K */

/* 升级标志区：Flash 末尾 8 字节，双字保险（AA AA + 55 55 同时成立才算有更新） */
#define IAP_FLAG_ADDR       (0x08080000UL - 8UL)

/* ========== 固件头（16 字节，位于 bin 最前面） ========== */
#define IAP_FW_MAGIC        0x31504149UL   /* "IAP1" */
#define IAP_FW_HEADER_SIZE  16UL

typedef struct {
    uint32_t magic;    /* IAP_FW_MAGIC */
    uint32_t version;  /* 固件版本号 */
    uint32_t length;   /* 固件数据长度（不含头） */
    uint32_t crc32;    /* 固件数据全量 CRC32 */
} iap_fw_header_t;

/* ========== 串口协议参数 ========== */
#define IAP_FRAME_H1        0xAA
#define IAP_FRAME_H2        0x55
#define IAP_CMD_START       0x01
#define IAP_CMD_DATA        0x02
#define IAP_CMD_END         0x03
#define IAP_CMD_ABORT       0x04
#define IAP_ACK             0x06
#define IAP_NACK            0x15
#define IAP_DATA_MAX        1024    /* 单帧数据最大字节数 */

/* 接收超时：无新帧超过该毫秒数判定失败 */
#define IAP_RX_TIMEOUT      500

/* 看门狗喂狗钩子：升级/搬运用时长，用户可在这里调 IWDG_ReloadCounter() */
#define IAP_FEED_WDG()

/* ========== 底层接口：用户在串口驱动里实现 ========== */
void     IAP_Uart_SendByte(uint8_t byte);   /* 发送 1 字节 */
uint32_t IAP_GetTick(void);                 /* 毫秒 tick（SysTick 1ms++） */

/* ========== 错误码 ========== */
typedef enum {
    IAP_OK = 0,
    IAP_ERR_BAD_HEADER,   /* 帧头/固件头错误 */
    IAP_ERR_BAD_CRC,      /* 帧 CRC16 错误 */
    IAP_ERR_BAD_LEN,      /* 长度错误 */
    IAP_ERR_FW_CRC,       /* 固件 CRC32 校验失败 */
    IAP_ERR_FW_TOO_BIG,   /* 固件超长 */
    IAP_ERR_FLASH,        /* Flash 操作失败 */
    IAP_ERR_TIMEOUT,      /* 接收超时 */
    IAP_ERR_BUSY          /* 状态机未就绪 */
} iap_err_t;

#endif
```

### 4. iap_crc.c / iap_crc.h

```c
/* ================= iap_crc.h ================= */
#ifndef __IAP_CRC_H
#define __IAP_CRC_H
#include "stm32f10x.h"
uint16_t IAP_CRC16(const uint8_t *buf, uint16_t len);   /* CCITT，poly 0x1021 */
uint32_t IAP_CRC32(const uint8_t *buf, uint32_t len);   /* IEEE 802.3 */
#endif
```

```c
/* ================= iap_crc.c ================= */
#include "iap_crc.h"

/* CRC16-CCITT（XModem）：初值 0x0000，多项式 0x1021，MSB 移位 */
uint16_t IAP_CRC16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0x0000;
    while(len--) {
        crc ^= (uint16_t)(*buf++) << 8;
        for(uint8_t i = 0; i < 8; i++) {
            if(crc & 0x8000) crc = (crc << 1) ^ 0x1021;
            else             crc <<= 1;
        }
    }
    return crc;
}

/* CRC32（IEEE 802.3/ZIP）：初值 0xFFFFFFFF，多项式 0x04C11DB7，结果异或 0xFFFFFFFF */
uint32_t IAP_CRC32(const uint8_t *buf, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFUL;
    while(len--) {
        crc ^= *buf++;
        for(uint8_t i = 0; i < 8; i++) {
            if(crc & 0x80000000UL) crc = (crc << 1) ^ 0x04C11DB7UL;
            else                   crc <<= 1;
        }
    }
    return crc ^ 0xFFFFFFFFUL;
}
```

> 说明：这里给的是位运算法（代码短、不易抄错），250K 固件全量 CRC32 约耗时 0.5s，可接受；追求速度可换成查表法（表驱动约快 10 倍）。

### 5. iap_flash.c / iap_flash.h

```c
/* ================= iap_flash.h ================= */
#ifndef __IAP_FLASH_H
#define __IAP_FLASH_H
#include "iap_config.h"
uint8_t IAP_Flash_ErasePage(uint32_t addr);                 /* 擦 1 页，仅限 APP 区 */
uint8_t IAP_Flash_EraseArea(uint32_t addr, uint32_t size);  /* 擦连续区（2K 对齐） */
uint8_t IAP_Flash_Write(uint32_t addr, const uint8_t *buf, uint32_t len); /* 任意长度写，页首对齐自动擦页 */
void    IAP_Flash_Read(uint32_t addr, uint8_t *buf, uint32_t len);
uint8_t IAP_Flash_Verify(uint32_t addr, const uint8_t *buf, uint32_t len); /* 回读比对 */
#endif
```

```c
/* ================= iap_flash.c ================= */
#include "iap_flash.h"
#include "stm32f10x_flash.h"

/* 可操作范围：APP1 ~ APP2 末尾；Boot 区（0x08000000~0x08002FFF）受保护，杜绝误擦 */
#define IAP_AREA_BEGIN  IAP_APP1_ADDR
#define IAP_AREA_END    (IAP_APP1_ADDR + 2UL * IAP_APP_SIZE)

static int IAP_Flash_CheckRange(uint32_t addr, uint32_t len)
{
    return (addr >= IAP_AREA_BEGIN) && (addr + len <= IAP_AREA_END);
}

uint8_t IAP_Flash_ErasePage(uint32_t addr)
{
    if(!IAP_Flash_CheckRange(addr, IAP_SECTOR_SIZE)) return IAP_ERR_FLASH;
    if(addr % IAP_SECTOR_SIZE)                       return IAP_ERR_BAD_LEN;
    FLASH_Unlock();
    FLASH_ErasePage(addr);
    FLASH_Lock();
    return IAP_OK;
}

uint8_t IAP_Flash_EraseArea(uint32_t addr, uint32_t size)
{
    if(!IAP_Flash_CheckRange(addr, size))                            return IAP_ERR_FLASH;
    if((addr % IAP_SECTOR_SIZE) || (size % IAP_SECTOR_SIZE))         return IAP_ERR_BAD_LEN;
    FLASH_Unlock();
    while(size) {
        FLASH_ErasePage(addr);
        addr += IAP_SECTOR_SIZE;
        size -= IAP_SECTOR_SIZE;
    }
    FLASH_Lock();
    return IAP_OK;
}

uint8_t IAP_Flash_Write(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    if(!IAP_Flash_CheckRange(addr, len)) return IAP_ERR_FLASH;
    FLASH_Unlock();
    if(addr % IAP_SECTOR_SIZE == 0) FLASH_ErasePage(addr);  /* 页首对齐：先擦整页（搬运场景） */
    while(len >= 2) {                                       /* F1 按半字编程 */
        uint16_t hw = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
        FLASH_ProgramHalfWord(addr, hw);
        addr += 2; buf += 2; len -= 2;
    }
    if(len) FLASH_ProgramHalfWord(addr, (uint16_t)buf[0]); /* 奇数尾字节 */
    FLASH_Lock();
    return IAP_OK;
}

void IAP_Flash_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    while(len--) { *buf++ = *(volatile uint8_t *)addr; addr++; }
}

uint8_t IAP_Flash_Verify(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    while(len--) {
        if(*(volatile uint8_t *)addr++ != *buf++) return IAP_ERR_BAD_CRC;
    }
    return IAP_OK;
}
```

### 6. iap_protocol.c / iap_protocol.h（帧解析，中断喂字节）

```c
/* ================= iap_protocol.h ================= */
#ifndef __IAP_PROTOCOL_H
#define __IAP_PROTOCOL_H
#include "iap_config.h"
void IAP_Protocol_Init(void);
void IAP_Protocol_Feed(uint8_t byte);                       /* 串口中断逐字节喂入（只解析，不干活） */
int  IAP_Protocol_Poll(uint8_t *cmd, uint8_t **data, uint16_t *len);  /* 主循环取完整帧，1=有 */
void IAP_Protocol_SendAck(uint8_t ack, uint8_t status);
void IAP_Protocol_SendFrame(uint8_t cmd, const uint8_t *data, uint16_t len); /* 组帧发送 */
#endif
```

```c
/* ================= iap_protocol.c ================= */
#include "iap_protocol.h"
#include "iap_crc.h"

typedef enum { ST_H1, ST_H2, ST_CMD, ST_LEN_L, ST_LEN_H, ST_DATA, ST_CRC_L, ST_CRC_H } iap_ps_t;

typedef struct {
    iap_ps_t st;
    uint8_t  cmd;
    uint16_t len, cnt, crc;
    uint8_t  data[IAP_DATA_MAX];
    volatile uint8_t ready;                /* 有完整帧待消费 */
    uint8_t  readyCmd;
    uint16_t readyLen;
    uint8_t  readyData[IAP_DATA_MAX];      /* 帧数据缓存，防覆盖 */
} iap_parser_t;

static iap_parser_t s;

/* 单字节 CRC16 递推（避免在中断里开大数组） */
static uint16_t IAP_CRC16_Step(uint16_t crc, uint8_t byte)
{
    crc ^= (uint16_t)byte << 8;
    for(uint8_t i = 0; i < 8; i++) {
        if(crc & 0x8000) crc = (crc << 1) ^ 0x1021;
        else             crc <<= 1;
    }
    return crc;
}

void IAP_Protocol_Init(void) { memset(&s, 0, sizeof(s)); }

void IAP_Protocol_Feed(uint8_t byte)   /* 串口中断里调用：只解析、校验、暂存 */
{
    switch(s.st) {
    case ST_H1:   s.st = (byte == IAP_FRAME_H1) ? ST_H2 : ST_H1; break;
    case ST_H2:   s.st = (byte == IAP_FRAME_H2) ? ST_CMD : ST_H1; break;
    case ST_CMD:  s.cmd = byte; s.st = ST_LEN_L; break;
    case ST_LEN_L: s.len = byte; s.st = ST_LEN_H; break;
    case ST_LEN_H:
        s.len |= (uint16_t)byte << 8;
        if(s.len > IAP_DATA_MAX) {                    /* 非法长度：NACK 后丢弃 */
            IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_BAD_LEN);
            s.st = ST_H1;
        } else {
            s.cnt = 0;
            s.st = s.len ? ST_DATA : ST_CRC_L;
        }
        break;
    case ST_DATA: s.data[s.cnt++] = byte;
        if(s.cnt >= s.len) s.st = ST_CRC_L;
        break;
    case ST_CRC_L: s.crc = byte; s.st = ST_CRC_H; break;
    case ST_CRC_H:
        s.crc |= (uint16_t)byte << 8;
        {
            uint16_t calc = 0;
            calc = IAP_CRC16_Step(calc, s.cmd);                     /* CRC 覆盖 CMD+LEN+DATA */
            calc = IAP_CRC16_Step(calc, (uint8_t)s.len);
            calc = IAP_CRC16_Step(calc, (uint8_t)(s.len >> 8));
            for(uint16_t i = 0; i < s.len; i++) calc = IAP_CRC16_Step(calc, s.data[i]);
            if(calc != s.crc) {
                IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_BAD_CRC);
            } else {
                s.readyCmd = s.cmd;
                s.readyLen = s.len;
                memcpy(s.readyData, s.data, s.len);
                s.ready = 1;
            }
        }
        s.st = ST_H1;
        break;
    default: s.st = ST_H1; break;
    }
}

int IAP_Protocol_Poll(uint8_t *cmd, uint8_t **data, uint16_t *len)
{
    if(!s.ready) return 0;
    *cmd  = s.readyCmd;
    *data = s.readyData;
    *len  = s.readyLen;
    s.ready = 0;
    return 1;
}

void IAP_Protocol_SendAck(uint8_t ack, uint8_t status)
{
    IAP_Uart_SendByte(ack);
    IAP_Uart_SendByte(status);
}

void IAP_Protocol_SendFrame(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0;
    IAP_Uart_SendByte(IAP_FRAME_H1);
    IAP_Uart_SendByte(IAP_FRAME_H2);
    IAP_Uart_SendByte(cmd);
    IAP_Uart_SendByte((uint8_t)len);
    IAP_Uart_SendByte((uint8_t)(len >> 8));
    crc = IAP_CRC16_Step(crc, cmd);
    crc = IAP_CRC16_Step(crc, (uint8_t)len);
    crc = IAP_CRC16_Step(crc, (uint8_t)(len >> 8));
    for(uint16_t i = 0; i < len; i++) {
        IAP_Uart_SendByte(data[i]);
        crc = IAP_CRC16_Step(crc, data[i]);
    }
    IAP_Uart_SendByte((uint8_t)crc);
    IAP_Uart_SendByte((uint8_t)(crc >> 8));
}
```

### 7. iap_core.c / iap_core.h（升级状态机，APP 与 Boot 强制模式共用）

```c
/* ================= iap_core.h ================= */
#ifndef __IAP_CORE_H
#define __IAP_CORE_H
#include "iap_config.h"
void IAP_Init(uint32_t targetAddr);   /* targetAddr：APP2（正常升级）或 APP1（Boot 强制模式） */
void IAP_Task(void);                  /* 主循环周期调用：取帧 → 写页 → 校验 → 置标志 */
int  IAP_IsUpgraded(void);            /* 本次升级完成？返回 1 后可软件复位 */
#endif
```

```c
/* ================= iap_core.c ================= */
#include "iap_core.h"
#include "iap_flash.h"
#include "iap_crc.h"
#include "iap_protocol.h"

typedef enum { IAP_IDLE, IAP_RECEIVING, IAP_READY } iap_state_t;

static iap_state_t      g_state;
static uint32_t         g_target;                      /* 升级目标区 */
static iap_fw_header_t  g_hdr;                         /* 固件头（RAM 副本） */
static uint8_t          g_pageBuf[IAP_SECTOR_SIZE];    /* 2K 页缓存 */
static uint32_t         g_pageAddr;                    /* 当前页写地址 */
static uint16_t         g_pageCnt;                     /* 页内已攒字节 */
static uint32_t         g_wrLen;                       /* 已收固件数据长度 */
static uint32_t         g_lastTick;                    /* 最后收到帧的时刻 */

/* 尾页落盘：不足 2K 的尾部补 0xFF 写整页 */
static void IAP_FlushPage(void)
{
    if(g_pageCnt) {
        memset(g_pageBuf + g_pageCnt, 0xFF, IAP_SECTOR_SIZE - g_pageCnt);
        IAP_Flash_Write(g_pageAddr, g_pageBuf, IAP_SECTOR_SIZE);
        g_pageCnt = 0;
        g_pageAddr += IAP_SECTOR_SIZE;
    }
}

static void IAP_OnStart(uint8_t *data, uint16_t len)
{
    if(len != IAP_FW_HEADER_SIZE) { IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_BAD_LEN); return; }
    memcpy(&g_hdr, data, IAP_FW_HEADER_SIZE);
    if(g_hdr.magic != IAP_FW_MAGIC) { IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_BAD_HEADER); return; }
    if(g_hdr.length == 0 || g_hdr.length > IAP_APP_SIZE - IAP_FW_HEADER_SIZE) {
        IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_FW_TOO_BIG); return; }
    if(IAP_Flash_EraseArea(g_target, IAP_APP_SIZE) != IAP_OK) {
        IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_FLASH); return; }
    /* 固件头立即落盘：该页刚擦过，先写 16B 头；DATA 从 +16 开始写，互不冲突 */
    if(IAP_Flash_Write(g_target, (uint8_t *)&g_hdr, IAP_FW_HEADER_SIZE) != IAP_OK) {
        IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_FLASH); return; }
    g_pageAddr = g_target + IAP_FW_HEADER_SIZE;         /* 数据从固件头之后开始写 */
    g_pageCnt  = 0;
    g_wrLen    = 0;
    g_state    = IAP_RECEIVING;
    IAP_Protocol_SendAck(IAP_ACK, IAP_OK);
}

static void IAP_OnData(uint8_t *data, uint16_t len)
{
    if(g_state != IAP_RECEIVING) { IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_BUSY); return; }
    if(g_wrLen + len > g_hdr.length) {                  /* 超长：直接废掉本次升级 */
        g_state = IAP_IDLE;
        IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_BAD_LEN); return; }
    for(uint16_t i = 0; i < len; i++) {
        g_pageBuf[g_pageCnt++] = data[i];
        if(g_pageCnt == IAP_SECTOR_SIZE) {              /* 攒满 2K，整页写 Flash */
            if(IAP_Flash_Write(g_pageAddr, g_pageBuf, IAP_SECTOR_SIZE) != IAP_OK) {
                g_state = IAP_IDLE;
                IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_FLASH); return; }
            g_pageCnt  = 0;
            g_pageAddr += IAP_SECTOR_SIZE;
        }
    }
    g_wrLen += len;
    IAP_Protocol_SendAck(IAP_ACK, IAP_OK);
}

static void IAP_OnEnd(void)
{
    if(g_state != IAP_RECEIVING) { IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_BUSY); return; }
    if(g_wrLen != g_hdr.length) {                       /* 收的字节数对不上：放弃 */
        g_state = IAP_IDLE;
        IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_BAD_LEN); return; }

    IAP_FlushPage();                                    /* 1. 尾页落盘 */

    /* 2. 全量 CRC32 校验固件数据（固件头已在 START 时落盘，这里不再动 Flash 数据区） */
    if(IAP_CRC32((uint8_t *)(g_target + IAP_FW_HEADER_SIZE), g_hdr.length) != g_hdr.crc32) {
        g_state = IAP_IDLE;
        IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_FW_CRC); return; }

    /* 3. 置升级标志：先擦标志页，再写 0xAAAA + 0x5555（双保险） */
    {
        uint16_t f1 = 0xAAAA, f2 = 0x5555;
        IAP_Flash_ErasePage(IAP_FLAG_ADDR & ~(IAP_SECTOR_SIZE - 1));
        IAP_Flash_Write(IAP_FLAG_ADDR,     (uint8_t *)&f1, 2);
        IAP_Flash_Write(IAP_FLAG_ADDR + 2, (uint8_t *)&f2, 2);
    }

    g_state = IAP_READY;
    IAP_Protocol_SendAck(IAP_ACK, IAP_OK);
}

static void IAP_OnAbort(void)
{
    g_state = IAP_IDLE;
    g_pageCnt = 0;
    IAP_Protocol_SendAck(IAP_ACK, IAP_OK);
}

void IAP_Task(void)   /* 主循环周期调用（1ms 或 1~10ms 均可） */
{
    uint8_t  cmd;
    uint8_t *data;
    uint16_t len;

    if(IAP_Protocol_Poll(&cmd, &data, &len)) {          /* 有完整帧，主循环上下文处理 */
        g_lastTick = IAP_GetTick();
        switch(cmd) {
        case IAP_CMD_START: IAP_OnStart(data, len); break;
        case IAP_CMD_DATA:  IAP_OnData(data, len);  break;
        case IAP_CMD_END:   IAP_OnEnd();            break;
        case IAP_CMD_ABORT: IAP_OnAbort();          break;
        default: IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_BAD_HEADER); break;
        }
    }
    if(g_state == IAP_RECEIVING) {                      /* 接收超时保护 */
        if(IAP_GetTick() - g_lastTick > IAP_RX_TIMEOUT) {
            g_state = IAP_IDLE;
            g_pageCnt = 0;
            IAP_Protocol_SendAck(IAP_NACK, IAP_ERR_TIMEOUT);
        }
    }
}

void IAP_Init(uint32_t targetAddr)
{
    g_target = targetAddr;
    g_state  = IAP_IDLE;
    IAP_Protocol_Init();
}

int IAP_IsUpgraded(void) { return g_state == IAP_READY; }
```

### 8. iap_boot.c / iap_boot.h（Boot 引导流程）

```c
/* ================= iap_boot.h ================= */
#ifndef __IAP_BOOT_H
#define __IAP_BOOT_H
#include "iap_config.h"
int  IAP_Boot_CheckUpgrade(void);        /* 读标志 + 校验 APP2 固件头，1=有待更新的完整固件 */
void IAP_Boot_MoveFw(void);              /* APP2 → APP1 逐页搬运 + 清标志 */
void IAP_Boot_ClearFlag(void);           /* 清升级标志（擦标志页） */
void IAP_Boot_LoadApp(uint32_t appAddr); /* 校验向量表 + 关中断 + 切 VTOR + 跳转（失败不返回跳转） */
#endif
```

```c
/* ================= iap_boot.c ================= */
#include "iap_boot.h"
#include "iap_flash.h"
#include "iap_crc.h"

int IAP_Boot_CheckUpgrade(void)
{
    uint16_t f1, f2;
    iap_fw_header_t hdr;

    IAP_Flash_Read(IAP_FLAG_ADDR,     (uint8_t *)&f1, 2);
    IAP_Flash_Read(IAP_FLAG_ADDR + 2, (uint8_t *)&f2, 2);
    if(f1 != 0xAAAA || f2 != 0x5555) return 0;          /* 标志不完整：不升级 */

    IAP_Flash_Read(IAP_APP2_ADDR, (uint8_t *)&hdr, IAP_FW_HEADER_SIZE);
    if(hdr.magic != IAP_FW_MAGIC)                                    return 0;
    if(hdr.length == 0 || hdr.length > IAP_APP_SIZE - IAP_FW_HEADER_SIZE) return 0;
    if(IAP_CRC32((uint8_t *)(IAP_APP2_ADDR + IAP_FW_HEADER_SIZE), hdr.length) != hdr.crc32) return 0;
    return 1;
}

void IAP_Boot_MoveFw(void)
{
    uint8_t  buf[512];
    uint32_t src = IAP_APP2_ADDR;
    uint32_t dst = IAP_APP1_ADDR;
    uint32_t left = IAP_APP_SIZE;

    while(left) {
        uint32_t n = (left > sizeof(buf)) ? (uint32_t)sizeof(buf) : left;
        IAP_Flash_Read(src, buf, n);
        if(IAP_Flash_Write(dst, buf, n) != IAP_OK) break;  /* 页首对齐时自动擦页 */
        src += n; dst += n; left -= n;
        IAP_FEED_WDG();                                     /* 搬运耗时长，必须喂狗 */
    }
    IAP_Boot_ClearFlag();
}

void IAP_Boot_ClearFlag(void)
{
    IAP_Flash_ErasePage(IAP_FLAG_ADDR & ~(IAP_SECTOR_SIZE - 1));
}

void IAP_Boot_LoadApp(uint32_t appAddr)
{
    uint32_t sp = *(volatile uint32_t *)appAddr;
    uint32_t rv = *(volatile uint32_t *)(appAddr + 4);

    if((sp & 0x2FFE0000UL) != 0x20000000UL) return;     /* 栈顶不在 SRAM：不跳 */
    if((rv & 0xFF000000UL) != 0x08000000UL) return;     /* 复位向量不在 Flash：不跳 */

    __set_PRIMASK(1);                                   /* 关全局中断 */
    SCB->VTOR = appAddr;                                /* 切向量表 */
    for(uint32_t i = 0; i < 8; i++) {                   /* 清全部外设中断与挂起 */
        NVIC->ICER[i] = 0xFFFFFFFFUL;
        NVIC->ICPR[i] = 0xFFFFFFFFUL;
    }
    __set_MSP(sp);                                      /* 设 APP 栈指针 */
    ((void (*)(void))rv)();                             /* 跳 APP 复位函数（不返回） */
}
```

### 9. 集成示例（APP / Boot / 上位机）

**APP 侧接线**（用户 main + 串口中断）：

```c
/* ---- 串口中断里只加一行 ---- */
void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
        IAP_Protocol_Feed((uint8_t)(USART1->DR & 0xFF));   /* 解析在中断，干活在主循环 */
    }
}

/* ---- 用户 main（APP1/APP2 通用） ---- */
#include "iap_core.h"
#include "iap_protocol.h"

int main(void)
{
    SCB->VTOR = 0x08003000;
    ...外设初始化（SysTick 1ms tick 必须开）...
    IAP_Init(IAP_APP2_ADDR);          /* 正常升级：新固件收进 APP2 备份区 */
    while(1) {
        IAP_Task();                   /* 周期调用：收帧→写页→CRC→置标志 */
        if(IAP_IsUpgraded()) {
            printf("升级完成，软件复位...\r\n");
            NVIC_SystemReset();       /* 自动进 Boot 完成搬运 */
        }
        ...业务代码...
    }
}
```

**Boot 侧接线**（用户 main）：

```c
#include "iap_boot.h"
#include "iap_core.h"
#include "iap_protocol.h"

int main(void)
{
    ...外设初始化（SysTick_Init 必须开：强制模式超时依赖 tick）...
    printf("BootLoader\r\n");

    if(KEY4 按住) {                   /* 强制升级模式：救砖入口 */
        printf("强制升级模式\r\n");
        IAP_Init(IAP_APP1_ADDR);      /* 目标直接写 APP1 区 */
        while(1) {
            IAP_Task();
            if(IAP_IsUpgraded()) NVIC_SystemReset();
        }
    }

    if(IAP_Boot_CheckUpgrade()) {     /* 有待更新的完整固件 */
        IAP_Boot_MoveFw();            /* 搬运 APP2 → APP1 并清标志 */
    }
    IAP_Boot_LoadApp(IAP_APP1_ADDR);  /* 校验 + 跳转（校验不过不跳，落到下面兜底） */

    while(1) {                        /* 兜底：APP1 不可用，LED 闪烁提示 */
        LedToggle(...); Delay_ms(200);
    }
}
```

**上位机升级时序**（串口调试工具/自写上位机）：

```text
发 START(带16B固件头) ──▶ 等 ACK(0x06,0x00)    失败重发
发 DATA(≤1024B, 偏移0起) ─▶ 等 ACK              失败重发当前帧
发 DATA(下一段) ──────────▶ 等 ACK              循环直至发完
发 END(无数据) ───────────▶ 等 ACK              成功后 MCU 自动软件复位（APP 侧）
```

| 应答状态码 | 含义 |
| --- | --- |
| 0x00 | 成功（ACK） |
| 0x01~0x08 | 对应 iap_err_t 枚举：帧头/CRC/长度/固件CRC/超长/Flash/超时/忙 |

**打包工具**：上位机发送前把 bin 头部 16 字节替换为固件头（`magic=0x31504149, version, length=bin长度, crc32=bin全量CRC32`），可直接用 Python 脚本一行生成：

```python
# gen_fw.py：给 bin 加 16B 固件头
import struct, zlib, sys
binf = open(sys.argv[1], 'rb').read()
hdr  = struct.pack('<IIII', 0x31504149, 1, len(binf), zlib.crc32(binf) & 0xFFFFFFFF)
open(sys.argv[1] + '.fw', 'wb').write(hdr + binf)
print('OK, size =', len(hdr) + len(binf))
```

**集成步骤清单**：

1. 新建 `user/iap/` 目录，放入 6 个模块文件；
2. 在 `iap_config.h` 确认分区宏（Boot/APP1/APP2 与工程 IROM1 一致）；
3. 实现 `IAP_Uart_SendByte`（串口驱动里）与 `IAP_GetTick`（SysTick 1ms tick）；
4. 串口中断里调用 `IAP_Protocol_Feed`；
5. APP：`IAP_Init(IAP_APP2_ADDR)` + 主循环 `IAP_Task()`；Boot：按上面示例接线；
6. 用 `gen_fw.py` 打包固件，上位机按协议发送。

