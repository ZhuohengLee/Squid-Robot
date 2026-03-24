/**********************************************************************
 * CH9434A.cpp
 *
 * 这个文件实现 CH9434A 芯片的底层 SPI 读写驱动。
 *********************************************************************/

// 引入 CH9434A 类声明。
#include "CH9434A.h"

// 实现构造函数。
CH9434A::CH9434A(uint8_t csPin, int8_t intPin)
    // 保存片选引脚编号。
    : _csPin(csPin),
      // 保存中断引脚编号。
      _intPin(intPin) {
}

// 实现初始化函数。
bool CH9434A::begin(uint32_t spiFreq) {
    // 把片选脚配置为输出。
    pinMode(_csPin, OUTPUT);
    // 默认先拉高片选脚。
    digitalWrite(_csPin, HIGH);

    // 如果提供了中断脚。
    if (_intPin >= 0) {
        // 就把中断脚配置为上拉输入。
        pinMode(_intPin, INPUT_PULLUP);
    }

    // 初始化 SPI 外设。
    SPI.begin();
    // 保存 SPI 时序配置。
    _spiSettings = SPISettings(spiFreq, MSBFIRST, SPI_MODE0);

    // 上电后先等待芯片稳定。
    delay(100);

    // 逐路初始化 4 个 UART 通道。
    for (uint8_t i = 0; i < CH9434A_NUM_UARTS; i++) {
        // 关闭该通道所有中断。
        writeReg(i, CH9434A_REG_IER, 0x00);
        // 使能 FIFO 并同时清空收发 FIFO。
        writeReg(i, CH9434A_REG_FCR,
                 CH9434A_FCR_ENABLE | CH9434A_FCR_RX_RESET | CH9434A_FCR_TX_RESET);
        // 把串口格式设为 8N1。
        writeReg(i, CH9434A_REG_LCR, CH9434A_LCR_8N1);
    }

    // 读取 UART0 的线路状态寄存器做一次基础连通性检查。
    const uint8_t lsr = readReg(0, CH9434A_REG_LSR);
    // 过滤明显无效的返回值。
    return (lsr != 0xFF && lsr != 0x00 && lsr != 0x10);
}

// 实现 UART 配置函数。
bool CH9434A::config(uint8_t uartNum, uint32_t baudrate, uint8_t config) {
    // 如果 UART 编号越界，就返回失败。
    if (uartNum >= CH9434A_NUM_UARTS) {
        return false;
    }

    // 先设置 UART 波特率。
    setBaudrate(uartNum, baudrate);
    // 再写入线路格式配置。
    writeReg(uartNum, CH9434A_REG_LCR, config);
    // 使能 FIFO 并清空收发缓存。
    writeReg(uartNum, CH9434A_REG_FCR,
             CH9434A_FCR_ENABLE | CH9434A_FCR_RX_RESET | CH9434A_FCR_TX_RESET);

    // 返回配置成功。
    return true;
}

// 实现写单字节函数。
void CH9434A::write(uint8_t uartNum, uint8_t data) {
    // 如果 UART 编号越界，就直接返回。
    if (uartNum >= CH9434A_NUM_UARTS) {
        return;
    }

    // 计算写超时时刻。
    const uint32_t timeout = millis() + 100;
    // 持续等待发送保持寄存器变为空。
    while (!(readReg(uartNum, CH9434A_REG_LSR) & CH9434A_LSR_THRE)) {
        // 如果已经超时，就放弃本次发送。
        if (millis() > timeout) {
            return;
        }
        // 让出 CPU，避免长时间忙等。
        yield();
    }

    // 把目标字节写入发送保持寄存器。
    writeReg(uartNum, CH9434A_REG_THR, data);
}

// 实现写缓冲区函数。
size_t CH9434A::write(uint8_t uartNum, const uint8_t* buffer, size_t length) {
    // 逐字节写出整个缓冲区。
    for (size_t i = 0; i < length; i++) {
        // 调用单字节写函数。
        write(uartNum, buffer[i]);
    }
    // 返回请求写入的总长度。
    return length;
}

