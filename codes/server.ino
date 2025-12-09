#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>

// ===== WiFi Config =====
const char* ssid = "laptop";
const char* password = "laptop11";

// ===== Web Server =====
WebServer server(80);

// ===== Anchors (fixed positions) =====
float anchors[3][2] = {
  {0.0, 0.0},    // Anchor 1
  {5.0, 0.0},    // Anchor 2
  {0.0, 5.0}     // Anchor 3
};

// ===== BLE Config =====
#define SCAN_TIME 2
BLEScan* pBLEScan;
String targetMac = "";
String lastHtml = "";

// ===== ESP-NOW Peers (Anchors) =====
uint8_t anchor1[] = {0x8C,0x4F,0x00,0xAD,0x87,0x6C};
uint8_t anchor2[] = {0x88,0x57,0x21,0x8B,0xBC,0x68};
uint8_t anchor3[] = {0x38,0x18,0x2B,0x84,0x08,0x04};

// ===== Structs =====
typedef struct {
  char mac[18]; // target MAC from server
} TargetMac;

typedef struct {
  int senderID;
  char mac[18];
  float avgRssi;
  float avgDistance;
  float kalmanRssi;
  float kalmanDistance;
} ScanResult;

ScanResult incomingReading;
ScanResult scannerData[3];
bool gotData[3] = {false};

// ===== Min-Max =====
void minMaxMethod(float d[3], float result[2]) {
  float x1 = anchors[0][0], y1 = anchors[0][1];
  float x2 = anchors[1][0], y2 = anchors[1][1];
  float x3 = anchors[2][0], y3 = anchors[2][1];
  float d1 = d[0], d2 = d[1], d3 = d[2];

  float xmin = max(max(x1 - d1, x2 - d2), x3 - d3);
  float xmax = min(min(x1 + d1, x2 + d2), x3 + d3);
  float ymin = max(max(y1 - d1, y2 - d2), y3 - d3);
  float ymax = min(min(y1 + d1, y2 + d2), y3 + d3);

  result[0] = (xmin + xmax) / 2.0;
  result[1] = (ymin + ymax) / 2.0;
}

// ===== Trilateration =====
bool trilaterationMethod(float d[3], float result[2]) {
  float x1 = anchors[0][0], y1 = anchors[0][1];
  float x2 = anchors[1][0], y2 = anchors[1][1];
  float x3 = anchors[2][0], y3 = anchors[2][1];
  float d1 = d[0], d2 = d[1], d3 = d[2];

  float A11 = 2 * (x2 - x1);
  float A12 = 2 * (y2 - y1);
  float A21 = 2 * (x3 - x1);
  float A22 = 2 * (y3 - y1);

  float b1 = d1*d1 - d2*d2 + x2*x2 - x1*x1 + y2*y2 - y1*y1;
  float b2 = d1*d1 - d3*d3 + x3*x3 - x1*x1 + y3*y3 - y1*y1;

  float det = A11*A22 - A12*A21;
  if (fabs(det) < 1e-6) return false;

  result[0] = (b1*A22 - b2*A12) / det;
  result[1] = (A11*b2 - A21*b1) / det;
  return true;
}

