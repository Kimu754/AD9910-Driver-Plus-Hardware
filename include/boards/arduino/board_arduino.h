#pragma once
#include "boards/board_abstraction.h"

class ArduinoBoard : public HWAbstraction {
public:

    // ----- GPIO Functions -----
    hw_status_t hw_pin_attach(const pin_t& pin) override = 0; // Implemented in subclass (validate + map + store)
    hw_status_t hw_pin_mode(uint8_t pin, pin_mode_t mode) override;  // Set the electrical direction of a pin after it has been attached and validated.
    hw_status_t hw_pin_write(uint8_t pin,hw_pin_value_t value) override;
    hw_status_t hw_pin_read(uint8_t pin, hw_pin_value_t* value) override;
    hw_status_t hw_pin_toggle(uint8_t pin) override;

    // --- SPI ---
    hw_status_t hw_spi_init(const spi_config_t& cfg) override; // Calls Functions that must be implemented in subclasses
    hw_status_t hw_spi_reset() override;
    hw_status_t hw_spi_transfer(const uint8_t* tx_data, uint8_t* rx_data, uint16_t len) override;
    hw_status_t hw_spi_write(const uint8_t* data, uint16_t len) override;
    hw_status_t hw_spi_read(uint8_t* data, uint16_t len) override;

    // --- Delay ---
    void hw_delay_us(uint32_t us) override;
    void hw_delay_ms(uint32_t ms) override;

    // ----- Serial / logging -----
    void hw_serial_begin(uint32_t baud) override;
    void hw_serial_write(const char* s) override;
    void hw_serial_writeln(const char* s) override;


protected:
    ArduinoBoard() = default;
    ~ArduinoBoard() override = default;

    spi_config_t cfg_{};
    bool initialized_ = false; 
    
    // ---- GPIO ----

    // Low-level data needed at runtime for a pin index lookup table
    struct hw_pin_meta_t {
        bool     attached;
        uint8_t  port_ix;   // used to get port base address
        uint8_t  bit;          // 0..15 etc.
        uint32_t bit_mask;     // 1u << bit
        uint8_t  mode;
        uint8_t  pull;
    };

    hw_status_t hw_pin_validate(const pin_t& pin) override = 0; // Will be Used internally by hw_pin_attach
    uint8_t     hw_pin_to_index(const pin_t& pin) const override = 0;    // map pin_t → uint8_t
    void*       hw_port_base_from_index(uint8_t port_ix) const override = 0;

    // ---- SPI ----
    hw_status_t hw_spi_validate_config(const spi_config_t& cfg) override = 0; // Used internally by hw_spi_init
    hw_status_t hw_spi_config() override;
    uint8_t hw_spi_mode() const override;

};