// 实现打印字符串函数。
size_t CH9434A::print(uint8_t uartNum, const char* str) {
    // 把字符串强转成字节数组后调用缓冲区写函数。
    return write(uartNum, reinterpret_cast<const uint8_t*>(str), strlen(str));
}

// 实现打印字符串并换行函数。
size_t CH9434A::println(uint8_t uartNum, const char* str) {
    // 先打印字符串主体。
    const size_t n = print(uartNum, str);
    // 发送回车符。
    write(uartNum, '\r');
    // 发送换行符。
    write(uartNum, '\n');
    // 返回总写出字符数。
    return n + 2;
}

// 实现读单字节函数。
uint8_t CH9434A::read(uint8_t uartNum) {
    // 如果 UART 编号越界，就返回 0。
    if (uartNum >= CH9434A_NUM_UARTS) {
        return 0;
    }

    // 如果接收数据就绪位未置位，就说明当前没有数据。
    if (!(readReg(uartNum, CH9434A_REG_LSR) & CH9434A_LSR_DR)) {
        return 0;
    }

    // 从接收保持寄存器读取一个字节并返回。
    return readReg(uartNum, CH9434A_REG_RHR);
}

// 实现读多个字节函数。
size_t CH9434A::read(uint8_t uartNum, uint8_t* buffer, size_t maxLength) {
    // 初始化已读取字节数为 0。
    size_t count = 0;

    // 在缓冲区未满时持续读取。
    while (count < maxLength) {
        // 如果当前已经没有可读数据，就退出循环。
        if (!(readReg(uartNum, CH9434A_REG_LSR) & CH9434A_LSR_DR)) {
            break;
        }
        // 读取一个字节放入缓冲区，并递增计数器。
        buffer[count++] = readReg(uartNum, CH9434A_REG_RHR);
    }

    // 返回实际读取的字节数。
    return count;
}

// 实现可读数据查询函数。
int CH9434A::available(uint8_t uartNum) {
    // 如果 UART 编号越界，就返回 0。
    if (uartNum >= CH9434A_NUM_UARTS) {
        return 0;
    }
    // 根据数据就绪位返回 1 或 0。
    return (readReg(uartNum, CH9434A_REG_LSR) & CH9434A_LSR_DR) ? 1 : 0;
}

// 实现清空接收 FIFO 的函数。
void CH9434A::flush(uint8_t uartNum) {
    // 如果 UART 编号越界，就直接返回。
    if (uartNum >= CH9434A_NUM_UARTS) {
        return;
    }
    // 使能 FIFO 并复位接收 FIFO。
    writeReg(uartNum, CH9434A_REG_FCR, CH9434A_FCR_ENABLE | CH9434A_FCR_RX_RESET);
}

// 实现线路状态读取函数。
uint8_t CH9434A::getLineStatus(uint8_t uartNum) {
    // 如果 UART 编号越界，就返回 0。
    if (uartNum >= CH9434A_NUM_UARTS) {
        return 0;
    }
    // 读取并返回线路状态寄存器。
    return readReg(uartNum, CH9434A_REG_LSR);
}

// 实现中断状态读取函数。
uint8_t CH9434A::getInterruptStatus(uint8_t uartNum) {
    // 如果 UART 编号越界，就返回 0。
    if (uartNum >= CH9434A_NUM_UARTS) {
        return 0;
    }
    // 读取并返回中断状态寄存器。
    return readReg(uartNum, CH9434A_REG_IIR);
}

// 实现中断使能函数。
void CH9434A::enableInterrupt(uint8_t uartNum, uint8_t flags) {
    // 如果 UART 编号越界，就直接返回。
    if (uartNum >= CH9434A_NUM_UARTS) {
        return;
    }
    // 向中断使能寄存器写入目标标志位。
    writeReg(uartNum, CH9434A_REG_IER, flags);
}

// 实现中断关闭函数。
void CH9434A::disableInterrupt(uint8_t uartNum) {
    // 如果 UART 编号越界，就直接返回。
    if (uartNum >= CH9434A_NUM_UARTS) {
        return;
    }
    // 把中断使能寄存器清零。
    writeReg(uartNum, CH9434A_REG_IER, 0x00);
}

