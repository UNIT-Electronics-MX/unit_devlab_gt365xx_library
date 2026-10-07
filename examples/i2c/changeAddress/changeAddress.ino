/**
 * @file changeAddress.ino
 * @brief Serial command tool to scan the I2C bus for DDP devices and
 *        reassign a GT36537 device I2C address (factory address 0x26).
 *
 * Open Serial at 115200 baud and enter, for example: scan / change 26 30
 * Compatible with ESP32, RP2040/RP2350, STM32 and AVR.
 *
 * @author Cesar Bautista, Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#include <Arduino.h>
#include <Wire.h>
#include <DevLabDDP.h>
#include <DevLab_I2C_Orchestrator.h>

#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_RP2350)
TwoWire &i2cBus = Wire1;
constexpr int SDA_PIN = 12;
constexpr int SCL_PIN = 13;
#elif defined(ARDUINO_ARCH_ESP32)
TwoWire &i2cBus = Wire;
constexpr int SDA_PIN = 6;
constexpr int SCL_PIN = 7;
#elif defined(ARDUINO_ARCH_STM32)
// STM32duino: default I2C pins of the selected board.
TwoWire &i2cBus = Wire;
constexpr int SDA_PIN = SDA;
constexpr int SCL_PIN = SCL;
#elif defined(ARDUINO_ARCH_AVR)
// AVR has fixed I2C pins; begin() ignores the pin numbers.
TwoWire &i2cBus = Wire;
constexpr int SDA_PIN = SDA;
constexpr int SCL_PIN = SCL;
#else
#error "Use an ESP32, RP2040, RP2350, STM32, or AVR master"
#endif

constexpr uint8_t FACTORY_ADDRESS = 0x26U;
DevLab_I2C_Orchestrator orchestrator(i2cBus);
DevLabDDP::Master sensor(orchestrator, DevLabDDP::DEVICE_GT36537);
String inputLine;

bool parseAddress(const String &text, uint8_t &address) {
  char *end = nullptr;
  const long value = strtol(text.c_str(), &end, 16);
  if (end == text.c_str() || *end != '\0' || value < 0x08L || value > 0x77L) {
    return false;
  }
  address = (uint8_t)value;
  return true;
}

void printAddress(uint8_t address) {
  Serial.print("0x");
  if (address < 0x10U) Serial.print('0');
  Serial.print(address, HEX);
}

void scanBus() {
  bool found = false;
  Serial.println("Address  Device");
  for (uint8_t address = 0x08U; address <= 0x77U; ++address) {
    if (!sensor.ping(address)) continue;
    found = true;
    printAddress(address);
    Serial.print("     ");
    DevLabDDP::DeviceInfo info;
    Serial.println(sensor.identify(address, info)
                       ? DevLabDDP::deviceName(info.deviceId)
                       : "non-DDP");
  }
  if (!found) Serial.println("--       none");
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  scan");
  Serial.println("  change <current_hex> <new_hex>");
  Serial.print("Example: change ");
  if (FACTORY_ADDRESS < 0x10U) Serial.print('0');
  Serial.print(FACTORY_ADDRESS, HEX);
  Serial.println(" 30");
}

void changeAddress(const String &oldText, const String &newText) {
  uint8_t oldAddress = 0U;
  uint8_t newAddress = 0U;
  if (!parseAddress(oldText, oldAddress) || !parseAddress(newText, newAddress) ||
      oldAddress == newAddress) {
    Serial.println("ERROR: addresses must be different hexadecimal values from 08 to 77");
    return;
  }
  if (!sensor.matchesExpectedDevice(oldAddress)) {
    Serial.println("ERROR: current address does not contain a GT36537 DDP device");
    return;
  }
  if (sensor.ping(newAddress)) {
    Serial.println("ERROR: new address is already in use");
    return;
  }
  if (!sensor.setI2cAddress(oldAddress, newAddress)) {
    Serial.println("ERROR: address change failed");
    return;
  }
  Serial.print("OK: device is now at ");
  printAddress(newAddress);
  Serial.println();
  scanBus();
}

void processCommand(String line) {
  line.trim();
  line.toLowerCase();
  if (line == "scan") {
    scanBus();
    return;
  }
  const int firstSpace = line.indexOf(' ');
  const int secondSpace = firstSpace < 0 ? -1 : line.indexOf(' ', firstSpace + 1);
  if (firstSpace < 0 || secondSpace < 0 ||
      line.substring(0, firstSpace) != "change") {
    Serial.println("ERROR: invalid command");
    printHelp();
    return;
  }
  String oldText = line.substring(firstSpace + 1, secondSpace);
  String newText = line.substring(secondSpace + 1);
  oldText.trim();
  newText.trim();
  changeAddress(oldText, newText);
}

void setup() {
  Serial.begin(115200);
  delay(500U);
  i2cBus.begin(SDA_PIN, SCL_PIN);
  i2cBus.setClock(400000U);
  printHelp();
  scanBus();
}

void loop() {
  while (Serial.available()) {
    const char character = (char)Serial.read();
    if (character == '\r' || character == '\n') {
      if (inputLine.length() > 0U) {
        processCommand(inputLine);
        inputLine = "";
      }
    } else if (inputLine.length() < 64U) {
      inputLine += character;
    } else {
      inputLine = "";
      Serial.println("ERROR: command is too long");
    }
  }
}
