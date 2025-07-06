#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"
#include <ESP8266WebServer.h>

const char* ssid = "10Diem"; // Tên WiFi
const char* password = "01010101"; // M?t kh?u WiFi

DHTesp dht;
int dhtPin = 16;
float temp;
float humi;
unsigned long currentTime = millis();

// C?u hình màn hình OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Kh?i t?o Web Server
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println();

  // C?u hình c?m bi?n DHT11
  dht.setup(dhtPin, DHTesp::DHT11);

  // C?u hình màn hình OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  // Hi?n th? thông tin kh?i d?ng trên OLED
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(F("IOT-ESP8266-K62"));
  display.display();
  delay(2000);
  display.clearDisplay();

  // K?t n?i WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Hi?n th? thông tin WiFi trên OLED
  display.setCursor(0, 0);
  display.print(F("WiFi connected"));
  display.setCursor(0, 16);
  display.print(WiFi.localIP());
  display.display();
  delay(2000);
  display.clearDisplay();

  // C?u hình Web Server
  server.on("/", handleRoot);
  server.on("/data", handleJson);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // C?p nh?t d? li?u t? c?m bi?n m?i giây
  if (millis() - currentTime > 1000) {
    float h = dht.getHumidity();
    float t = dht.getTemperature();
    if (dht.getStatusString() == "OK") {
      temp = t;
      humi = h;

      // Hi?n th? d? li?u lên OLED
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print(F("T: "));
      display.print(temp, 1);
      display.print((char)223); // Ký t? d?
      display.print(F("C"));

      display.setCursor(64, 0);
      display.print(F("H: "));
      display.print(humi, 1);
      display.print(F("%"));

      display.setCursor(0, 24);
      display.print(F("Nghia-CQ.62.KS.KTVT"));
      display.display();
    }
    currentTime = millis();
  }

  // X? lý yêu c?u t? Web Server
  server.handleClient();
}

// G?i trang web chính
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>IOT-62</title>";
  html += "<meta charset='UTF-8'>";
  html += "<style>";
  html += "body { background-color: #222; color: MediumSeaGreen; text-align: center; font-family: Arial, sans-serif; }";
  html += "h1 { margin-top: 20px; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>C?M BI?N NHI?T Ð? VÀ Ð? ?M</h1>";
  html += "<p>Nhi?t d?: <span id='temp'>--</span> &deg;C</p>";
  html += "<p>Ð? ?m: <span id='humi'>--</span> %</p>";
  html += "<script>";
  html += "function fetchData() {";
  html += "  fetch('/data').then(response => response.json()).then(data => {";
  html += "    document.getElementById('temp').innerText = data.temperature;";
  html += "    document.getElementById('humi').innerText = data.humidity;";
  html += "  });";
  html += "}";
  html += "setInterval(fetchData, 1000);"; // C?p nh?t m?i giây
  html += "fetchData();";
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// G?i d? li?u JSON
void handleJson() {
  String json = "{";
  json += "\"temperature\":" + String(temp, 1) + ",";
  json += "\"humidity\":" + String(humi, 1);
  json += "}";
  server.send(200, "application/json", json);
}

