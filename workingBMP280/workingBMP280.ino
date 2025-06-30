#include <Wire.h>
#include <Adafruit_BMP280.h> // Use BMP280 library, not BME280

// Define I2C pins (for ESP32)
#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_BMP280 bmp;  // I2C interface

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Start I2C on specific pins
  Wire.begin(SDA_PIN, SCL_PIN);
  
  Serial.println("🔍 Searching for BMP280...");

  // Try I2C address 0x76
  if (!bmp.begin(0x76)) {
    Serial.println("❌ BMP280 not found at 0x76. Trying 0x77...");
    if (!bmp.begin(0x77)) {
      Serial.println("❌ BMP280 not found at 0x77 either. Check wiring and pull-up resistors.");
      while (1);  // Stop execution
    }
  }

  Serial.println("✅ BMP280 detected!");
}

void loop() {
  Serial.println("📡 BMP280 Readings:");

  Serial.print("🌡️ Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" °C");

  Serial.print("🌬️ Pressure = ");
  Serial.print(bmp.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.println("-----------------------------");
  delay(2000);
}
