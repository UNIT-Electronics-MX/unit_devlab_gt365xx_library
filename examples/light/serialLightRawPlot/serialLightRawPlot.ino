/**
 * @file serialLightRawPlot.ino
 * @brief Streams the raw GT36537 ADC0 count (0-4095), one integer per line,
 *        at 115200 baud for python/serial_plotter.py or the Arduino Serial
 *        Plotter.
 *
 * The GT36537 is an LDR read through PA2 (ADC_IN2, 12 bits). The value is a
 * relative light measurement (0-4095), not lux: the direction of the reading
 * depends on which end of the divider the LDR is connected to.
 *
 * @author Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#include <Arduino.h>
#include <Wire.h>
#include <DevLab_GT365xx.h>

#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_RP2350)
  #define I2C_BUS Wire1
  constexpr int SDA_PIN = 12;
  constexpr int SCL_PIN = 13;
#elif defined(ARDUINO_ARCH_ESP32)
  #define I2C_BUS Wire
  constexpr int SDA_PIN = 6;
  constexpr int SCL_PIN = 7;
#elif defined(ARDUINO_ARCH_STM32)
  // STM32duino: default I2C pins of the selected board.
  #define I2C_BUS Wire
  constexpr int SDA_PIN = SDA;
  constexpr int SCL_PIN = SCL;
#elif defined(ARDUINO_ARCH_AVR)
  // AVR has fixed I2C pins (Uno/Nano: A4/A5, Mega: 20/21, Leonardo: 2/3);
  // SDA/SCL come from the board variant and begin() ignores the pin numbers.
  #define I2C_BUS Wire
  constexpr int SDA_PIN = SDA;
  constexpr int SCL_PIN = SCL;
#else
#error "Use an ESP32, RP2040, RP2350, STM32, or AVR master"
#endif

constexpr uint32_t I2C_FREQ = 400000UL;
constexpr uint8_t SENSOR_ADDRESS = 0x26U;
constexpr uint32_t READ_INTERVAL_MS = 20U;

DevLab_GT365xx sensor(I2C_BUS, SENSOR_ADDRESS, I2C_FREQ);

void setup() {
  Serial.begin(115200);

  sensor.beginRecovered(SDA_PIN, SCL_PIN);
  if (!sensor.busReady()) {
    Serial.println("ERROR: I2C bus is blocked");
    return;
  }

  if (!sensor.isConnected()) {
    Serial.println("ERROR: expected GT36537 DDP device ID 0x0106 at 0x26");
  }
}

// Output is intentionally one decimal integer per line so it can be consumed
// directly by python/serial_plotter.py without choosing a field.
void loop() {
  static uint32_t lastRead = 0U;
  if (millis() - lastRead < READ_INTERVAL_MS) {
    return;
  }
  lastRead = millis();

  uint16_t raw = 0U;
  if (sensor.readRaw(raw)) {
    Serial.println(raw);
  } else {
    Serial.println("ERROR: light read failed");
  }
}
