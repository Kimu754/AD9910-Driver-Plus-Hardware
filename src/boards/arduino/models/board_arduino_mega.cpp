#include "boards/board_abstraction.h"
#include "boards/arduino/board_arduino.h"
#include "boards/arduino/models/board_arduino_mega.h" 
#include <pins.h>   
#include <avr/io.h>
#include <Arduino.h>

ArduinoMega::ArduinoMega() {}

// Definition of the static pin meta table declared in ArduinoDue
ArduinoBoard::hw_pin_meta_t ArduinoMega::s_gpio_pins[ArduinoMega::HW_MAX_PINS]; 

// =============== Private helpers ===============
ArduinoBoard::hw_pin_meta_t* ArduinoMega::hw_pin_get_meta(uint8_t pin){
    if (pin >= HW_MAX_PINS) {return nullptr;}
    return &s_gpio_pins[pin];}


// =============== GPIO - ArduinoMegaBoard ===============
// ---- Pin and PORT LookUp ----
void* ArduinoMega::hw_port_base_from_index(uint8_t port_ix) const {
    switch (port_ix) {
        case PA: return (void*)&PORTA;
        case PB: return (void*)&PORTB;
        case PC: return (void*)&PORTC;
        case PD: return (void*)&PORTD;
        case PE: return (void*)&PORTE;
        case PF: return (void*)&PORTF;
        case PG: return (void*)&PORTG;
        case PH: return (void*)&PORTH;
        case PJ: return (void*)&PORTJ;
        case PK: return (void*)&PORTK;
        case PL: return (void*)&PORTL;
        default:
            return nullptr;
    }
}

constexpr uint8_t map_port_id_to_arduino(port_id_t p) {
    switch (p) {
        case PORT_A: return PA;
        case PORT_B: return PB;
        case PORT_C: return PC;
        case PORT_D: return PD;
        case PORT_E: return PE;
        case PORT_F: return PF;
        case PORT_G: return PG;
        case PORT_H: return PH;
        case PORT_J: return PJ;
        case PORT_K: return PK;
        case PORT_L: return PL;
        default:     return 0xFF;
    }
}


// (port , bit) -> Pin Number on Board (Board Specific)
uint8_t ArduinoMega::hw_pin_to_index(const pin_t& pin) const 
{
    const uint8_t arduino_port = map_port_id_to_arduino(pin.port);
    if (arduino_port == 0xFF)
        return PIN_U8_UNKNOWN;

    const uint8_t target_mask = uint8_t(1u << pin.pin);

    for (uint8_t arduino_pin = 0; arduino_pin < HW_MAX_PINS; ++arduino_pin) {
        if (digitalPinToPort(arduino_pin) == arduino_port &&
            digitalPinToBitMask(arduino_pin) == target_mask)
        {
            return arduino_pin;  // FOUND
        }
    }

    return PIN_U8_UNKNOWN;
}


// ---- Validate Pin ----
HWAbstraction::hw_status_t ArduinoMega::hw_pin_validate(const pin_t& pin) {
    if (!pin_has_port_pin(pin)) {return HW_INVALID_PIN;}

    if (pin.pin >= 8u) {return HW_INVALID_PIN;}

    const uint8_t idx = hw_pin_to_index(pin);
    if (idx == PIN_U8_UNKNOWN) {return HW_INVALID_PIN;}
    return HW_OK;
}

// ---- Attach Pin ----
HWAbstraction::hw_status_t ArduinoMega::hw_pin_attach(const pin_t& pin){

    // 1. Validate pin description
    const hw_status_t st = hw_pin_validate(pin);
    if (st != HW_OK) {return st;}

    // 2. Map to board index
    const uint8_t idx = hw_pin_to_index(pin);
    if (idx == PIN_U8_UNKNOWN || idx >= HW_MAX_PINS)
        return HW_INVALID_PIN;

    // 3. Get Meta
    hw_pin_meta_t* meta = hw_pin_get_meta(idx);
    if (!meta)                      //  <-- REQUIRED FIX
            return HW_ERROR;            // or HW_INVALID_PIN; both acceptable

    // Use Arduino’s own port/mask directly:
    const uint8_t arduino_port = digitalPinToPort(idx);
    const uint8_t arduino_mask = digitalPinToBitMask(idx);

    // 4. Store the metadata 
    meta->port_ix   = arduino_port;       // logical port (A,B,C,...)
    meta->bit       = pin.pin;                              // bit number 0–7 in that port
    meta->bit_mask  = arduino_mask;
    meta->mode      = static_cast<uint8_t>(pin.mode);       // e.g. INPUT / OUTPUT / ALT
    meta->pull      = static_cast<uint8_t>(pin.pull);       // e.g. NONE / PULLUP
    meta->attached  = 1; 

    // Attach
    hw_pin_mode(idx, pin.mode);

    return HW_OK;
}


// =============== SPI - ArduinoMegaBoard ===============

// ---- Validate SPI Config ----
// ATmega2560 core clock = 16 MHz
// SPI can run up to F_CPU/2 ≈ 8 MHz
HWAbstraction::hw_status_t ArduinoMega:: hw_spi_validate_config(const spi_config_t& cfg){

    constexpr uint32_t kMaxSpiHz= 8000000u;         // Clock bounds for Mega 2560
    if (cfg.spi_clock_hz == 0 || cfg.spi_clock_hz > kMaxSpiHz) return HW_INVALID_ARG;

    // Mode 0..3
    if (cfg.mode > 3) return HW_INVALID_ARG;

    // bit_order: 0 = MSB first, 1 = LSB first
    if (cfg.bit_order > 1) return HW_INVALID_ARG;

    return HW_OK;}

