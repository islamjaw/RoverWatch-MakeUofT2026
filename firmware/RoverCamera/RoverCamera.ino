#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// WiFi credentials
const char* ssid = "Jawwad";
const char* password = "password";

// Flask server (UPDATE THIS TO YOUR LAPTOP'S IP ON HOTSPOT)
const char* flaskServer = "http://192.168.123.78:5000/classify";

// Sensor pins
#define DHTPIN 13
#define DHTTYPE DHT11
#define MQ2_PIN 12

DHT dht(DHTPIN, DHTTYPE);

// Mock Coordinates
float current_lat = 43.6628;
float current_lon = -79.3958;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== SurvivalSense ESP32-CAM ===");
  
  // Initialize sensors
  dht.begin();
  pinMode(MQ2_PIN, INPUT);
  
  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; 
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Camera init failed: 0x%x\n", err);
    return;
  }
  Serial.println("✅ Camera initialized!");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");
}

void loop() {
  static unsigned long lastCapture = 0;
  
  if (millis() - lastCapture >= 10000) {
    lastCapture = millis();
    
    // 1. Read REAL sensors (Added Humidity)
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity(); // <--- NEW
    int airQuality = analogRead(MQ2_PIN);
    
    // Check if reading failed
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("❌ Failed to read from DHT sensor!");
      return;
    }

    Serial.println("\n========================================");
    Serial.printf("🌡️ Temp: %.1f°C | 💧 Hum: %.1f%% | 🌫️ Air: %d\n", temperature, humidity, airQuality);
    
    // 2. Capture and send image with Humidity
    sendImageToFlask(temperature, humidity, airQuality);
    
    Serial.println("========================================\n");
  }
  delay(1000);
}

// Updated signature to include humidity
void sendImageToFlask(float temp, float hum, int gas) {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) return;

  HTTPClient http;
  http.begin(flaskServer);
  http.setTimeout(15000); 
  
  String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
  String contentType = "multipart/form-data; boundary=" + boundary;
  
  // 3. Build multipart body including Humidity
  String head = "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"lat\"\r\n\r\n" + String(current_lat, 4) + "\r\n";
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"lon\"\r\n\r\n" + String(current_lon, 4) + "\r\n";
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"temp\"\r\n\r\n" + String(temp, 1) + "\r\n";
  head += "--" + boundary + "\r\n";
  // --- NEW HUMIDITY BLOCK ---
  head += "Content-Disposition: form-data; name=\"humidity\"\r\n\r\n" + String(hum, 1) + "\r\n";
  head += "--" + boundary + "\r\n";
  // --------------------------
  head += "Content-Disposition: form-data; name=\"gas\"\r\n\r\n" + String(gas) + "\r\n";
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"image\"; filename=\"capture.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";
  
  String tail = "\r\n--" + boundary + "--\r\n";
  uint32_t totalLen = head.length() + fb->len + tail.length();
  
  uint8_t *postBody = (uint8_t*)malloc(totalLen);
  if (postBody) {
    memcpy(postBody, head.c_str(), head.length());
    memcpy(postBody + head.length(), fb->buf, fb->len);
    memcpy(postBody + head.length() + fb->len, tail.c_str(), tail.length());
    
    http.addHeader("Content-Type", contentType);
    int httpResponseCode = http.POST(postBody, totalLen);
    
    if (httpResponseCode == 200) {
      Serial.println("🤖 AI COMMANDER UPDATED: " + http.getString());
    } else {
      Serial.printf("❌ HTTP Error: %d\n", httpResponseCode);
    }
    free(postBody);
  }
  
  http.end();
  esp_camera_fb_return(fb);
}