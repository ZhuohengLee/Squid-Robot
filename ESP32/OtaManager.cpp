/**********************************************************************
 * OtaManager.cpp
 *
 * OTA 管理器实现，含网页配网（Captive Portal）+ 浏览器控制台。
 * 仅使用 ESP32 内置 WebServer，无需 ESPAsyncWebServer。
 *********************************************************************/

#include "OtaManager.h"
#include "OtaConfig.h"

#include <ArduinoOTA.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <SD.h>
#include "TeeStream.h"

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

// ── 浏览器统一控制台页面（文件管理 + 串口控制台）─────────────────────
static const char CONSOLE_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="zh-CN"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Squid Robot 控制台</title>
<script src="https://cdnjs.cloudflare.com/ajax/libs/jszip/3.10.1/jszip.min.js"></script>
<style>
*{box-sizing:border-box;margin:0;padding:0;font-family:-apple-system,'Segoe UI',Roboto,sans-serif}
body{background:#1a1a1a;color:#ddd;height:100vh;display:flex;flex-direction:column;font-size:13px;overflow:hidden}
.title{padding:6px 12px;background:#0d2a4f;color:#fff;font-weight:600;font-size:13px;border-bottom:1px solid #1a4070}
.bar{padding:6px 8px;background:#252525;border-bottom:1px solid #333;display:flex;align-items:center;gap:6px;flex-wrap:wrap}
.bar button{background:#2d4f7f;color:#fff;border:0;padding:5px 10px;border-radius:3px;cursor:pointer;font-size:12px}
.bar button:hover:not(:disabled){background:#3a629a}
.bar button:disabled{background:#333;color:#666;cursor:default}
.path{flex:1;color:#8fcfff;font-family:Consolas,monospace;padding:0 6px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;min-width:80px}
#files{flex:1 1 40%;overflow:auto;background:#202020;min-height:120px}
table{width:100%;border-collapse:collapse}
th{position:sticky;top:0;background:#2a2a2a;text-align:left;padding:6px 10px;font-weight:600;color:#aaa;border-bottom:1px solid #333;font-size:11px}
th.r{text-align:right}
th.cb,td.cb{width:32px;padding:4px 4px 4px 12px;text-align:center}
th.cb input,td.cb input{cursor:pointer;width:14px;height:14px;accent-color:#3a629a}
td{padding:4px 10px;border-bottom:1px solid #2a2a2a;cursor:pointer;user-select:none}
tr:hover td{background:#2a2a2a}
tr.sel td{background:#2d4f7f55}
td.r{text-align:right;color:#888;font-family:Consolas,monospace}
.dir{color:#8fcfff}
.file{color:#ddd}
.fs{padding:4px 12px;color:#888;font-size:11px;background:#1f1f1f;border-bottom:1px solid #333;border-top:1px solid #333}
#out{flex:1 1 40%;background:#0d0d0d;color:#00ff41;font-family:Consolas,'Courier New',monospace;padding:8px 12px;overflow-y:auto;white-space:pre-wrap;font-size:12px;line-height:1.5;min-height:120px}
.cmd{display:flex;gap:6px;padding:6px;background:#252525;border-top:1px solid #333}
#inp{flex:1;background:#111;color:#00ff41;border:1px solid #333;padding:6px 10px;font-family:Consolas,monospace;font-size:12px;border-radius:3px;outline:none}
#inp:focus{border-color:#00ff41}
.cmd button{background:#00ff41;color:#000;border:0;padding:6px 18px;border-radius:3px;cursor:pointer;font-weight:bold;font-size:12px}
.cmd button:hover{background:#33ff66}
input[type=file]{display:none}
</style></head>
<body>
<div class="title">🦑 Squid Robot 控制台</div>

<div class="bar">
  <button id="up">↑ 上级</button>
  <span class="path" id="path">/</span>
  <button id="refresh">⟳ 刷新</button>
  <button id="upload">⬆ 上传</button>
  <button id="download" disabled>⬇ 下载</button>
  <button id="delete" disabled>✕ 删除</button>
  <input type="file" id="filepick" multiple>
</div>
<div id="files"><table>
  <thead><tr>
    <th class="cb"><input type="checkbox" id="selAll" title="全选/全不选"></th>
    <th>名称</th>
    <th class="r">大小</th>
  </tr></thead>
  <tbody id="tb"></tbody>
</table></div>
<div class="fs" id="fs">—</div>

<div id="out"></div>
<div class="cmd">
  <input id="inp" placeholder="输入命令，回车发送...">
  <button id="send">发送</button>
</div>

<script>
const $=id=>document.getElementById(id);
const tb=$('tb'),out=$('out'),inp=$('inp'),path=$('path'),fs=$('fs');
let cur='/',pos=0;
let curItems=[];           // 当前目录项快照（含 dir/size 等元数据）
const sel=new Set();       // 选中项的 name 集合（仅当前目录）

function fmt(n){
  if(n<1024) return n+' B';
  if(n<1048576) return (n/1024).toFixed(1)+' KB';
  if(n<1073741824) return (n/1048576).toFixed(2)+' MB';
  return (n/1073741824).toFixed(2)+' GB';
}
function join(base,name){return (base==='/'?'':base.replace(/\/$/,''))+'/'+name;}

function refreshButtons(){
  const has=sel.size>0;
  $('download').disabled=!has;
  $('delete').disabled=!has;
}
function refreshSelAll(){
  const cbs=tb.querySelectorAll('input.cb');
  const all=$('selAll');
  if(cbs.length===0){all.checked=false;all.indeterminate=false;return;}
  let n=0;cbs.forEach(c=>{if(c.checked)n++;});
  all.checked=(n===cbs.length);
  all.indeterminate=(n>0&&n<cbs.length);
}
function toggleRow(tr,checked){
  const name=tr.dataset.name;
  const cb=tr.querySelector('input.cb');
  if(cb) cb.checked=checked;
  if(checked){sel.add(name);tr.classList.add('sel');}
  else{sel.delete(name);tr.classList.remove('sel');}
  refreshButtons();refreshSelAll();
}

async function load(p){
  fs.textContent='读取 '+p+' ...';
  try{
    const r=await fetch('/files?path='+encodeURIComponent(p));
    if(!r.ok) throw new Error('HTTP '+r.status);
    const j=await r.json();
    cur=j.path||p;
    path.textContent=cur;
    sel.clear();
    $('selAll').checked=false;$('selAll').indeterminate=false;
    $('up').disabled=(cur==='/');
    const items=(j.items||[]).slice().sort((a,b)=>{
      if(a.dir!==b.dir) return a.dir?-1:1;
      return (a.name||'').localeCompare(b.name||'');
    }).filter(it=>!!it.name);
    curItems=items;
    tb.innerHTML='';
    items.forEach(it=>{
      const tr=document.createElement('tr');
      tr.dataset.name=it.name;
      tr.dataset.dir=it.dir?'1':'';
      const tdC=document.createElement('td');
      tdC.className='cb';
      const cb=document.createElement('input');
      cb.type='checkbox';cb.className='cb';
      cb.onclick=e=>{e.stopPropagation();toggleRow(tr,cb.checked);};
      tdC.appendChild(cb);
      const tdN=document.createElement('td');
      tdN.className=it.dir?'dir':'file';
      tdN.textContent=(it.dir?'📁 ':'📄 ')+it.name;
      const tdS=document.createElement('td');
      tdS.className='r';
      tdS.textContent=it.dir?'':fmt(it.size||0);
      tr.appendChild(tdC);tr.appendChild(tdN);tr.appendChild(tdS);
      // 单击行（非 checkbox）：进入目录；双击：单击行为 + 文件直接下载
      tr.onclick=e=>{if(e.target===cb||e.target.closest('td.cb'))return;if(it.dir)load(join(cur,it.name));};
      tr.ondblclick=e=>{if(e.target===cb||e.target.closest('td.cb'))return;if(it.dir)load(join(cur,it.name));else doDownload(it.name);};
      tb.appendChild(tr);
    });
    refreshButtons();refreshSelAll();
    fs.textContent='就绪 — '+items.length+' 项';
  }catch(e){fs.textContent='读取失败: '+e.message;}
}

$('selAll').onchange=e=>{
  const check=e.target.checked;
  tb.querySelectorAll('tr').forEach(tr=>toggleRow(tr,check));
};

function doDownload(name){
  const a=document.createElement('a');
  a.href='/file?path='+encodeURIComponent(join(cur,name));
  a.download=name;
  document.body.appendChild(a);a.click();a.remove();
}

// 递归把 item（文件或目录）打包进 zip
async function collectIntoZip(zip,parentDir,item,prefix){
  const fullPath=join(parentDir,item.name);
  const entryName=prefix+item.name;
  if(item.dir){
    const r=await fetch('/files?path='+encodeURIComponent(fullPath));
    if(!r.ok) throw new Error('list '+fullPath+' failed');
    const j=await r.json();
    zip.folder(entryName);
    for(const child of (j.items||[])){
      if(!child.name) continue;
      await collectIntoZip(zip,fullPath,child,entryName+'/');
    }
  }else{
    const r=await fetch('/file?path='+encodeURIComponent(fullPath));
    if(!r.ok) throw new Error('download '+fullPath+' failed');
    const buf=await r.arrayBuffer();
    zip.file(entryName,buf);
  }
}

$('up').onclick=()=>{
  if(cur==='/') return;
  const idx=cur.lastIndexOf('/');
  load(idx<=0?'/':cur.substring(0,idx));
};
$('refresh').onclick=()=>load(cur);

$('download').onclick=async()=>{
  if(sel.size===0) return;
  const items=curItems.filter(it=>sel.has(it.name));
  // 单文件快路径：浏览器原生下载
  if(items.length===1&&!items[0].dir){
    doDownload(items[0].name);
    return;
  }
  if(typeof JSZip==='undefined'){
    fs.textContent='打包失败：JSZip 未加载（需互联网访问 cdnjs）';
    return;
  }
  fs.textContent='正在打包 '+items.length+' 项...';
  try{
    const zip=new JSZip();
    let i=0;
    for(const it of items){
      await collectIntoZip(zip,cur,it,'');
      i++;
      fs.textContent='正在打包... ('+i+'/'+items.length+')';
    }
    fs.textContent='生成 ZIP...';
    const blob=await zip.generateAsync({type:'blob'},m=>{
      fs.textContent='生成 ZIP... '+m.percent.toFixed(0)+'%';
    });
    const baseName=(items.length===1?items[0].name:('squid-'+cur.replace(/[\/\s]+/g,'_').replace(/^_+|_+$/g,'')||'root'));
    const a=document.createElement('a');
    a.href=URL.createObjectURL(blob);
    a.download=baseName+'.zip';
    document.body.appendChild(a);a.click();
    setTimeout(()=>{URL.revokeObjectURL(a.href);a.remove();},200);
    fs.textContent='✓ 下载完成: '+a.download+' ('+fmt(blob.size)+')';
  }catch(e){fs.textContent='打包失败: '+e.message;}
};

$('delete').onclick=async()=>{
  if(sel.size===0) return;
  const names=[...sel];
  const preview=names.length<=8?names.join('\n'):(names.slice(0,8).join('\n')+'\n...（共 '+names.length+' 项）');
  if(!confirm('确定删除以下 '+names.length+' 项？\n\n'+preview+'\n\n此操作不可恢复。')) return;
  fs.textContent='删除 '+names.length+' 项...';
  let ok=0,fail=0;
  for(const name of names){
    try{
      const r=await fetch('/file?path='+encodeURIComponent(join(cur,name)),{method:'DELETE'});
      if(r.ok) ok++; else fail++;
    }catch{fail++;}
    fs.textContent='删除中... ('+(ok+fail)+'/'+names.length+')';
  }
  fs.textContent='✓ 已删除 '+ok+(fail>0?'，失败 '+fail:'');
  load(cur);
};

$('upload').onclick=()=>$('filepick').click();
$('filepick').onchange=async(ev)=>{
  const files=[...ev.target.files];
  if(files.length===0) return;
  let ok=0,fail=0;
  for(let i=0;i<files.length;i++){
    const f=files[i];
    fs.textContent='上传 ('+(i+1)+'/'+files.length+') '+f.name+' ('+fmt(f.size)+')...';
    try{
      const fd=new FormData();
      fd.append('file',f,f.name);
      const r=await fetch('/upload?path='+encodeURIComponent(cur),{method:'POST',body:fd});
      if(!r.ok) throw new Error('HTTP '+r.status);
      ok++;
    }catch(e){fail++;}
  }
  fs.textContent='✓ 上传完成 '+ok+(fail>0?'，失败 '+fail:'');
  ev.target.value='';
  load(cur);
};

// ── 串口控制台（按字节而非字符前进 pos，避免 UTF-8 多字节漂移）──
async function poll(){
  try{
    const r=await fetch('/log?p='+pos);
    const buf=await r.arrayBuffer();
    const total=parseInt(r.headers.get('X-Total')||'0',10);
    if(buf.byteLength>0){
      const text=new TextDecoder('utf-8').decode(buf);
      out.textContent+=text;
      if(out.textContent.length>60000)
        out.textContent=out.textContent.substring(out.textContent.length-40000);
      out.scrollTop=out.scrollHeight;
    }
    if(total>0) pos=total;
  }catch(e){}
}
async function send(){
  const v=inp.value.trim();
  if(!v) return;
  inp.value='';
  out.textContent+='> '+v+'\n';
  out.scrollTop=out.scrollHeight;
  try{
    await fetch('/cmd',{method:'POST',headers:{'Content-Type':'text/plain'},body:v});
  }catch(e){out.textContent+='[发送失败: '+e.message+']\n';}
}
$('send').onclick=send;
inp.addEventListener('keydown',e=>{if(e.key==='Enter')send();});

load('/');
setInterval(poll,250);
poll();
inp.focus();
</script>
</body></html>
)HTML";

// ────────────────────────────────────────────────────────────────────
namespace {
bool hasValue(const char* v) { return v && v[0] != '\0'; }
constexpr byte DNS_PORT     = 53;
const char*    PROV_AP_SSID = "SquidRobot-Setup";
}

// ────────────────────────────────────────────────────────────────────
OtaManager::OtaManager()
    : _ready(false),
      _provisioningDone(false),
      _consoleActive(false),
      _routesRegistered(false),
      _ntpSynced(false),
      _ntpEpoch(0),
      _ntpSyncMs(0),
      _webServer(80) {}

// ────────────────────────────────────────────────────────────────────
void OtaManager::begin(bool underwaterMode) {
    _ready            = false;
    _provisioningDone = false;
    _consoleActive    = false;
    _logBuf           = "";   // 清空旧缓冲，防止 pos 越界返回空

    if (underwaterMode) {
        WiFi.mode(WIFI_OFF);
        g_dbg->println(F("OTA: underwater mode detected, WiFi disabled."));
        return;
    }

    WiFi.persistent(true);
    WiFi.setAutoReconnect(true);

    if (!beginNvsMode()) {
        g_dbg->println(F("WiFi: no saved credentials, starting provisioning portal."));
        beginProvisioningPortal();
    }

    configureArduinoOta();
    ArduinoOTA.begin();
    syncNtp();
    _ready = true;
}

// ────────────────────────────────────────────────────────────────────
void OtaManager::handle() {
    if (!_ready) return;
    ArduinoOTA.handle();
    if (_consoleActive) _webServer.handleClient();
}

bool OtaManager::isReady() const { return _ready; }
IPAddress OtaManager::getLocalIP() const { return WiFi.localIP(); }

// ────────────────────────────────────────────────────────────────────
// 1. 用 NVS 保存的凭据连接
// ────────────────────────────────────────────────────────────────────
bool OtaManager::beginNvsMode() {
    WiFi.mode(WIFI_STA);
    WiFi.begin();   // 使用 NVS 保存的 SSID/PASS，DHCP 自动分配 IP

    g_dbg->print(F("WiFi: trying NVS saved credentials"));
    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        g_dbg->print('.');
        delay(250);
    }
    g_dbg->println();

    if (WiFi.status() == WL_CONNECTED) {
        g_dbg->print(F("WiFi connected (NVS): "));
        g_dbg->println(WiFi.SSID());
        return true;
    }

    WiFi.disconnect(true);
    return false;
}

// ────────────────────────────────────────────────────────────────────
// 2. 配网热点 + Captive Portal（阻塞直到成功）
// ────────────────────────────────────────────────────────────────────
bool OtaManager::beginProvisioningPortal() {
    g_dbg->println(F("WiFi: starting provisioning portal..."));

    WiFi.mode(WIFI_AP);
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    if (!WiFi.softAP(PROV_AP_SSID)) {
        g_dbg->println(F("Provisioning AP failed."));
        return false;
    }
    g_dbg->print(F("Provisioning AP: "));
    g_dbg->print(PROV_AP_SSID);
    g_dbg->print(F("  IP: "));
    g_dbg->println(WiFi.softAPIP());

    _dnsServer.start(DNS_PORT, "*", apIP);
    WiFi.scanNetworks(true);

    _webServer.on("/",     HTTP_GET,  [this]() { handlePortalRoot(); });
    _webServer.on("/save", HTTP_POST, [this]() { handlePortalSave(); });
    _webServer.on("/scan", HTTP_GET,  [this]() { handlePortalScan(); });
    _webServer.onNotFound(            [this]() { handlePortalNotFound(); });
    _webServer.begin();

    g_dbg->println(F("Portal web server started."));
    g_dbg->println(F("Connect to 'SquidRobot-Setup' and open any webpage."));

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
// 配网网页处理
// ────────────────────────────────────────────────────────────────────
void OtaManager::handlePortalRoot() {
    String html = FPSTR(PORTAL_HTML);
    html.replace("{{SCAN}}", buildScanResults());
    _webServer.send(200, "text/html", html);
}

void OtaManager::handlePortalSave() {
    if (!_webServer.hasArg("ssid") || _webServer.arg("ssid").isEmpty()) {
        _webServer.send(400, "text/html",
            "<meta charset='UTF-8'><p style='color:red'>错误：SSID 不能为空</p>");
        return;
    }

    String ssid = _webServer.arg("ssid");
    String pass = _webServer.hasArg("pass") ? _webServer.arg("pass") : "";

    String html = FPSTR(PORTAL_SUCCESS_HTML);
    html.replace("{{SSID}}", ssid);
    _webServer.send(200, "text/html", html);

    g_dbg->print(F("[Portal] Trying: "));
    g_dbg->println(ssid);

    _webServer.stop();
    WiFi.softAPdisconnect(true);
    delay(300);  // 等待 AP 完全关闭再切换模式

    if (connectSta(ssid.c_str(), pass.c_str(), OTA_CONNECT_TIMEOUT_MS)) {
        _provisioningDone = true;
    } else {
        g_dbg->println(F("[Portal] Connect failed, restarting portal..."));
        delay(500);
        ESP.restart();
    }
}

void OtaManager::handlePortalScan() {
    WiFi.scanNetworks();
    _webServer.send(200, "text/html; charset=utf-8", buildScanResults());
}

void OtaManager::handlePortalNotFound() {
    _webServer.sendHeader("Location", "http://192.168.4.1/", true);
    _webServer.send(302, "text/plain", "");
}

String OtaManager::buildScanResults() {
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_RUNNING) return "<p style='color:#888;font-size:13px'>扫描中...</p>";
    if (n <= 0)                 return "<p style='color:#888;font-size:13px'>未发现网络</p>";

    String html = "";
    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        int    rssi = WiFi.RSSI(i);
        bool   enc  = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        html += "<div class='net' onclick=\"fill('" + ssid + "')\">";
        html += "<span>" + ssid + "</span>";
        html += "<span class='rssi'>" + String(rssi) + "dBm";
        if (enc) html += "<span class='sec'> &#x1F512;</span>";
        html += "</span></div>";
    }
    WiFi.scanDelete();
    return html;
}

// ────────────────────────────────────────────────────────────────────
// 通用 STA 连接
// ────────────────────────────────────────────────────────────────────
bool OtaManager::connectSta(const char* ssid, const char* pass, uint32_t timeoutMs) {
    // persistent(false)：先不写入 NVS，连接成功后再手动保存，避免保存错误密码
    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);  // 防止旧连接干扰
    WiFi.mode(WIFI_STA);
    delay(100);
    WiFi.begin(ssid, (pass && pass[0]) ? pass : nullptr);

    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
        g_dbg->print('.');
        delay(250);
    }
    g_dbg->println();

    if (WiFi.status() == WL_CONNECTED) {
        // 连接成功，将已验证的凭据持久化到 NVS（不断开连接）
        esp_wifi_set_storage(WIFI_STORAGE_FLASH);
        wifi_config_t conf;
        memset(&conf, 0, sizeof(conf));
        strlcpy(reinterpret_cast<char*>(conf.sta.ssid),     ssid, sizeof(conf.sta.ssid));
        if (pass && pass[0]) {
            strlcpy(reinterpret_cast<char*>(conf.sta.password), pass, sizeof(conf.sta.password));
        }
        esp_wifi_set_config(WIFI_IF_STA, &conf);
        WiFi.setAutoReconnect(true);  // 恢复自动重连
        WiFi.persistent(true);

        g_dbg->print(F("WiFi connected: "));
        g_dbg->print(WiFi.SSID());
        g_dbg->print(F("  IP: "));
        g_dbg->println(WiFi.localIP());
        return true;
    }

    // 连接失败：不保存凭据，下次重启直接进入配网
    WiFi.persistent(true);
    WiFi.disconnect(true);
    g_dbg->println(F("WiFi connect timeout."));
    return false;
}

// ────────────────────────────────────────────────────────────────────
// NTP
// ────────────────────────────────────────────────────────────────────
bool OtaManager::syncNtp() {
    // UTC+9（韩国标准时间 KST）
    configTime(9 * 3600, 0, "pool.ntp.org", "time.google.com", "kr.pool.ntp.org");
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 5000)) {
        g_dbg->println(F("NTP: sync failed."));
        return false;
    }
    _ntpEpoch  = mktime(&timeinfo);
    _ntpSyncMs = millis();
    _ntpSynced = true;

    char buf[24];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    g_dbg->print(F("NTP synced: "));
    g_dbg->println(buf);
    return true;
}

bool OtaManager::hasTime() const { return _ntpSynced; }

void OtaManager::getSessionFolderName(char* buf, size_t len) const {
    if (!_ntpSynced) {
        strncpy(buf, "no_time", len);
        return;
    }
    const time_t now = _ntpEpoch + (time_t)((millis() - _ntpSyncMs) / 1000UL);
    struct tm* t = localtime(&now);
    strftime(buf, len, "%Y-%m-%d_%H-%M-%S", t);
}

// ────────────────────────────────────────────────────────────────────
// 浏览器串口控制台（HTTP 轮询，无需额外库）
// ────────────────────────────────────────────────────────────────────
void OtaManager::beginWebConsole(std::function<void(uint8_t*, size_t)> msgCallback) {
    if (!_ready) return;
    _cmdCb = msgCallback;
    _consolePrint.setBuffer(&_logBuf);

    // 路由只注册一次——重复调用（enterDebugMode 场景）只需重启监听即可
    if (!_routesRegistered) {
        // 串口控制台
        _webServer.on("/console", HTTP_GET,    [this]() { handleConsoleHtml(); });
        _webServer.on("/log",     HTTP_GET,    [this]() { handleConsoleLog(); });
        _webServer.on("/cmd",     HTTP_POST,   [this]() { handleConsoleCmd(); });

        // SD 文件管理
        _webServer.on("/files",   HTTP_GET,    [this]() { handleFileList(); });
        _webServer.on("/file",    HTTP_GET,    [this]() { handleFileDownload(); });
        _webServer.on("/file",    HTTP_DELETE, [this]() { handleFileDelete(); });

        // 文件上传：multipart/form-data 流式接收（避免大文件 OOM）
        _webServer.on("/upload",  HTTP_POST,
            [this]() {
                _webServer.sendHeader("Access-Control-Allow-Origin", "*");
                _webServer.send(200, "text/plain", "OK");
            },
            [this]() { handleFileUploadStream(); });

        _routesRegistered = true;
    }

    _webServer.stop();   // 先停，再 begin，确保干净重启
    _webServer.begin();
    _consoleActive = true;
    g_dbg->print(F("HTTP: http://"));
    g_dbg->print(WiFi.localIP());
    g_dbg->println(F("  /log /files /console"));
}

Print* OtaManager::getConsolePrint() { return &_consolePrint; }

void OtaManager::handleConsoleHtml() {
    _webServer.send_P(200, "text/html", CONSOLE_HTML);
}

void OtaManager::handleConsoleLog() {
    // pos 是浏览器记住的"我已经看到的总字节数"（按 _consolePrint.getTotal() 的口径）。
    // X-Total 响应头返回新的 total，浏览器据此前进 pos——不再依赖 JS string.length，
    // 这样中文等多字节字符也不会让 pos 漂移。
    const size_t pos      = _webServer.hasArg("p") ? (size_t)_webServer.arg("p").toInt() : 0;
    const size_t total    = _consolePrint.getTotal();
    const size_t bufStart = _consolePrint.getBufStart();

    _webServer.sendHeader("X-Total", String((uint32_t)total));
    _webServer.sendHeader("Access-Control-Allow-Origin", "*");

    if (pos >= total) {
        _webServer.send(200, "application/octet-stream", "");
        return;
    }
    if (pos < bufStart) {
        // 客户端落后于滑动窗口起点，发整个当前缓冲并附提示
        String body = "[... lost " + String((uint32_t)(bufStart - pos)) + " bytes ...]\n";
        body += _logBuf;
        _webServer.send(200, "application/octet-stream", body);
        return;
    }
    _webServer.send(200, "application/octet-stream", _logBuf.substring(pos - bufStart));
}

void OtaManager::handleConsoleCmd() {
    if (_cmdCb && _webServer.hasArg("plain")) {
        String cmd = _webServer.arg("plain");
        cmd.trim();
        if (cmd.length() > 0)
            _cmdCb(reinterpret_cast<uint8_t*>(cmd.begin()), cmd.length());
    }
    _webServer.send(200, "text/plain", "OK");
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
        g_dbg->println(F("\nOTA start"));
    });
    ArduinoOTA.onEnd([]() {
        g_dbg->println(F("\nOTA end"));
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static uint8_t last = 255;
        const uint8_t  pct  = total ? (uint8_t)((progress * 100U) / total) : 0;
        if (pct != last && pct % 10 == 0) {
            last = pct;
            g_dbg->printf("OTA: %u%%\n", pct);
        }
    });
    ArduinoOTA.onError([](ota_error_t e) {
        g_dbg->printf("OTA error[%u]\n", (unsigned)e);
    });
}

// ────────────────────────────────────────────────────────────────────
// HTTP SD 文件管理接口
// ────────────────────────────────────────────────────────────────────

// GET /files?path=/          → JSON 目录列表
// {"items":[{"name":"xxx","dir":true},{"name":"yyy","dir":false,"size":1234}]}
void OtaManager::handleFileList() {
    _webServer.sendHeader("Access-Control-Allow-Origin", "*");
    String path = _webServer.hasArg("path") ? _webServer.arg("path") : "/";
    if (path.isEmpty()) path = "/";

    File dir = SD.open(path);
    if (!dir || !dir.isDirectory()) {
        _webServer.send(404, "application/json", "{\"error\":\"not a directory\"}");
        return;
    }

    String json = "{\"path\":\"" + path + "\",\"items\":[";
    bool first = true;
    File entry = dir.openNextFile();
    while (entry) {
        if (!first) json += ",";
        first = false;
        json += "{\"name\":\"";
        json += entry.name();
        json += "\",\"dir\":";
        json += entry.isDirectory() ? "true" : "false";
        if (!entry.isDirectory()) {
            json += ",\"size\":";
            json += entry.size();
        }
        json += "}";
        entry.close();
        entry = dir.openNextFile();
    }
    json += "]}";
    dir.close();
    _webServer.send(200, "application/json", json);
}

// GET /file?path=/xxx/yyy    → 下载文件
void OtaManager::handleFileDownload() {
    _webServer.sendHeader("Access-Control-Allow-Origin", "*");
    if (!_webServer.hasArg("path")) {
        _webServer.send(400, "text/plain", "missing path");
        return;
    }
    String path = _webServer.arg("path");
    File f = SD.open(path, FILE_READ);
    if (!f || f.isDirectory()) {
        _webServer.send(404, "text/plain", "not found");
        return;
    }
    // 根据扩展名设置 Content-Type
    String ct = "application/octet-stream";
    if (path.endsWith(".csv"))  ct = "text/csv";
    if (path.endsWith(".log"))  ct = "text/plain";
    if (path.endsWith(".txt"))  ct = "text/plain";
    if (path.endsWith(".json")) ct = "application/json";

    _webServer.sendHeader("Content-Disposition",
        "attachment; filename=\"" + path.substring(path.lastIndexOf('/') + 1) + "\"");
    _webServer.streamFile(f, ct);
    f.close();
}

// POST /upload?path=/dir   → multipart/form-data 流式上传到 dir/<filename>
// onUpload 回调在 START/WRITE/END 三个状态依次触发，避免一次性把文件读进 RAM。
void OtaManager::handleFileUploadStream() {
    HTTPUpload& up = _webServer.upload();

    if (up.status == UPLOAD_FILE_START) {
        String dir = _webServer.hasArg("path") ? _webServer.arg("path") : "/";
        if (dir.isEmpty()) dir = "/";
        if (!dir.endsWith("/")) dir += "/";
        // 防御：filename 可能含子路径，只取叶名
        String leaf = up.filename;
        int lastSlash = leaf.lastIndexOf('/');
        if (lastSlash >= 0) leaf = leaf.substring(lastSlash + 1);
        if (leaf.isEmpty()) {
            g_dbg->println(F("[Upload] empty filename"));
            return;
        }
        const String full = dir + leaf;
        if (_uploadFile) _uploadFile.close();
        if (SD.exists(full)) SD.remove(full);
        _uploadFile = SD.open(full, FILE_WRITE);
        if (!_uploadFile) {
            g_dbg->print(F("[Upload] open failed: "));
            g_dbg->println(full);
        } else {
            g_dbg->print(F("[Upload] start: "));
            g_dbg->println(full);
        }
    } else if (up.status == UPLOAD_FILE_WRITE) {
        if (_uploadFile) _uploadFile.write(up.buf, up.currentSize);
    } else if (up.status == UPLOAD_FILE_END) {
        if (_uploadFile) {
            _uploadFile.close();
            g_dbg->print(F("[Upload] done: "));
            g_dbg->print((uint32_t)up.totalSize);
            g_dbg->println(F(" bytes"));
        }
    } else if (up.status == UPLOAD_FILE_ABORTED) {
        if (_uploadFile) _uploadFile.close();
        g_dbg->println(F("[Upload] aborted"));
    }
}

// 递归删除：先清空目录内所有子项，再 rmdir
bool OtaManager::removeRecursive(const String& path) {
    File entry = SD.open(path);
    if (!entry) return false;
    if (!entry.isDirectory()) {
        entry.close();
        return SD.remove(path);
    }

    File child = entry.openNextFile();
    while (child) {
        // child.name() 在新 ESP32 core 返回叶名，老版本返回完整路径——两者都兼容
        String childName = child.name();
        String childPath;
        if (childName.startsWith("/")) {
            childPath = childName;
        } else {
            childPath = path;
            if (!childPath.endsWith("/")) childPath += "/";
            childPath += childName;
        }
        child.close();
        if (!removeRecursive(childPath)) {
            entry.close();
            return false;
        }
        child = entry.openNextFile();
    }
    entry.close();
    return SD.rmdir(path);
}

// DELETE /file?path=/xxx/yyy → 递归删除文件或目录
void OtaManager::handleFileDelete() {
    _webServer.sendHeader("Access-Control-Allow-Origin", "*");
    if (!_webServer.hasArg("path")) {
        _webServer.send(400, "text/plain", "missing path");
        return;
    }
    const String path = _webServer.arg("path");
    if (path == "/" || path.isEmpty()) {
        _webServer.send(400, "text/plain", "refuse to delete root");
        return;
    }
    if (!SD.exists(path)) {
        _webServer.send(404, "text/plain", "not found");
        return;
    }
    const bool ok = removeRecursive(path);
    _webServer.send(ok ? 200 : 500, "text/plain", ok ? "OK" : "delete failed");
}
