#include <SPI.h>
#include <LoRa.h>

// Pin definitions for RA-02 module on ESP32
#define SS_PIN    5     // NSS/CS pin (GPIO 5)
#define RST_PIN   14    // Reset pin (GPIO 14)
#define DIO0_PIN  2     // DIO0 pin for interrupt (GPIO 2)

// LoRa configuration
#define FREQUENCY 433E6  // 433 MHz
#define BANDWIDTH 125E3  // 125 kHz
#define SPREADING_FACTOR 7
#define CODING_RATE 5    // 4/5
#define SYNC_WORD 0x12
#define TX_POWER 17      // 17 dBm (max for SX1278)

bool moduleFound = false;
int packetCounter = 0;

void setup() {
  Serial.begin(115200);  // Higher baud rate for ESP32
  delay(1000);  // Give ESP32 time to initialize instead of while(!Serial)
  
  Serial.println("LoRa RA-02 (SX1278) Module Checker");
  Serial.println("===================================");
  
  // Initialize SPI for ESP32 (optional but recommended)
  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS
  
  // Initialize LoRa pins
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
  
  // Check if module is connected and responding
  Serial.print("Initializing LoRa module... ");
  
  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("FAILED!");
    Serial.println("Check connections:");
    Serial.println("VCC -> 3.3V");
    Serial.println("GND -> GND");
    Serial.println("MOSI -> GPIO 23");
    Serial.println("MISO -> GPIO 19");
    Serial.println("SCK -> GPIO 18");
    Serial.println("NSS -> GPIO 5");
    Serial.println("RST -> GPIO 14");
    Serial.println("DIO0 -> GPIO 2");
    while (1);
  }
  
  Serial.println("SUCCESS!");
  moduleFound = true;
  
  // Configure LoRa parameters
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE);
  LoRa.setSyncWord(SYNC_WORD);
  LoRa.setTxPower(TX_POWER);
  
  // Read and display module information
  displayModuleInfo();
  
  Serial.println("\nStarting module tests...");
  Serial.println("Note: Without antenna, range will be very limited (~1-2 meters)");
  Serial.println();
}

void loop() {
  if (!moduleFound) return;
  
  // Test 1: Module Communication Test
  testRegisterAccess();
  delay(2000);
  
  // Test 2: Transmit Test
  testTransmission();
  delay(3000);
  
  // Test 3: Receive Test (non-blocking)
  testReception();
  delay(2000);
  
  // Test 4: RSSI Test
  testRSSI();
  delay(5000);
}

void displayModuleInfo() {
  Serial.println("\n=== Module Information ===");
  Serial.print("Frequency: ");
  Serial.print(FREQUENCY / 1E6);
  Serial.println(" MHz");
  
  Serial.print("Bandwidth: ");
  Serial.print(BANDWIDTH / 1E3);
  Serial.println(" kHz");
  
  Serial.print("Spreading Factor: ");
  Serial.println(SPREADING_FACTOR);
  
  Serial.print("Coding Rate: 4/");
  Serial.println(CODING_RATE);
  
  Serial.print("Sync Word: 0x");
  Serial.println(SYNC_WORD, HEX);
  
  Serial.print("TX Power: ");
  Serial.print(TX_POWER);
  Serial.println(" dBm");
}

void testRegisterAccess() {
  Serial.println("=== Module Communication Test ===");
  
  // Test basic communication by trying to change settings
  Serial.print("Testing module communication... ");
  
  // Try to set a known configuration
  long originalFreq = 433E6;
  long testFreq = 434E6;
  
  // Set original frequency
  LoRa.setFrequency(originalFreq);
  
  // Try to change frequency and test with a transmission
  LoRa.setFrequency(testFreq);
  
  // Test if we can send a packet (this will fail if module not working)
  LoRa.beginPacket();
  LoRa.print("comm_test");
  int result = LoRa.endPacket();
  
  if (result == 1) {
    Serial.println("SUCCESS!");
    Serial.println("✓ Module responding to commands");
    
    // Reset to original frequency
    LoRa.setFrequency(originalFreq);
    Serial.println("✓ Frequency control working");
  } else {
    Serial.println("FAILED!");
    Serial.println("✗ Module not responding properly");
    return;
  }
  
  // Test other configuration changes
  Serial.print("Testing configuration changes... ");
  
  // Test spreading factor change
  int originalSF = SPREADING_FACTOR;
  LoRa.setSpreadingFactor(8);
  
  // Test transmission with new settings
  LoRa.beginPacket();
  LoRa.print("config_test");
  int configResult = LoRa.endPacket();
  
  if (configResult == 1) {
    LoRa.setSpreadingFactor(originalSF); // Reset
    Serial.println("SUCCESS!");
    Serial.println("✓ Configuration control working");
  } else {
    LoRa.setSpreadingFactor(originalSF); // Reset anyway
    Serial.println("WARNING!");
    Serial.println("⚠ Some configuration changes may not be working");
  }
}

void testTransmission() {
  Serial.println("\n=== Transmission Test ===");
  
  String message = "Test packet #" + String(packetCounter++);
  
  Serial.print("Sending: ");
  Serial.println(message);
  
  unsigned long startTime = millis();
  
  LoRa.beginPacket();
  LoRa.print(message);
  int result = LoRa.endPacket();
  
  unsigned long duration = millis() - startTime;
  
  if (result == 1) {
    Serial.println("✓ Packet sent successfully");
  } else {
    Serial.println("✗ Packet transmission failed");
  }
  
  Serial.print("Transmission time: ");
  Serial.print(duration);
  Serial.println(" ms");
}

void testReception() {
  Serial.println("\n=== Reception Test ===");
  
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    Serial.print("Received packet: '");
    
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    
    Serial.print(received);
    Serial.print("' with RSSI ");
    Serial.print(LoRa.packetRssi());
    Serial.print(" dBm, SNR ");
    Serial.print(LoRa.packetSnr());
    Serial.println(" dB");
    
    Serial.println("✓ Reception working");
  } else {
    Serial.println("No packets received (normal without second module)");
  }
}

void testRSSI() {
  Serial.println("\n=== RSSI Test ===");
  
  int rssi = LoRa.rssi();
  Serial.print("Current RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");
  
  if (rssi < -120) {
    Serial.println("✓ RSSI reading normal (no signal detected)");
  } else {
    Serial.println("📡 Signal detected - may indicate nearby LoRa activity");
  }
}

// Additional utility functions for advanced testing

void testModulationSettings() {
  Serial.println("\n=== Testing Different Settings ===");
  
  // Store original settings
  int originalSF = SPREADING_FACTOR;
  
  // Test different spreading factors
  for (int sf = 7; sf <= 12; sf++) {
    Serial.print("Testing SF");
    Serial.print(sf);
    Serial.print(": ");
    
    // Set spreading factor (always succeeds, but may not work if module broken)
    LoRa.setSpreadingFactor(sf);
    
    // Try a quick transmission test to verify it's working
    LoRa.beginPacket();
    LoRa.print("SF test");
    int result = LoRa.endPacket();
    
    if (result == 1) {
      Serial.println("✓");
    } else {
      Serial.println("✗ TX failed");
    }
    delay(100);
  }
  
  // Reset to original settings
  LoRa.setSpreadingFactor(originalSF);
  Serial.println("✓ Settings restored");
}

void enterSleepMode() {
  Serial.println("Entering sleep mode...");
  LoRa.sleep();
}

void wakeUp() {
  Serial.println("Waking up module...");
  LoRa.idle();
}