#include "boards/board_abstraction.h"
#include "boards/arduino/board_arduino.h"
#include "boards/arduino/models/board_arduino_due.h"    


// Definition of the static pin meta table declared in ArduinoDue
HWAbstraction::hw_pin_meta_t ArduinoDue::s_gpio_pins[ArduinoDue::HW_MAX_PINS]; 


// =============== Private helpers ===============

hw_pin_meta_t* ArduinoDue::hw_pin_get_meta(uint8_t pin){
    if (pin >= HW_MAX_PINS) {return nullptr;}
    return &s_gpio_pins[pin];}


// =============== GPIO ===============

// ---- ArduinoDueBoard:: Validate Pin ----
hw_status_t ArduinoDue::hw_pin_validate(const pin_t& pin){

    // Basic structural checks first
    if (!pin_has_port_pin(pin)) { return HW_INVALID_PIN;}

    // Optional:  enforce that a function is set (e.g. GPIO vs SPI alt, etc.)
    // if (!pin_has_func(pin)) {return HW_INVALID_ARG;}

    // Map to board index
    const uint8_t idx = hw_pin_to_index(pin);
    if (idx == PIN_U8_UNKNOWN || idx >= HW_MAX_PINS) {return HW_INVALID_PIN;}

    return HW_OK;}



// ---- ArduinoDueBoard:: Pin LookUp ----

// TODO
uint8_t ArduinoDue::hw_pin_to_index(const pin_t& pin) const{
    // IMPLEMENTATION HERE
    // Return 0..HW_MAX_PINS-1 or PIN_U8_UNKNOWN
}

// TODO
void* ArduinoDue::hw_port_base_from_index(uint8_t port_ix) const{
    // IMPLEMENTATION HERE
    // Return PIOA/PIOB/... or nullptr
}


// ---- ArduinoDueBoard:: Attach Pin ----
hw_status_t ArduinoDue::hw_pin_attach(const pin_t& pin){

    // Validate pin description
    const hw_status_t st = hw_pin_validate(pin);
    if (st != HW_OK) {return st;}

    // Map to board index
    const uint8_t idx = hw_pin_to_index(pin);
    if (idx == PIN_U8_UNKNOWN || idx >= HW_MAX_PINS) {return HW_INVALID_PIN;}

    hw_pin_meta_t* meta = hw_pin_get_meta(idx);
    if (!meta) {return HW_INVALID_PIN;}

    // TODO 
    // Now store the metadata based on your pin_t layout and hw_pin_meta_t layout.
    // This is where YOUR struct definitions matter. I can’t fill the members for you
    // without seeing them, so I keep it generic.

    // PSEUDO-CODE — adapt to your real member names:
    //
    // meta->port_ix = static_cast<uint8_t>(pin.port);
    // meta->pin     = pin.pin;
    // meta->mode    = static_cast<uint8_t>(pin.mode);
    // meta->pull    = static_cast<uint8_t>(pin.pull);
    // meta->status  = PIN_ENABLED;   // if you track enabled/attached state
    //
    // Then you can also configure the MCU HW here if you want to do it at attach-time:
    //
    // Pio* pio = static_cast<Pio*>(hw_port_base_from_index(meta->port_ix));
    // if (!pio) {
    //     return HW_INVALID_PIN;
    // }
    //
    // // Enable clock for PIOx, configure PIO controller, etc.
    // // This is SAM3X/Arduino-core specific; follow the official Arduino Due core
    // // (digital_pin_to_port, g_APinDescription, etc.) to do it correctly.

    return HW_OK;
}


// =============== SPI ===============

// ---- ArduinoDueBoard:: Validate SPI Config ----
    // SAM3X8E core clock = 84 MHz
    // SPI can run up to F_CPU/2 ≈ 42 MHz
hw_status_t ArduinoDue:: hw_spi_validate_config(const spi_config_t& cfg){

    constexpr uint32_t kMaxSpiHz= 42000000u;         // Clock bounds
    if (cfg.clock_hz == 0 || cfg.clock_hz > kMaxSpiHz) return HW_INVALID_ARG;

    // Mode 0..3
    if (cfg.mode > 3) return HW_INVALID_ARG;

    // bit_order: 0 = MSB first, 1 = LSB first
    if (cfg.bit_order > 1) return HW_INVALID_ARG;

    return HW_OK;}

