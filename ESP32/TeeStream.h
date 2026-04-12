/**********************************************************************
 * TeeStream.h
 *
 * 将 Print 输出同时写入两个流。
 * 用于把 USB Serial 的调试输出同步转发到 HC-12。
 *********************************************************************/

#ifndef TEE_STREAM_H
#define TEE_STREAM_H

#include <Print.h>

class TeeStream : public Print {
public:
  TeeStream(Print& a, Print& b)
    : _a(a), _b(b), _c(nullptr) {}

  // 动态挂载第三路输出（如 WebConsole），WiFi 连上后调用
  void setThird(Print* c) { _c = c; }

  size_t write(uint8_t c) override {
    _a.write(c);
    _b.write(c);
    if (_c) _c->write(c);
    return 1;
  }

  size_t write(const uint8_t* buf, size_t size) override {
    _a.write(buf, size);
    _b.write(buf, size);
    if (_c) _c->write(buf, size);
    return size;
  }

private:
  Print& _a;
  Print& _b;
  Print* _c;
};

// 全局调试输出：HC-12 初始化前指向 USB Serial，之后指向 TeeStream。
extern Print* g_dbg;

#endif  // TEE_STREAM_H
