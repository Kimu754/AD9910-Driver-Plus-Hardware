#pragma once
#include "boards/arduino/board_arduino.h"

class ArduinoDue : public ArduinoBoard {
public:
    ArduinoDue();
    ~ArduinoDue() override = default;

    // ----- GPIO -----
    hw_status_t hw_pin_attach(const pin_t& pin) override;

protected:
    // ---- GPIO hooks ----
    hw_status_t hw_pin_validate(const pin_t& pin) override;
    uint8_t     hw_pin_to_index(const pin_t& pin) const override;    // map pin_t → uint8_t
    void*       hw_port_base_from_index(uint8_t port_ix) const override;

    // ---- SPI hooks ----
    hw_status_t hw_spi_validate_config(const spi_config_t& cfg) override;

private:
    static constexpr uint8_t HW_MAX_PINS = 66;
    static hw_pin_meta_t s_gpio_pins[HW_MAX_PINS]; // declaration
    hw_pin_meta_t* hw_pin_get_meta(uint8_t pin);       
};

