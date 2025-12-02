#pragma once
#include "core_types.h"
#include <pins.h>

class HWAbstraction {
public:

    // ======================== Types ========================
    struct spi_config_t {
        uint32_t spi_clock_hz;  // SPI clock frequency
        uint8_t  bit_order;     // 0 = MSB first, 1 = LSB first (if you want)
        uint8_t  mode;          // SPI mode 0..3
        uint8_t  read_dummy;    // dummy byte for reads
    };

    enum hw_pin_value_t { HW_PIN_LOW = 0, HW_PIN_HIGH = 1 };
    enum hw_status_t { HW_OK = 0, HW_ERROR = -1, HW_INVALID_PIN = -2, HW_INVALID_ARG = -3, HW_TIMEOUT = -4, HW_NOT_INIT = -5};

    // ======================== Virtual functions ========================

    // ----- GPIO Functions -----
    // Each operation is just: index → cached pointer/bitmask → register write/read.
    // We pay the cost once at attach (hw_pin_attach), then our hot-path calls are minimal.

    virtual hw_status_t hw_pin_attach(const pin_t& pin) = 0;
    virtual hw_status_t hw_pin_mode(uint8_t pin, pin_mode_t mode) = 0;
    virtual hw_status_t hw_pin_write(uint8_t pin, hw_pin_value_t value) = 0;
    virtual hw_status_t hw_pin_read(uint8_t pin, hw_pin_value_t* value) = 0;
    virtual hw_status_t hw_pin_toggle(uint8_t pin) = 0;

    uint8_t pin_to_index(const pin_t& pin) const {return hw_pin_to_index(pin);}

    // ----- SPI Functions -----
    virtual hw_status_t hw_spi_init(const spi_config_t& cfg)= 0;
    virtual hw_status_t hw_spi_reset() = 0;
    virtual hw_status_t hw_spi_transfer(const uint8_t* tx_data, uint8_t* rx_data, uint16_t len) = 0;
    virtual hw_status_t hw_spi_write(const uint8_t* data, uint16_t len) = 0;
    virtual hw_status_t hw_spi_read(uint8_t* data, uint16_t len) = 0;

    // ----- Delays -----
    virtual void hw_delay_us(uint32_t us) = 0;
    virtual void hw_delay_ms(uint32_t ms) = 0;


    // ----- Serial / logging ----- 
    virtual void hw_serial_begin(uint32_t) {} 
    virtual void hw_serial_write(const char* ) {} 
    virtual void hw_serial_writeln(const char* s) { hw_serial_write(s); hw_serial_write("\r\n"); }

    // ======================== Destructor ========================
    virtual ~HWAbstraction() = default;

protected:
    // ---- GPIO hooks ----
    virtual hw_status_t hw_pin_validate(const pin_t& pin)= 0;  // Used internally by hw_pin_attach
    virtual uint8_t     hw_pin_to_index(const pin_t& pin) const = 0;
    virtual void*       hw_port_base_from_index(uint8_t port_ix) const = 0;

    // ---- SPI hooks ----
    virtual hw_status_t hw_spi_validate_config(const spi_config_t& cfg) = 0; // Used internally by hw_spi_init
    virtual hw_status_t hw_spi_config() = 0;
    virtual uint8_t hw_spi_mode() const = 0;
};







