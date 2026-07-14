/**********************************************************************
 * OtaManager.h
 *
 * OTA 管理器，集成网页配网（Captive Portal）+ 浏览器串口控制台
 *                                           + HTTP SD 文件管理接口。
 * 仅使用 ESP32 内置 WebServer，无需额外库。
 *
 * 启动流程：
 *   1. 尝试连接 NVS 已保存的 WiFi（最多 10s）
 *   2. 若失败，启动配网热点 + 强制门户网页，阻塞直到配网成功
 *   3. OTA 服务启动
 *   4. 调用 beginWebConsole() 启动浏览器串口 + SD 文件接口
 *
 * HTTP SD 文件接口（beginWebConsole 调用后可用）：
 *   GET  /files?path=/          列出目录，返回 JSON
 *   GET  /file?path=/xxx/yyy    下载文件
 *   POST /file?path=/xxx/yyy    上传文件（body 为二进制内容）
 *   DELETE /file?path=/xxx/yyy  删除文件或空目录
 *********************************************************************/

#ifndef ESP32_OTA_MANAGER_H
#define ESP32_OTA_MANAGER_H

#include <Arduino.h>
#include <DNSServer.h>
#include <IPAddress.h>
#include <SD.h>
#include <WebServer.h>
#include <functional>
#include <time.h>

// ── 浏览器控制台 Print 桥接 ───────────────────────────────────────────
// 将 Print 输出写入滑动窗口缓冲；额外维护 _total（总写入字节数），
// 便于浏览器在缓冲被截断时识别"丢字节"并重新对齐。
class WebConsolePrint : public Print {
public:
    static constexpr size_t BUF_MAX = 8192;

    void setBuffer(String* buf) { _buf = buf; _total = 0; }
    size_t getTotal() const { return _total; }
    size_t getBufStart() const {
        return (_buf && _total >= _buf->length()) ? (_total - _buf->length()) : 0;
    }

    size_t write(uint8_t c) override {
        if (!_buf) return 1;
        if (_buf->length() >= BUF_MAX) *_buf = _buf->substring(BUF_MAX / 2);
        *_buf += (char)c;
        _total++;
        return 1;
    }

    size_t write(const uint8_t* data, size_t size) override {
        for (size_t i = 0; i < size; i++) write(data[i]);
        return size;
    }

private:
    String* _buf   = nullptr;
    size_t  _total = 0;
};

// ── OtaManager ───────────────────────────────────────────────────────
class OtaManager {
public:
    OtaManager();

    void begin(bool underwaterMode = false);
    void handle();

    bool isReady() const;
    IPAddress getLocalIP() const;

    // NTP 时间（WiFi 连接后自动同步）
    bool syncNtp();
    bool hasTime() const;
    void getSessionFolderName(char* buf, size_t len) const;

    // 浏览器串口 + HTTP SD 文件接口：WiFi 就绪后调用
    // 控制台地址：http://<IP>/console
    // 文件列表：  http://<IP>/files?path=/
    void beginWebConsole(std::function<void(uint8_t*, size_t)> msgCallback);
    Print* getConsolePrint();

private:
    bool     _ready;
    bool     _provisioningDone;
    bool     _consoleActive;
    bool     _routesRegistered;   // 路由只注册一次，防止重复调用 beginWebConsole 出错
    bool     _ntpSynced;
    time_t   _ntpEpoch;
    uint32_t _ntpSyncMs;

    String          _pendingSsid;
    String          _pendingPass;
    String          _logBuf;
    WebConsolePrint _consolePrint;
    std::function<void(uint8_t*, size_t)> _cmdCb;

    DNSServer  _dnsServer;
    WebServer  _webServer;

    bool beginNvsMode();
    bool beginProvisioningPortal();

    void configureArduinoOta();

    void handlePortalRoot();
    void handlePortalSave();
    void handlePortalScan();
    void handlePortalNotFound();
    String buildScanResults();

    void handleConsoleHtml();
    void handleConsoleLog();
    void handleConsoleCmd();

    // HTTP SD 文件接口
    void handleFileList();           // GET    /files?path=
    void handleFileDownload();       // GET    /file?path=
    void handleFileUploadStream();   // POST   /upload?path=  （onUpload 流式回调）
    void handleFileDelete();         // DELETE /file?path=

    // 递归删除文件或目录（处理含子项的 session 文件夹）
    static bool removeRecursive(const String& path);

    bool connectSta(const char* ssid, const char* pass, uint32_t timeoutMs);

    File _uploadFile;   // 流式上传时打开的目标文件
};

#endif // ESP32_OTA_MANAGER_H
