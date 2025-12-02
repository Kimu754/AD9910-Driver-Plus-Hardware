// Main Goal: 
// downstream code must check valid_mask first before touching other pin information.

#pragma once  
#include <cstdint>  

#define PIN_U8_UNKNOWN 0xFF  // Marker for unknown 8-bit values

// Pin status: enabled or disabled
typedef enum : uint8_t {
    PIN_DISABLED = 0,  // Pin is inactive
    PIN_ENABLED        // Pin is active
} pin_status_t;

// Pin direction or function mode
typedef enum : uint8_t {
    PIN_INPUT = 0,  // Configured as input
    PIN_OUTPUT,     // Configured as output
    PIN_ALT         // Configured for alternate function (peripheral)
} pin_mode_t;

// Internal pull resistor configuration
typedef enum : uint8_t {
    PIN_NOPULL = 0,  // No internal pull resistor
    PIN_PULLUP,      // Pull-up resistor enabled
    PIN_PULLDOWN     // Pull-down resistor enabled
} pin_pull_t;

// Drive strength configuration
typedef enum : uint8_t {
    PIN_LOW = 0,     // Low drive strength
    PIN_MEDIUM,      // Medium drive strength
    PIN_HIGH         // High drive strength
} pin_drive_t;

// Hardware port identifiers
typedef enum : uint8_t {
    PORT_A = 0,     // Port A
    PORT_B,         // Port B
    PORT_C,         // Port C
    PORT_D,         // Port D
    PORT_E,         // Port E
    PORT_F,         // Port F
    PORT_G,         // Port G
    PORT_H,         // Port H
    PORT_I,         // Port I
    PORT_J,         // Port J
    PORT_K,         // Port K
    PORT_L,         // Port L
    PORT_UNKNOWN    // Unknown or invalid port
} port_id_t;

// Functional role identifiers for a pin
enum {
    PIN_FUNC_NONE = 0,  // No specific function
    PIN_FUNC_CLK,       // Clock line
    PIN_FUNC_DATA,      // Data line
    PIN_FUNC_RESET,     // Reset control
    PIN_FUNC_SYNC,      // Synchronization line
    PIN_FUNC_CS         // Chip select
};

// Peripheral type identifiers
enum {
    PIN_PERIPH_NONE = 0,  // No peripheral
    PIN_PERIPH_SPI,       // SPI peripheral
    PIN_PERIPH_I2C,       // I2C peripheral
    PIN_PERIPH_UART,      // UART peripheral
    PIN_PERIPH_GPIO,      // GPIO (general-purpose)
    PIN_PERIPH_DDS        // DDS (direct digital synthesis)
};

// Bitmask flags that specify which fields are valid in a pin_t
enum {
    PINV_PORT   = 1 << 0,  // Port field is valid
    PINV_PIN    = 1 << 1,  // Pin number field is valid
    PINV_MODE   = 1 << 2,  // Mode field is valid
    PINV_PULL   = 1 << 3,  // Pull configuration is valid
    PINV_DRIVE  = 1 << 4,  // Drive strength field is valid
    PINV_FUNC   = 1 << 5,  // Function ID is valid
    PINV_PERIPH = 1 << 6   // Peripheral ID is valid
};

// Main structure describing a hardware pin configuration
typedef struct {
    uint8_t valid_mask;   // Bitmask indicating which fields are valid inside this pin_t 
    port_id_t port;       // Physical port identifier
    uint8_t pin;          // Pin number within the port
    pin_mode_t mode;      // Input/output/alternate mode
    pin_pull_t pull;      // Pull-up/down configuration
    pin_drive_t drive;    // Drive strength level
    uint8_t func_id;      // Functional role (PIN_FUNC_*)
    uint8_t periph_id;    // Associated peripheral (PIN_PERIPH_*)
} pin_t;

static_assert(sizeof(pin_t) == 8, "pin_t must stay 8 bytes");


// Core builder
constexpr pin_t make_pin(   port_id_t port, 
                            uint8_t pin,
                            pin_mode_t mode,
                            pin_pull_t pull     = PIN_NOPULL,
                            pin_drive_t drive   = PIN_MEDIUM,
                            uint8_t func_id     = PIN_FUNC_NONE,
                            uint8_t periph_id   = PIN_PERIPH_NONE ) {
    return pin_t{   PINV_PORT | PINV_PIN | PINV_MODE | PINV_PULL | PINV_DRIVE | PINV_FUNC | PINV_PERIPH , 
                    port, 
                    pin, 
                    mode, 
                    pull, 
                    drive, 
                    func_id, 
                    periph_id};
}

// GPIO
constexpr pin_t PIN_GPIO_OUT(port_id_t port, uint8_t pin,
                             uint8_t periph = PIN_PERIPH_GPIO,
                             pin_pull_t pull = PIN_NOPULL,
                             pin_drive_t drive = PIN_MEDIUM) {
    return make_pin(port, pin, PIN_OUTPUT, pull, drive, PIN_FUNC_NONE, periph);
}

constexpr pin_t PIN_GPIO_IN(port_id_t port, uint8_t pin,
                            uint8_t periph = PIN_PERIPH_GPIO,
                            pin_pull_t pull = PIN_NOPULL,
                            pin_drive_t drive = PIN_LOW) {
    return make_pin(port, pin, PIN_INPUT, pull, drive, PIN_FUNC_NONE, periph);                
}

constexpr pin_t PIN_SPI_ALT(port_id_t port, uint8_t pin, uint8_t func /*CLK/DATA/CS*/) {
    return make_pin(port, pin, PIN_ALT, PIN_NOPULL, PIN_MEDIUM, func, PIN_PERIPH_SPI);
}

// Unknowns
constexpr pin_t PIN_UNKNOWN_OUT() {
    return pin_t{
        PINV_MODE | PINV_PULL | PINV_DRIVE | PINV_PERIPH,
        PORT_UNKNOWN,
        PIN_U8_UNKNOWN,
        PIN_OUTPUT,
        PIN_NOPULL,
        PIN_MEDIUM,
        PIN_FUNC_NONE,
        PIN_PERIPH_GPIO
    };
}

constexpr pin_t PIN_UNKNOWN_IN(uint8_t func = PIN_FUNC_NONE) {
    return pin_t{
        PINV_MODE | PINV_PULL | PINV_DRIVE | PINV_FUNC | PINV_PERIPH,
        PORT_UNKNOWN,
        PIN_U8_UNKNOWN,
        PIN_INPUT,
        PIN_NOPULL,
        PIN_LOW,
        func,
        PIN_PERIPH_GPIO
    };
}

// CHECK PINS
inline constexpr bool pin_has_port_pin(const pin_t& p) {
    return (p.valid_mask & (PINV_PORT | PINV_PIN)) == (PINV_PORT | PINV_PIN);
}

inline constexpr bool pin_has_func(const pin_t& p) {
    return (p.valid_mask & PINV_FUNC) != 0;
}