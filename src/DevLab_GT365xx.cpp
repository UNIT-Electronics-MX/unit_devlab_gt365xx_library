/**
 * @file DevLab_GT365xx.cpp
 * @brief DevLab GT365xx light sensor (LDR) driver over DDP/I2C.
 *
 * @author Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#include "DevLab_GT365xx.h"

bool DevLab_GT365xx::begin() {
    _verified = false;
    _bus.setClock(_clock);
    _busReady = _bus.begin();
    if (!_busReady) return false;
    return (_verified = _ddp.matchesExpectedDevice(_address, &_info));
}

bool DevLab_GT365xx::begin(uint8_t sdaPin, uint8_t sclPin, uint32_t clock) {
    _verified = false;
    _clock = clock;
    _bus.setClock(_clock);
    _busReady = _bus.begin(sdaPin, sclPin);
    if (!_busReady) return false;
    return (_verified = _ddp.matchesExpectedDevice(_address, &_info));
}

bool DevLab_GT365xx::beginRecovered(uint8_t sdaPin, uint8_t sclPin, uint32_t timeoutUs, bool restart) {
    _verified = false;
    _bus.setClock(_clock);
    _busReady = _bus.beginRecovered(sdaPin, sclPin, timeoutUs, restart);
    if (!_busReady) return false;
    return (_verified = _ddp.matchesExpectedDevice(_address, &_info));
}

void DevLab_GT365xx::printInfo(Print &out) const {
    DevLabDDP::printDeviceInfo(out, _address, _info, _ddp.expectedDeviceId());
}

bool DevLab_GT365xx::readRaw(uint16_t &rawValue) {
    if (!_verified) return false;

    uint8_t bytes[2];
    if (!_ddp.readCommand(_address, CMD_READ_ADC0, bytes, 2U,_kAdcResponseDelayMs)) {
        return false;
    }
    rawValue = (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
    return true;
}

bool DevLab_GT365xx::readPercentage(float &percentage) {
  uint16_t raw;
  if (!readRaw(raw)) return false;
  percentage = (raw * 100.0f) / 4095.0f;
  return true;
}

bool DevLab_GT365xx::readInvertRaw(uint16_t &invertedValue) {
    uint16_t raw;
    if (!readRaw(raw)) return false;
    raw = (~raw) & 0x0FFFU;
    invertedValue = raw;
    return true;
}

bool DevLab_GT365xx::readInvertPercentage(float &percentage) {
    uint16_t inverted;
    if (!readInvertRaw(inverted)) return false;
    percentage = (inverted * 100.0f) / 4095.0f;
    return true;
}

