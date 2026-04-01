/**********************************************************************
 * OtaManager.cpp
 *
 * OTA 管理器实现，含网页配网（Captive Portal）。
 *********************************************************************/

#include "OtaManager.h"

#include <ArduinoOTA.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "OtaConfig.h"

// ────────────────────────────────────────────────────────────────────
// 配网页面 HTML
// ────────────────────────────────────────────────────────────────────
static const char PORTAL_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Squid Robot WiFi Setup</title>
<style>
  body{font-family:sans-serif;background:#1a1a2e;color:#eee;
       display:flex;justify-content:center;padding:32px 16px;margin:0}
  .card{background:#16213e;border-radius:12px;padding:28px;
        width:100%;max-width:380px}
  h2{margin:0 0 8px;color:#00d4ff;text-align:center;font-size:20px}
  .sub{text-align:center;color:#888;font-size:13px;margin-bottom:24px}
  label{display:block;margin-bottom:6px;font-size:13px;color:#aaa}
  input{width:100%;box-sizing:border-box;padding:10px;border-radius:6px;
        border:1px solid #0f3460;background:#0f3460;color:#fff;
        font-size:15px;margin-bottom:16px}
  button{width:100%;padding:12px;border-radius:6px;border:none;
         background:#00d4ff;color:#1a1a2e;font-size:16px;
         font-weight:bold;cursor:pointer}
  .net{padding:10px 14px;background:#0f3460;border-radius:6px;
       margin-bottom:8px;cursor:pointer;font-size:14px;display:flex;
       justify-content:space-between;align-items:center}
  .net:hover{background:#1a4a8a}
  .rssi{color:#888;font-size:12px}
  .sec{text-align:right;font-size:11px;color:#00d4ff;margin-left:8px}
  hr{border:none;border-top:1px solid #0f3460;margin:20px 0}
</style></head>
<body><div class="card">
  <h2>&#x1F991; Squid Robot</h2>
  <p class="sub">WiFi 配网 — 连接后可使用 OTA 无线烧录</p>
  <form method="POST" action="/save">
    <label>WiFi 名称 (SSID)</label>
    <input type="text" name="ssid" id="ssid" placeholder="输入或点击下方网络" required>
    <label>密码</label>
    <input type="password" name="pass" placeholder="无密码留空">
    <button type="submit">连接并保存</button>
  </form>
  <hr>
  <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:10px">
    <p style="color:#aaa;font-size:13px;margin:0">附近网络：</p>
    <button type="button" id="scanBtn" onclick="doScan()"
            style="width:auto;padding:6px 14px;font-size:13px;background:#0f3460;
                   color:#00d4ff;border:1px solid #00d4ff;border-radius:6px;cursor:pointer">
      重新扫描
    </button>
  </div>
  <div id="netList">{{SCAN}}</div>
</div>
<script>
function fill(s){document.getElementById('ssid').value=s}
function doScan(){
  var btn=document.getElementById('scanBtn');
  var list=document.getElementById('netList');
  btn.disabled=true; btn.textContent='扫描中...';
  list.innerHTML='<p style="color:#888;font-size:13px">扫描中，请稍候...</p>';
  fetch('/scan').then(function(r){return r.text()}).then(function(html){
    list.innerHTML=html;
    btn.disabled=false; btn.textContent='重新扫描';
  }).catch(function(){
    btn.disabled=false; btn.textContent='重新扫描';
  });
}
</script>
</body></html>
)HTML";

static const char PORTAL_SUCCESS_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>正在连接...</title>
<style>
  body{font-family:sans-serif;background:#1a1a2e;color:#eee;
       display:flex;justify-content:center;padding:60px 16px}
  .card{background:#16213e;border-radius:12px;padding:32px;
        text-align:center;max-width:360px}
  h2{color:#00d4ff}
  .hint{color:#888;font-size:13px;margin-top:16px}
</style></head>
<body><div class="card">
  <h2>正在连接...</h2>
  <p>SSID: <b>{{SSID}}</b></p>
  <p class="hint">连接成功后请关闭此页面。<br>若连接失败，设备将重新启动配网。</p>
</div></body></html>
)HTML";

// ────────────────────────────────────────────────────────────────────
namespace {
bool hasValue(const char* v) { return v && v[0] != '\0'; }

String escapeHtml(String value) {
    value.replace("&", "&amp;");
    value.replace("\"", "&quot;");
    value.replace("'", "&#39;");
    value.replace("<", "&lt;");
    value.replace(">", "&gt;");
    value.replace("\r", " ");
    value.replace("\n", " ");
    return value;
}
// 上电后 WiFi 总连接时间窗口，超时自动进入配网热点
constexpr uint32_t WIFI_TOTAL_TIMEOUT_MS   = 30000;
constexpr byte     DNS_PORT                = 53;
const char*        PROV_AP_SSID            = "SquidRobot-Setup";
}

// ────────────────────────────────────────────────────────────────────
OtaManager::OtaManager()
    : _ready(false),
      _usingSoftAP(false),
      _provisioningDone(false),
      _webServer(80) {}

// ────────────────────────────────────────────────────────────────────
void OtaManager::begin(bool underwaterMode) {
    _ready            = false;
    _usingSoftAP      = false;
    _provisioningDone = false;

    if (underwaterMode) {
        // 水下模式：关闭 WiFi 射频，不初始化 OTA
        WiFi.mode(WIFI_OFF);
        Serial.println(F("OTA: underwater mode detected, WiFi disabled."));
        return;
    }

    WiFi.persistent(true);       // 允许 NVS 保存凭据
    WiFi.setAutoReconnect(true);

    // 30s 总时间窗口：NVS + 硬编码 SSID 共享此预算，超时进入配网热点
    const uint32_t deadline = millis() + WIFI_TOTAL_TIMEOUT_MS;

    if (beginNvsMode(deadline)) {
        // NVS 保存的凭据连接成功
    } else if (beginStationMode(deadline)) {
        // OtaConfig 硬编码 SSID 连接成功
    } else {
        Serial.println(F("WiFi: 30s timeout reached, starting provisioning portal."));
        if (!beginProvisioningPortal()) {
            beginSoftAPMode();   // 配网也失败，兜底 SoftAP OTA
        }
    }

    configureArduinoOta();
    ArduinoOTA.begin();
    _ready = true;

    Serial.print(F("OTA ready  : "));
    Serial.print(OTA_HOSTNAME);
    Serial.print(F("  @  "));
    Serial.println(getLocalIP());
}

// ────────────────────────────────────────────────────────────────────
void OtaManager::handle() {
    if (_ready) {
        ArduinoOTA.handle();
    }
}

bool OtaManager::isReady()        const { return _ready; }
bool OtaManager::isUsingSoftAP()  const { return _usingSoftAP; }
IPAddress OtaManager::getLocalIP() const {
    return _usingSoftAP ? WiFi.softAPIP() : WiFi.localIP();
}

// ────────────────────────────────────────────────────────────────────
// 1. 用 NVS 保存的凭据连接
// ────────────────────────────────────────────────────────────────────
bool OtaManager::beginNvsMode(uint32_t deadline) {
    WiFi.mode(WIFI_STA);
    WiFi.begin();   // 无参数 = 使用 NVS 保存的 SSID/PASS

    Serial.print(F("WiFi: trying NVS saved credentials"));
    // NVS 尝试最多占用一半剩余时间，最长 10s
    const uint32_t now     = millis();
    const uint32_t remaining = deadline > now ? deadline - now : 0;
    const uint32_t timeout   = min(remaining / 2, (uint32_t)10000);
    const uint32_t start     = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
        Serial.print('.');
        delay(250);
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print(F("WiFi connected (NVS): "));
        Serial.println(WiFi.SSID());
        return true;
    }

    WiFi.disconnect(true);
    return false;
}

// ────────────────────────────────────────────────────────────────────
// 2. 用 OtaConfig 硬编码 SSID 连接
// ────────────────────────────────────────────────────────────────────
bool OtaManager::beginStationMode(uint32_t deadline) {
    if (!hasValue(OTA_STA_SSID)) return false;

    Serial.print(F("WiFi: connecting to "));
    Serial.println(OTA_STA_SSID);

    // 使用剩余时间，最长受 OTA_STA_CONNECT_TIMEOUT_MS 限制
    const uint32_t now       = millis();
    const uint32_t remaining = deadline > now ? deadline - now : 0;
    const uint32_t timeout   = min(remaining, (uint32_t)OTA_STA_CONNECT_TIMEOUT_MS);

    return connectSta(OTA_STA_SSID, OTA_STA_PASSWORD, timeout);
}

// ────────────────────────────────────────────────────────────────────
// 3. 配网热点 + Captive Portal（阻塞直到成功）
// ────────────────────────────────────────────────────────────────────
bool OtaManager::beginProvisioningPortal() {
    Serial.println(F("WiFi: starting provisioning portal..."));

    // 启动 AP
    WiFi.mode(WIFI_AP);
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    if (!WiFi.softAP(PROV_AP_SSID)) {
        Serial.println(F("Provisioning AP failed."));
        return false;
    }
    Serial.print(F("Provisioning AP: "));
    Serial.print(PROV_AP_SSID);
    Serial.print(F("  IP: "));
    Serial.println(WiFi.softAPIP());

    // DNS 将所有域名重定向到本机（强制门户）
    _dnsServer.start(DNS_PORT, "*", apIP);

    // 扫描附近 WiFi 供用户选择
    Serial.println(F("Scanning WiFi..."));
    WiFi.scanNetworks(true);  // 异步扫描，不阻塞

    // 注册 HTTP 路由
    _webServer.on("/",     HTTP_GET,  [this]() { handlePortalRoot(); });
    _webServer.on("/save", HTTP_POST, [this]() { handlePortalSave(); });
    _webServer.on("/scan", HTTP_GET,  [this]() { handlePortalScan(); });
    _webServer.onNotFound(           [this]() { handlePortalNotFound(); });
    _webServer.begin();
    Serial.println(F("Portal web server started."));
    Serial.println(F("Connect to 'SquidRobot-Setup' and open any webpage."));

    // 阻塞直到配网成功
    while (!_provisioningDone) {
        _dnsServer.processNextRequest();
        _webServer.handleClient();
        delay(5);
    }

    _dnsServer.stop();
    _webServer.stop();
    WiFi.softAPdisconnect(true);
    return true;
}

// ────────────────────────────────────────────────────────────────────
// 4. SoftAP 兜底（仅 OTA，无配网）
// ────────────────────────────────────────────────────────────────────
bool OtaManager::beginSoftAPMode() {
    if (!hasValue(OTA_AP_SSID) || !hasValue(OTA_AP_PASSWORD)) return false;

    Serial.print(F("OTA SoftAP fallback: "));
    Serial.println(OTA_AP_SSID);

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(OTA_AP_SSID, OTA_AP_PASSWORD)) {
        Serial.println(F("SoftAP fallback failed."));
        return false;
    }
    _usingSoftAP = true;
    return true;
}

// ────────────────────────────────────────────────────────────────────
// 配网页面 - 首页
// ────────────────────────────────────────────────────────────────────
void OtaManager::handlePortalRoot() {
    String html = FPSTR(PORTAL_HTML);
    html.replace("{{SCAN}}", buildScanResults());
    _webServer.send(200, "text/html", html);
}

// ────────────────────────────────────────────────────────────────────
// 配网页面 - 提交处理
// ────────────────────────────────────────────────────────────────────
void OtaManager::handlePortalSave() {
    if (!_webServer.hasArg("ssid") || _webServer.arg("ssid").isEmpty()) {
        _webServer.send(400, "text/html",
            "<meta charset='UTF-8'><p style='color:red'>错误：SSID 不能为空</p>");
        return;
    }

    String ssid = _webServer.arg("ssid");
    String pass = _webServer.hasArg("pass") ? _webServer.arg("pass") : "";

    // 先回复页面，再尝试连接（避免浏览器超时）
    String html = FPSTR(PORTAL_SUCCESS_HTML);
    html.replace("{{SSID}}", escapeHtml(ssid));
    _webServer.send(200, "text/html", html);

    Serial.print(F("[Portal] Trying: "));
    Serial.println(ssid);

    // 切换到 STA 模式连接
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);

    if (connectSta(ssid.c_str(), pass.c_str(), OTA_STA_CONNECT_TIMEOUT_MS)) {
        Serial.print(F("[Portal] Connected: "));
        Serial.println(WiFi.localIP());
        _provisioningDone = true;
    } else {
        Serial.println(F("[Portal] Connect failed, restarting provisioning..."));
        WiFi.disconnect(true);
        ESP.restart();  // 重启重新进入配网
    }
}

// ────────────────────────────────────────────────────────────────────
// 配网页面 - 404 → 重定向首页（强制门户关键）
// ────────────────────────────────────────────────────────────────────
void OtaManager::handlePortalNotFound() {
    _webServer.sendHeader("Location", "http://192.168.4.1/", true);
    _webServer.send(302, "text/plain", "");
}

// ────────────────────────────────────────────────────────────────────
// 配网页面 - 手动重新扫描（同步扫描，约 2s，返回网络列表 HTML 片段）
// ────────────────────────────────────────────────────────────────────
void OtaManager::handlePortalScan() {
    Serial.println(F("[Portal] Manual rescan requested."));
    WiFi.scanNetworks();   // 同步扫描，阻塞约 2s
    _webServer.send(200, "text/html; charset=utf-8", buildScanResults());
}

// ────────────────────────────────────────────────────────────────────
// 生成 WiFi 扫描结果 HTML
// ────────────────────────────────────────────────────────────────────
String OtaManager::buildScanResults() {
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_RUNNING) return "<p style='color:#888;font-size:13px'>扫描中...</p>";
    if (n <= 0)                 return "<p style='color:#888;font-size:13px'>未发现网络</p>";

    String html = "";
    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        int    rssi = WiFi.RSSI(i);
        bool   enc  = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        const String safeSsid = escapeHtml(ssid);
        html += "<div class='net' data-ssid=\"" + safeSsid + "\" onclick='fill(this.dataset.ssid)'>";
        html += "<span>" + safeSsid + "</span>";
        html += "<span class='rssi'>" + String(rssi) + "dBm";
        if (enc) html += "<span class='sec'> &#x1F512;</span>";
        html += "</span></div>";
    }
    WiFi.scanDelete();
    return html;
}

// ────────────────────────────────────────────────────────────────────
// 通用 STA 连接（带超时、进度点）
// ────────────────────────────────────────────────────────────────────
bool OtaManager::connectSta(const char* ssid, const char* pass, uint32_t timeoutMs) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, (pass && pass[0]) ? pass : nullptr);

    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
        Serial.print('.');
        delay(250);
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print(F("WiFi connected: "));
        Serial.print(WiFi.SSID());
        Serial.print(F("  IP: "));
        Serial.println(WiFi.localIP());
        return true;
    }

    WiFi.disconnect(true);
    Serial.println(F("WiFi connect timeout."));
    return false;
}

// ────────────────────────────────────────────────────────────────────
// ArduinoOTA 配置
// ────────────────────────────────────────────────────────────────────
void OtaManager::configureArduinoOta() {
    ArduinoOTA.setHostname(OTA_HOSTNAME);

    if (hasValue(OTA_PASSWORD)) {
        ArduinoOTA.setPassword(OTA_PASSWORD);
    }

    ArduinoOTA.onStart([]() {
        Serial.println(F("\nOTA start"));
    });
    ArduinoOTA.onEnd([]() {
        Serial.println(F("\nOTA end"));
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static uint8_t last = 255;
        const uint8_t  pct  = total ? (uint8_t)((progress * 100U) / total) : 0;
        if (pct != last && pct % 10 == 0) {
            last = pct;
            Serial.printf("OTA: %u%%\n", pct);
        }
    });
    ArduinoOTA.onError([](ota_error_t e) {
        Serial.printf("OTA error[%u]\n", (unsigned)e);
    });
}
