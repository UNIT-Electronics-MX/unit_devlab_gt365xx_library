/**
 * @file DevLab_GT365xx.h
 * @brief DevLab GT365xx light sensor (LDR) driver over DDP/I2C.
 *
 * @author Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#ifndef DEVLAB_GT365xx_H
#define DEVLAB_GT365xx_H

#pragma once

#include "DevLabDDP.h"
#include "DevLabDDPConsole.h"
#include "DevLab_I2C_Orchestrator.h"


class DevLab_GT365xx 
{
public:
    explicit DevLab_GT365xx(TwoWire &wire = Wire, uint8_t address = 0x26U, uint32_t clock = 400000UL)
    : _bus(wire, clock), _ddp(_bus, DevLabDDP::DEVICE_GT36537), _address(address), _clock(clock) {}

    bool begin();
    bool begin(uint8_t sdaPin, uint8_t sclPin, uint32_t clock = 400000UL);
    bool beginRecovered(uint8_t sdaPin, uint8_t sclPin, uint32_t timeoutUs = 20000, bool restart = false);
    bool readRaw(uint16_t &rawValue);
    bool readPercentage(float &percentage);
    bool readInvertRaw(uint16_t &invertedValue);
    bool readInvertPercentage(float &percentage);
    bool busReady() const { return _busReady; }
    bool isConnected() const { return _verified; }
    const DevLabDDP::DeviceInfo &deviceInfo() const { return _info; }
    void printInfo(Print &out = Serial) const;

    DevLabDDP::Master &protocol() { return _ddp; }
    DevLab_I2C_Orchestrator &bus() { return _bus; }

    private:
    DevLab_I2C_Orchestrator _bus;
    DevLabDDP::Master _ddp;
    static constexpr uint16_t _kAdcResponseDelayMs = 2U;
    uint8_t _address;
    uint32_t _clock;
    bool _busReady = false;
    bool _verified = false;
    DevLabDDP::DeviceInfo _info;
};


#endif
