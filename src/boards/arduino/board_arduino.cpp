#include "boards/arduino/board_arduino.h"
#include <Arduino.h>
#include <SPI.h>


// ----- GPIO Functions -----
HWAbstraction::hw_status_t ArduinoBoard::hw_pin_mode(uint8_t pin, pin_mode_t mode){
    switch (mode) {
        case PIN_INPUT:  ::pinMode(pin, INPUT);  break;
        case PIN_OUTPUT: ::pinMode(pin, OUTPUT); break;
        case PIN_ALT:
        default:
            return HW_INVALID_ARG;
    }
    return HW_OK;
}

HWAbstraction::hw_status_t ArduinoBoard::hw_pin_write(uint8_t pin, hw_pin_value_t value) {
    switch (value) {
        case HW_PIN_LOW:  ::digitalWrite(pin, LOW);  break;
        case HW_PIN_HIGH: ::digitalWrite(pin, HIGH); break;
        default: return HW_INVALID_ARG;
    }
    return HW_OK;}

HWAbstraction::hw_status_t ArduinoBoard::hw_pin_read(uint8_t pin, hw_pin_value_t *value) {
    if (!value) return HW_INVALID_ARG;
    int v = ::digitalRead(pin);
    *value = (v == HIGH) ? HW_PIN_HIGH : HW_PIN_LOW;
    return HW_OK;}

HWAbstraction::hw_status_t ArduinoBoard::hw_pin_toggle(uint8_t pin) {
    int v = ::digitalRead(pin);
    ::digitalWrite(pin, (v == HIGH) ? LOW : HIGH);
    return HW_OK;}


// --- SPI ---
uint8_t ArduinoBoard::hw_spi_mode() const {
    switch (cfg_.mode)
    {
        case 0: return SPI_MODE0;
        case 1: return SPI_MODE1;
        case 2: return SPI_MODE2;
        case 3: return SPI_MODE3;
        default: return SPI_MODE0;  // safe fallback
    }
}

// Used internally by hw_spi_init
HWAbstraction::hw_status_t ArduinoBoard::hw_spi_config(){
    SPI.begin();
    SPI.beginTransaction(SPISettings(   cfg_.spi_clock_hz,
                                        cfg_.bit_order == 0 ? MSBFIRST : LSBFIRST,
                                        hw_spi_mode()   )
                                    );
    return HW_OK;}

HWAbstraction::hw_status_t ArduinoBoard::hw_spi_init(const spi_config_t& cfg) {
    if (initialized_) return HW_OK;

    auto st = hw_spi_validate_config(cfg);  // 1) validate is to be implemented in SubClasses
    if (st != HW_OK) return st;

    cfg_ = cfg;                             // 2) store config

    st = hw_spi_config();                   // 3) apply to SPI
    if (st != HW_OK) return st;
    initialized_ = true;
    return HW_OK;
}


HWAbstraction::hw_status_t ArduinoBoard::hw_spi_reset() {
#if defined(SPI_HAS_TRANSACTION)
    SPI.end();
    SPI.begin();
    return HW_OK;
#else
    return HW_ERROR;
#endif
}

HWAbstraction::hw_status_t ArduinoBoard::hw_spi_transfer(const uint8_t* tx_data, uint8_t* rx_data, uint16_t len) {
    if (!initialized_) return HW_NOT_INIT;
    if (!tx_data || len == 0) return HW_INVALID_ARG;

    SPI.beginTransaction(SPISettings(
        cfg_.spi_clock_hz,
        cfg_.bit_order == 0 ? MSBFIRST : LSBFIRST,
        hw_spi_mode()   ));
    for (uint16_t i = 0; i < len; ++i) {
        uint8_t r = SPI.transfer(tx_data[i]);
        if (rx_data) rx_data[i] = r;
    }
    SPI.endTransaction();
    return HW_OK;}

HWAbstraction::hw_status_t ArduinoBoard::hw_spi_write(const uint8_t* data, uint16_t len) {
    if (!initialized_) return HW_NOT_INIT;
    return hw_spi_transfer(data, nullptr, len);}

HWAbstraction::hw_status_t ArduinoBoard::hw_spi_read(uint8_t* data, uint16_t len) {
    if (!initialized_) return HW_NOT_INIT;
    if (!data || len == 0) return HW_INVALID_ARG;
    SPI.beginTransaction(SPISettings(   cfg_.spi_clock_hz,
                                        cfg_.bit_order == 0 ? MSBFIRST : LSBFIRST,
                                        hw_spi_mode()  ));
    for (uint16_t i = 0; i < len; ++i) data[i] = SPI.transfer(cfg_.read_dummy);
    SPI.endTransaction();
    return HW_OK;}

// --- Delay ---
void ArduinoBoard::hw_delay_us(uint32_t us) { ::delayMicroseconds(us); }
void ArduinoBoard::hw_delay_ms(uint32_t ms) { ::delay(ms); }

// --- Serial ---
void ArduinoBoard::hw_serial_begin(uint32_t baud) {Serial.begin(baud);}
void ArduinoBoard::hw_serial_write(const char* s) { if (!s) return; Serial.print(s);}
void ArduinoBoard::hw_serial_writeln(const char* s) { if (!s) return; Serial.println(s);}







