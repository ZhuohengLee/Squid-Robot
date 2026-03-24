/**********************************************************************
 * CH9434A.h
 *
 * 这个文件声明 CH9434A SPI 转 4 路 UART 芯片的驱动接口。
 *********************************************************************/

// 防止头文件被重复包含。
#ifndef CH9434A_H
// 定义头文件保护宏。
#define CH9434A_H

// 引入 Arduino 基础类型。
#include <Arduino.h>
// 引入 Arduino SPI 接口定义。
#include <SPI.h>

// 定义接收保持寄存器地址。
#define CH9434A_REG_RHR         0x00
// 定义发送保持寄存器地址。
#define CH9434A_REG_THR         0x00
// 定义中断使能寄存器地址。
#define CH9434A_REG_IER         0x01
// 定义中断识别寄存器地址。
#define CH9434A_REG_IIR         0x02
// 定义 FIFO 控制寄存器地址。
#define CH9434A_REG_FCR         0x02
// 定义线路控制寄存器地址。
#define CH9434A_REG_LCR         0x03
// 定义调制解调器控制寄存器地址。
#define CH9434A_REG_MCR         0x04
// 定义线路状态寄存器地址。
#define CH9434A_REG_LSR         0x05
// 定义调制解调器状态寄存器地址。
#define CH9434A_REG_MSR         0x06
// 定义分频低字节寄存器地址。
#define CH9434A_REG_DLL         0x00
// 定义分频高字节寄存器地址。
#define CH9434A_REG_DLM         0x01

// 定义 DLAB 位掩码。
#define CH9434A_LCR_DLAB        0x80
// 定义 SBC 位掩码。
#define CH9434A_LCR_SBC         0x40
// 定义偶校验选择位掩码。
#define CH9434A_LCR_EPAR        0x10
// 定义校验使能位掩码。
#define CH9434A_LCR_PARITY      0x08
// 定义停止位配置位掩码。
#define CH9434A_LCR_STOPB       0x04
// 定义 8N1 配置值。
#define CH9434A_LCR_8N1         0x03
// 定义 7N1 配置值。
#define CH9434A_LCR_7N1         0x02
// 定义 6N1 配置值。
#define CH9434A_LCR_6N1         0x01
// 定义 5N1 配置值。
#define CH9434A_LCR_5N1         0x00

// 定义发送完全空闲位掩码。
#define CH9434A_LSR_TEMT        0x40
// 定义发送保持寄存器空位掩码。
#define CH9434A_LSR_THRE        0x20
// 定义中断中止位掩码。
#define CH9434A_LSR_BI          0x10
// 定义帧错误位掩码。
#define CH9434A_LSR_FE          0x08
// 定义校验错误位掩码。
#define CH9434A_LSR_PE          0x04
// 定义溢出错误位掩码。
#define CH9434A_LSR_OE          0x02
// 定义接收数据就绪位掩码。
#define CH9434A_LSR_DR          0x01

// 定义 FIFO 使能位掩码。
#define CH9434A_FCR_ENABLE      0x01
// 定义接收 FIFO 复位位掩码。
#define CH9434A_FCR_RX_RESET    0x02
// 定义发送 FIFO 复位位掩码。
#define CH9434A_FCR_TX_RESET    0x04

// 定义接收中断使能位掩码。
#define CH9434A_IER_RX          0x01
// 定义发送中断使能位掩码。
#define CH9434A_IER_TX          0x02
// 定义线路状态中断使能位掩码。
#define CH9434A_IER_LSR         0x04
// 定义调制解调器状态中断使能位掩码。
#define CH9434A_IER_MSR         0x08

// 定义芯片支持的 UART 通道数量。
#define CH9434A_NUM_UARTS       4
// 定义相邻 UART 通道的寄存器地址步长。
#define CH9434A_UART_BASE_STEP  0x10

// 声明 CH9434A 驱动类。
class CH9434A {
public:
    // 声明构造函数，允许传入片选脚和可选中断脚。
    CH9434A(uint8_t csPin, int8_t intPin = -1);

    // 声明初始化芯片和 SPI 参数的函数。
    bool begin(uint32_t spiFreq = 10000000);

    // 声明配置指定 UART 波特率和格式的函数。
    bool config(uint8_t uartNum, uint32_t baudrate, uint8_t config = CH9434A_LCR_8N1);

    // 声明向指定 UART 写入单个字节的函数。
    void write(uint8_t uartNum, uint8_t data);
    // 声明向指定 UART 写入整个缓冲区的函数。
    size_t write(uint8_t uartNum, const uint8_t* buffer, size_t length);
    // 声明向指定 UART 打印字符串的函数。
    size_t print(uint8_t uartNum, const char* str);
    // 声明向指定 UART 打印一行字符串的函数。
    size_t println(uint8_t uartNum, const char* str);

    // 声明从指定 UART 读取单个字节的函数。
    uint8_t read(uint8_t uartNum);
    // 声明从指定 UART 读取多个字节的函数。
    size_t read(uint8_t uartNum, uint8_t* buffer, size_t maxLength);
    // 声明查询指定 UART 是否有数据可读的函数。
    int available(uint8_t uartNum);
    // 声明清空指定 UART 接收 FIFO 的函数。
    void flush(uint8_t uartNum);

    // 声明读取线路状态寄存器的函数。
    uint8_t getLineStatus(uint8_t uartNum);
    // 声明读取中断状态寄存器的函数。
    uint8_t getInterruptStatus(uint8_t uartNum);

    // 声明开启 UART 中断的函数。
    void enableInterrupt(uint8_t uartNum, uint8_t flags);
    // 声明关闭 UART 中断的函数。
    void disableInterrupt(uint8_t uartNum);

    // 声明查询发送是否完成的函数。
    bool isTxEmpty(uint8_t uartNum);
    // 声明等待发送完成的函数。
    void waitTxComplete(uint8_t uartNum, uint32_t timeout = 100);

private:
    // 保存 SPI 片选脚编号。
    uint8_t _csPin;
    // 保存可选中断脚编号。
    int8_t _intPin;
    // 保存 SPI 时序配置对象。
    SPISettings _spiSettings;

    // 声明获取 UART 基地址的辅助函数。
    uint16_t getBaseAddr(uint8_t uartNum);
    // 声明写寄存器的底层辅助函数。
    void writeReg(uint8_t uartNum, uint8_t reg, uint8_t value);
    // 声明读寄存器的底层辅助函数。
    uint8_t readReg(uint8_t uartNum, uint8_t reg);
    // 声明设置波特率分频的辅助函数。
    void setBaudrate(uint8_t uartNum, uint32_t baudrate);
};

// 结束头文件保护。
#endif // CH9434A_H
