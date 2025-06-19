#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <Wire.h>
#define RXD2 16
#define TXD2 17
const char* ap_ssid = "DA_1v1";
const char* ap_password = "327681122";
float temperature = 0;
float humidity = 0;
unsigned year = 0,month = 0,day = 0,hour = 0,minute = 0,second = 0;
AsyncWebServer server(80);
Preferences preferences;
unsigned long previousMillis = 0;
const long interval = 5000; 
void saveWiFiConfig(String ssid, String password) {
    preferences.begin("wifi-config", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
    Serial.println("✅ WiFi saved! Restart ESP...");
    delay(3000);
    ESP.restart();
}
void connectWiFi() {
    preferences.begin("wifi-config", true);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();

    if (ssid != "") {
        Serial.println("🔗 Connecting to WiFi: " + ssid);
        WiFi.begin(ssid.c_str(), password.c_str());

        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n✅ WiFi connected successfully!");
            Serial.print("📶 ESP32 at IP: http://");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("\n❌ WiFi failed! Create AP to enter WiFi.");
            WiFi.softAP(ap_ssid, ap_password);
            Serial.print("📡 Access the configuration at: http://");
            Serial.println(WiFi.softAPIP());
        }
    } else {
        Serial.println("⚠️ Saved WiFi not found, enter AP mode!");
        WiFi.softAP(ap_ssid, ap_password);
        Serial.print("📡 Access the configuration at: http://");
        Serial.println(WiFi.softAPIP());
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
    connectWiFi();
    Serial.println("WiFi AP đã tạo!");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("ESP32 đang hoạt động tại IP: " + WiFi.localIP().toString());
    }
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
              "<title>WiFi Settings</title>"
              "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
              "<script src='https://cdn.jsdelivr.net/npm/chartjs-plugin-datalabels'></script>"
              "<style>"
              "body { font-family: Arial, sans-serif; background-color: #f2f2f2; color: #333; padding: 20px; text-align: center; }"
              "h1, h2 { color: #0066cc; font-size: 28px; margin-bottom: 20px; }"
              "form { background-color: #fff; display: inline-block; text-align: left; padding: 20px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); max-width: 400px; margin: 0 auto 20px; }"
              "label { display: block; margin-top: 10px; font-weight: bold; }"
              "input[type='text'], input[type='password'] { width: 100%; padding: 8px; margin-top: 5px; border: 1px solid #ccc; border-radius: 4px; }"
              "input[type='submit'] { margin-top: 15px; padding: 10px 20px; background-color: #28a745; color: white; border: none; border-radius: 4px; cursor: pointer; }"
              "input[type='submit']:hover { background-color: #218838; }"
              "p { margin: 8px 0; font-size: 16px; }"
              ".chart-container { display: flex; flex-direction: column; align-items: center; margin-top: 30px; }"
              "canvas { max-width: 600px; width: 90%; margin: 20px auto; }"
              "</style></head><body>";

html += "<h1>WiFi Settings</h1>"
        "<form action='/config' method='POST'>"
        "<label>WiFi SSID:</label><input type='text' name='ssid'>"
        "<label>Password:</label><input type='password' name='password'>"
        "<input type='submit' value='Check WiFi'>"
        "</form>"

        "<h2>Information for PIC16F887</h2>"
        "<p><strong>Real date:</strong> <span id='date'></span></p>"
        "<p><strong>Real time:</strong> <span id='time'></span></p>"
        "<p><strong>Temperature:</strong> <span id='temperature'></span> °C</p>"
        "<p><strong>Humidity:</strong> <span id='humidity'></span> %</p>"

        "<div class='chart-container'>"
        "<canvas id='sensorChart'></canvas>"
        "</div>"

        "<script>"
        "let chartData = { labels: [], datasets: ["
        "{ label: 'Temperature (°C)', borderColor: 'red', data: [], fill: false },"
        "{ label: 'Humidity (%)', borderColor: 'blue', data: [], fill: false }"
        "]};"

        "let ctx = document.getElementById('sensorChart').getContext('2d');"
        "let sensorChart = new Chart(ctx, {"
        "    type: 'line',"
        "    data: chartData,"
        "    options: {"
        "        plugins: {"
        "            datalabels: {"
        "                anchor: 'end',"
        "                align: 'top',"
        "                formatter: function(value) { return value.toFixed(1); },"
        "                font: { weight: 'bold' }"
        "            }"
        "        },"
        "        scales: {"
        "            x: { title: { display: true, text: 'Time' } },"
        "            y: { beginAtZero: true, suggestedMax: 100 }"
        "        }"
        "    },"
        "    plugins: [ChartDataLabels]"
        "});"

        "function updateChart(data) {"
        "    let now = new Date().toLocaleTimeString();"
        "    chartData.labels.push(now);"
        "    chartData.datasets[0].data.push(data.temperature);"
        "    chartData.datasets[1].data.push(data.humidity);"
        "    if (chartData.labels.length > 10) {"
        "        chartData.labels.shift();"
        "        chartData.datasets[0].data.shift();"
        "        chartData.datasets[1].data.shift();"
        "    }"
        "    sensorChart.update();"
        "}"

        "setInterval(() => {"
        "    fetch('/sensor').then(r => r.json()).then(d => {"
        "        document.getElementById('date').innerText = d.date;"
        "        document.getElementById('time').innerText = d.time;"
        "        document.getElementById('temperature').innerText = d.temperature;"
        "        document.getElementById('humidity').innerText = d.humidity;"
        "        updateChart(d);"
        "    });"
        "}, 5000);"
        "</script></body></html>";


    request->send(200, "text/html", html);
});


    server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            saveWiFiConfig(request->getParam("ssid", true)->value(), request->getParam("password", true)->value());
            request->send(200, "text/plain", "✅ WiFi saved! Restart ESP...");
        } else {
            request->send(400, "text/plain", "⚠️ Missing information for WiFi!");
        }
    });
    server.on("/sensor", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{ \"date\": \"" + String(day) + "-" + String(month) + "-" + String(year) + " (DD-MM-YY) "+
                  "\", \"time\": \"" + String(hour) + ":" + String(minute) + ":" + String(second) + " (HH:MM:SS) "+ 
                  "\", \"temperature\": " + String(temperature) + 
                  ", \"humidity\": " + String(humidity) + " }";
        request->send(200, "application/json", json);
    });

    server.begin();
}
void processInput(String data) {
  int firstCommaIndex = data.indexOf(';');
  String doam = data.substring(0, firstCommaIndex); 
  String tempandtime = data.substring(firstCommaIndex + 1); 
  humidity = doam.toFloat();
  int secondCommaIndex = tempandtime.indexOf(';');
  String nhietdo = tempandtime.substring(0, secondCommaIndex); 
  temperature = nhietdo.toFloat();
  
  String realtime = tempandtime.substring(secondCommaIndex + 1);
  int values[6];  
  int index = 0;
  char realtimeChar[realtime.length() + 1]; 
  realtime.toCharArray(realtimeChar, sizeof(realtimeChar)); 
  char *token = strtok(realtimeChar, ":");
  while (token != NULL && index < 6) {
      values[index] = atoi(token);  
      index++;
      token = strtok(NULL, ":");
  }
  year = values[0]+2000;
  month = values[1];
  day = values[2];
  hour = values[3];
  minute = values[4];
  second = values[5];
  
}
void loop() {
    if (Serial2.available()) {
      String data = Serial2.readStringUntil('\n'); 
      if (data.length() > 0) {
        processInput(data); 
      }
    }
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        Serial.println("Update Data...");
    }
}