// 实现发送完成检查函数。
bool CH9434A::isTxEmpty(uint8_t uartNum) {
    // 检查线路状态寄存器中的发送完全空闲位。
    return (getLineStatus(uartNum) & CH9434A_LSR_TEMT) != 0;
}

// 实现等待发送完成函数。
void CH9434A::waitTxComplete(uint8_t uartNum, uint32_t timeout) {
    // 记录等待起始时间。
    const uint32_t start = millis();
    // 只要发送还没完成，就持续等待。
    while (!isTxEmpty(uartNum)) {
        // 如果等待时间已经超时，就退出。
        if (millis() - start > timeout) {
            break;
        }
        // 让出 CPU，避免长时间忙等。
        yield();
    }
}

// 实现 UART 基地址获取函数。
uint16_t CH9434A::getBaseAddr(uint8_t uartNum) {
    // 当前实现不直接使用基地址，因此仅消除未使用参数警告。
    (void)uartNum;
    // 返回 0 作为占位值。
    return 0;
}

// 实现寄存器写函数。
void CH9434A::writeReg(uint8_t uartNum, uint8_t reg, uint8_t value) {
    // 计算目标寄存器地址。
    const uint8_t addr = reg + (CH9434A_UART_BASE_STEP * uartNum);

    // 开始一次 SPI 事务。
    SPI.beginTransaction(_spiSettings);
    // 拉低片选脚，选中 CH9434A。
    digitalWrite(_csPin, LOW);

    // 发送写命令，最高位为 1。
    SPI.transfer(0x80 | addr);
    // 按当前经验增加短延时，改善时序稳定性。
    delayMicroseconds(1);
    // 发送寄存器值。
    SPI.transfer(value);

    // 再增加三个短延时，给芯片留出内部锁存时间。
    delayMicroseconds(1);
    delayMicroseconds(1);
    delayMicroseconds(1);

    // 拉高片选脚，结束对芯片的访问。
    digitalWrite(_csPin, HIGH);
    // 结束 SPI 事务。
    SPI.endTransaction();
}

// 实现寄存器读函数。
uint8_t CH9434A::readReg(uint8_t uartNum, uint8_t reg) {
    // 计算目标寄存器地址。
    const uint8_t addr = reg + (CH9434A_UART_BASE_STEP * uartNum);

    // 开始一次 SPI 事务。
    SPI.beginTransaction(_spiSettings);
    // 拉低片选脚，选中 CH9434A。
    digitalWrite(_csPin, LOW);

    // 发送读命令，最高位保持 0。
    SPI.transfer(addr);
    // 增加三个短延时，满足当前驱动经验时序。
    delayMicroseconds(1);
    delayMicroseconds(1);
    delayMicroseconds(1);

    // 发送 dummy 字节并同时取回返回值。
    const uint8_t value = SPI.transfer(0xFF);
    // 再增加一个短延时。
    delayMicroseconds(1);

    // 拉高片选脚，结束对芯片的访问。
    digitalWrite(_csPin, HIGH);
    // 结束 SPI 事务。
    SPI.endTransaction();

    // 返回读到的寄存器值。
    return value;
}

// 实现波特率配置函数。
void CH9434A::setBaudrate(uint8_t uartNum, uint32_t baudrate) {
    // 定义 CH9434A 的内部时钟频率。
    const uint32_t clockFreq = 32000000;
    // 根据官方公式计算分频值。
    const uint16_t divisor = clockFreq / (8 * baudrate);

    // 先读取当前线路控制寄存器。
    const uint8_t lcr = readReg(uartNum, CH9434A_REG_LCR);
    // 置位 DLAB，切换到分频寄存器访问模式。
    writeReg(uartNum, CH9434A_REG_LCR, lcr | CH9434A_LCR_DLAB);
    // 写入分频低字节。
    writeReg(uartNum, CH9434A_REG_DLL, divisor & 0xFF);
    // 写入分频高字节。
    writeReg(uartNum, CH9434A_REG_DLM, (divisor >> 8) & 0xFF);
    // 清除 DLAB，恢复正常寄存器访问模式。
    writeReg(uartNum, CH9434A_REG_LCR, lcr & ~CH9434A_LCR_DLAB);
}