// ===== ESP-NOW Receive Callback =====
void onDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  if (len != sizeof(ScanResult)) return;
  memcpy(&incomingReading, incomingData, sizeof(incomingReading));

  int id = incomingReading.senderID - 1;
  if (id < 0 || id >= 3) return;

  scannerData[id] = incomingReading;
  gotData[id] = true;

  Serial.printf("Received from Anchor %d -> RSSI: %.2f | Dist: %.2f | Kalman Dist: %.2f\n",
                incomingReading.senderID,
                incomingReading.avgRssi,
                incomingReading.avgDistance,
                incomingReading.kalmanDistance);

  if (gotData[0] && gotData[1] && gotData[2]) {
    float d_std[3], d_kal[3];
    for (int i = 0; i < 3; i++) {
      d_std[i] = scannerData[i].avgDistance;
      d_kal[i] = scannerData[i].kalmanDistance;
    }

    // --- Calculate both positions ---
    float minmaxStd[2], minmaxKal[2];
    minMaxMethod(d_std, minmaxStd);
    minMaxMethod(d_kal, minmaxKal);

    float trilatStd[2], trilatKal[2];
    bool okStd = trilaterationMethod(d_std, trilatStd);
    bool okKal = trilaterationMethod(d_kal, trilatKal);

    Serial.println("\n--- Final Results ---");
    Serial.printf("STD Trilateration: (%.2f, %.2f)\n", trilatStd[0], trilatStd[1]);
    Serial.printf("KALMAN Trilateration: (%.2f, %.2f)\n", trilatKal[0], trilatKal[1]);
    Serial.println("---------------------\n");

    // ===== HTML Visualization (Side by Side) =====
    lastHtml = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="utf-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <style>
        body { background:#121212; color:white; font-family:Arial; text-align:center; }
        .row { display:flex; flex-wrap:wrap; justify-content:center; }
        .card { background:#1e1e1e; margin:15px; padding:15px;
                border-radius:12px; width:90%; max-width:420px; }
        h2 { color:#00e676; }
        ul { list-style:none; padding:0; }
        li { background:#222; margin:5px 0; padding:8px; border-radius:6px; text-align:left; }
        canvas { background:#222; border-radius:8px; margin-top:15px; }
        button { padding:8px 15px; margin:10px; border:none; border-radius:8px;
                 background:#00e676; color:#000; cursor:pointer; }
        button:hover { background:#1de9b6; }
      </style>
    </head>
    <body>
      <h2>Localization Comparison</h2>
      <div class="row">
    )rawliteral";

    // ---- Standard Averaging Section ----
    lastHtml += "<div class='card'><h3>Standard Averaging</h3><ul>";
    for (int i=0;i<3;i++) {
      lastHtml += "<li>Anchor " + String(i+1) +
                  " → Distance: <b>" + String(d_std[i],2) + " m</b></li>";
    }
    lastHtml += "</ul>";
    lastHtml += "<h4>Min-Max: ("+String(minmaxStd[0],2)+","+String(minmaxStd[1],2)+")</h4>";
    lastHtml += "<h4>Trilateration: ("+String(trilatStd[0],2)+","+String(trilatStd[1],2)+")</h4>";

    lastHtml += "<canvas id='stdMap' width='400' height='400'></canvas></div>";

    // ---- Kalman Filter Section ----
    lastHtml += "<div class='card'><h3>Kalman Filtered</h3><ul>";
    for (int i=0;i<3;i++) {
      lastHtml += "<li>Anchor " + String(i+1) +
                  " → Distance: <b>" + String(d_kal[i],2) + " m</b></li>";
    }
    lastHtml += "</ul>";
    lastHtml += "<h4>Min-Max: ("+String(minmaxKal[0],2)+","+String(minmaxKal[1],2)+")</h4>";
    lastHtml += "<h4>Trilateration: ("+String(trilatKal[0],2)+","+String(trilatKal[1],2)+")</h4>";

    lastHtml += "<canvas id='kalMap' width='400' height='400'></canvas></div>";

    lastHtml += "</div><a href='/'><button>Back</button></a>";

    // --- JavaScript to draw both graphs ---
    // --- JavaScript to draw both graphs ---
lastHtml += R"rawliteral(
  <script>
    function draw(canvasId, anchors, pos, color, label) {
      var ctx = document.getElementById(canvasId).getContext('2d');
      ctx.font = "12px Arial";
      ctx.fillStyle = "white";
      function pt(x,y,c,t) {
        ctx.fillStyle=c;
        ctx.beginPath();
        ctx.arc(x*50+50, 350-y*50, 6, 0, 2*Math.PI);
        ctx.fill();
        ctx.fillText(t, x*50+60, 350-y*50);
      }
      for(let i=0;i<3;i++) pt(anchors[i][0],anchors[i][1],'blue','A'+(i+1));
      pt(pos[0],pos[1],color,label);
    }

    var anchors = [[0,0],[5,0],[0,5]];
    draw('stdMap', anchors, [)rawliteral" + String(trilatStd[0]) + "," + String(trilatStd[1]) + R"rawliteral(], 'red', 'STD');
    draw('kalMap', anchors, [)rawliteral" + String(trilatKal[0]) + "," + String(trilatKal[1]) + R"rawliteral(], 'red', 'KAL');
    draw('stdMap', anchors, [)rawliteral" + String(minmaxStd[0]) + "," + String(minmaxStd[1]) + R"rawliteral(], 'green', 'MIN');
    draw('kalMap', anchors, [)rawliteral" + String(minmaxKal[0]) + "," + String(minmaxKal[1]) + R"rawliteral(], 'green', 'MIN');
  </script></body></html>
)rawliteral";

  }
}

// ===== Web UI Handlers =====
void handleRoot() {
  BLEScanResults* results = pBLEScan->start(SCAN_TIME, false);
  String html = "<!DOCTYPE html><html><head>\
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
  <style>\
    body { background:#121212; color:white; font-family:Arial; text-align:center;}\
    table { margin:auto; border-collapse:collapse; width:90%; max-width:700px;}\
    th, td { border:1px solid #333; padding:10px;}\
    th { background:#00e676; color:#000;}\
    tr:hover { background:#1e1e1e;}\
    button { padding:8px 15px; margin:5px; border:none; border-radius:8px;\
             background:#00e676; color:#000; cursor:pointer;}\
    button:hover { background:#1de9b6;}\
  </style></head><body>\
  <h2>ESP32 BLE Device List</h2>\
  <table><tr><th>MAC</th><th>Name</th><th>Select</th></tr>";

  for (int i=0;i<results->getCount();i++) {
    BLEAdvertisedDevice d = results->getDevice(i);
    String mac = d.getAddress().toString().c_str();
    mac.toLowerCase();
    String name = d.getName().c_str();
    if (name == "") name = "(Unknown)";
    html += "<tr><td>"+mac+"</td><td>"+name+"</td>\
                   <td><a href='/select?mac="+mac+"'><button>Track</button></a></td></tr>";
  }
  pBLEScan->clearResults();

  html += "</table></body></html>";
  server.send(200, "text/html", html);
}

void handleSelect() {
  if (!server.hasArg("mac")) {
    server.send(400,"text/plain","No MAC given");
    return;
  }
  targetMac = server.arg("mac");
  TargetMac t;
  strncpy(t.mac, targetMac.c_str(), sizeof(t.mac));
  t.mac[sizeof(t.mac)-1] = '\0';

  esp_now_send(anchor1,(uint8_t*)&t,sizeof(t));
   Serial.print("Tracking selected MAC: ");
Serial.println(t.mac);
  esp_now_send(anchor2,(uint8_t*)&t,sizeof(t));
  Serial.print("Tracking selected MAC: ");
Serial.println(t.mac);

  esp_now_send(anchor3,(uint8_t*)&t,sizeof(t));
   Serial.print("Tracking selected MAC: ");
Serial.println(t.mac);


  String html = R"rawliteral(
  <!DOCTYPE html>
  <html><head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body { background:#121212; color:white; font-family:Arial; text-align:center; }
      h2 { color:#00e676; margin-top:40px; }
      button { padding:10px 20px; margin:20px; border:none; border-radius:8px;
               background:#00e676; color:#000; cursor:pointer; }
      button:hover { background:#1de9b6; }
      #timer { font-size:18px; margin-top:20px; color:#bbb; }
    </style>
  </head><body>
    <h2>Target )rawliteral" + targetMac + R"rawliteral( sent to anchors!</h2>
    <p id="timer">Redirecting to results in 60s...</p>
    <a href='/results'><button>See Results</button></a>
    <script>
      var t = 60;
      var timerEl = document.getElementById("timer");
      var countdown = setInterval(function(){
        t--;
        timerEl.innerHTML = "Redirecting to results in " + t + "s...";
        if(t <= 0){
          clearInterval(countdown);
          window.location.href = "/results";
        }
      },1000);
    </script></body></html>)rawliteral";

  server.send(200,"text/html",html);
}

void handleResults() {
  if (lastHtml=="") server.send(200,"text/html","<h2>No results yet</h2>");
  else server.send(200,"text/html",lastHtml);
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);

  if (esp_now_init()!=ESP_OK) {
    Serial.println("ESP-NOW init failed!"); return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, anchor1, 6); peerInfo.channel=0; peerInfo.encrypt=false; esp_now_add_peer(&peerInfo);
  memcpy(peerInfo.peer_addr, anchor2, 6); peerInfo.channel=0; peerInfo.encrypt=false; esp_now_add_peer(&peerInfo);
  memcpy(peerInfo.peer_addr, anchor3, 6); peerInfo.channel=0; peerInfo.encrypt=false; esp_now_add_peer(&peerInfo);

  server.on("/",handleRoot);
  server.on("/select",handleSelect);
  server.on("/results",handleResults);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
