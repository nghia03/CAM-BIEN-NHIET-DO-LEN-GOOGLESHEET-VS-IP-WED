#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h> // Thu vien I2C
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"
#include <ESP8266WebServer.h> // Thu vien Web Server

// Khai báo thông tin ket noi WiFi và Google Apps Script
const char* host = "script.google.com";
const int httpsPort = 443;
String GAS_ID = "AKfycby01INe_YO24Cxy4FQwV8pddYQ7wCu8phKQ8PzrBfqQTo7fbo5fDfEQNSEokfCOpw4j";
WiFiClientSecure client;
const char* ssid = "HELLO E";     // Tên wifi
const char* password = "01010101";  // Mat khau

DHTesp dht;
int dhtPin = 16;
float temp;
float humi;
unsigned long currentTime = millis();
unsigned long currentTime2 = millis();

// Khai báo màn h́nh OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (ho?c -1 n?u chia s? chân reset c?a Arduino)
#define SCREEN_ADDRESS 0x3C // Đ?a ch? màn h́nh OLED

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Khai báo Web Server ch?y trên ESP8266
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  dht.setup(dhtPin, DHTesp::DHT11);

  // Kh?i t?o màn h́nh OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000); // Hi?n th? chào m?ng trong 2 giây
  display.clearDisplay();

  // Hi?n th? "Welcome!" trên màn h́nh OLED
  display.setTextSize(1);             
  display.setTextColor(SSD1306_WHITE);  
  display.setCursor(0, 0);             
  display.print(F("IOT-ESP8266-K62"));
  display.display();
  delay(2000); // Hi?n th? trong 2 giây

  // K?t n?i WiFi
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setInsecure();

  display.clearDisplay();
  display.setCursor(0, 0);        
  display.print(F("T:"));    
  display.setCursor(32, 0);        
  display.print(F("H:"));
  display.display();

  // C?u h́nh Web Server
  server.on("/", handleRoot); // Đinh nghia du?ng d?n chính
  server.begin();             // BAt dAu Web Server
  Serial.println("HTTP server started");
}

void loop() {
  // doc du lieu tu cam bien DHT11
  if (millis() - currentTime > 1000) {
    float h = dht.getHumidity();
    float t = dht.getTemperature();
    Serial.print(dht.getStatusString());
    Serial.print("\t");
    Serial.print(h, 1);
    Serial.print("\t\t");
    Serial.print(t, 1);
    if (dht.getStatusString() == "OK") {
      temp = t;
      humi = h;

      // C?p nh?t thông tin lên OLED
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print(F("T: "));
      display.print(dataForm(t, 4, 1));
      display.print((char)223); // Kich thuoc 
      display.print(F("C"));

      display.setCursor(64, 0);
      display.print(F("H: "));
      display.print(dataForm(h, 3, 0));
      display.print(F("%"));

      display.setCursor(0, 24); //vi tri X,Y
      display.print(F("Nghia-CQ.62.KS.KTVT"));

      display.display();
    }
    currentTime = millis();
  }

  // C?p nh?t lên Google Sheets sau m?i 10 giây
  if (millis() - currentTime2 > 10000) {
    upData();
    currentTime2 = millis();
  }

  // X? lư yêu c?u HTTP t? Web Server
  server.handleClient();
}

void upData() {
  Serial.print("connecting to ");
  Serial.println(host);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String Send_Data_URL = "sts=write";
  Send_Data_URL += "&temp=" + String(temp);
  Send_Data_URL += "&humi=" + String(humi);

  String url = "/macros/s/" + GAS_ID + "/exec?" + Send_Data_URL;
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println("OK");
}

// Hàm d format d? li?u hien thi
String dataForm(float value, int leng, int decimal) {
  String str = String(value, decimal);
  if (str.length() < leng) {
    int space = leng - str.length();
    for (int i = 0; i < space; ++i) {
      str = " " + str;
    }
  }
  return str;
}

// Hàm xu lư yêu cau HTTP ( trang web)
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>IOT-62</title></head><body style='color:blue; text-align:center;'>";
  html += "<h1>CAM BIEN NHIET DO VS DO AM ESP8266 CH340</h1>";
  html += "<h1>DINH VAN NGHIA</h1>";
  html += "<h1>MSSV:6251020069</h1>";
   html += "<h1>CQ.62.KS.KTVT</h1>"; 
  html += "<p>Nhiet do: " + String(temp) + " &deg;C</p>";
  html += "<p>Do Am : " + String(humi) + " %</p>";
  html += "</body></html>";
  server.send(200, "text/html", html); // GUi phan hoi HTML
}
