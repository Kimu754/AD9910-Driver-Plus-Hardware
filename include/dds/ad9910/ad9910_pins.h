#pragma once 

#include "dds/dds_base.h"
#include <pins.h>
#include <array>
#include <cstddef>


// =================== Enumeration of AD9910 pins ===================
enum class DdsPin : uint8_t {
    SPI_SCLK, SPI_SDIO, SPI_SDO, SPI_CS,
    IO_UPDATE, IO_RESET, MASTER_RESET,
    PROFILE0, PROFILE1, PROFILE2,
    OSK, PDCLK, TX_ENABLE, F0, F1,
    DRHOLD, PWR_DWN, DRCTL, DROVER, SYNC_CLK, 
    RAM_SWP_OVR, PLL_LOCK, COUNT };

// =================== Helpers for the DdsClass  ===================
constexpr inline size_t idx(DdsPin id) { return static_cast<size_t>(id); }
constexpr size_t kPinCount = idx(DdsPin::COUNT);  // total pin count

// =================== Create the PinMap For Mega ===================
constexpr std::array<pin_t, kPinCount> make_dds_pins_mega() {
    std::array<pin_t, kPinCount> a{};

    // Manual Mapping Pin Number -> (Port, Bit) 
    // Sources:
        // - Arduino Mega 2560 Pins DataSheet
        // - Github Repo GRA & AFACH AD9910 V3 (see README) 

    // SPI
    a[idx(DdsPin::SPI_SCLK)] = make_pin(PORT_B, 7, PIN_OUTPUT );            // DDS_SPI_SCLK_PIN  13 (AVR 26)
    a[idx(DdsPin::SPI_SDIO)] = make_pin(PORT_B, 5, PIN_OUTPUT );            // DDS_SPI_SDIO_PIN  11 (AVR 24)
    a[idx(DdsPin::SPI_SDO)]  = make_pin(PORT_B, 6, PIN_INPUT  );            // DDS_SPI_SDO_PIN   12  (AVR 25)
    a[idx(DdsPin::SPI_CS)]   = make_pin(PORT_B, 4, PIN_OUTPUT );            // DDS_SPI_CS_PIN 10 (AVR 23)

    // Control
    a[idx(DdsPin::IO_UPDATE)]    = make_pin(PORT_H, 3, PIN_OUTPUT  );      // DDS_IO_UPDATE_PIN    6   (AVR 15)
    a[idx(DdsPin::IO_RESET)]     = make_pin(PORT_H, 6, PIN_OUTPUT  );      // DDS_IO_RESET_PIN     9   (AVR 18)
    a[idx(DdsPin::MASTER_RESET)] = make_pin(PORT_J, 1, PIN_OUTPUT  );      // DDS_MASTER_RESET_PIN 14  (AVR 64)

    // Profiles
    a[idx(DdsPin::PROFILE0)]    = make_pin(PORT_J, 0, PIN_OUTPUT);  // DDS_PROFILE_0_PIN    15 (AVR 63)
    a[idx(DdsPin::PROFILE1)]    = make_pin(PORT_H, 1, PIN_OUTPUT);  // DDS_PROFILE_1_PIN    16 (AVR 13)
    a[idx(DdsPin::PROFILE2)]    = make_pin(PORT_H, 0, PIN_OUTPUT);  // DDS_PROFILE_2_PIN    17 (AVR 12)

    // Other
    a[idx(DdsPin::OSK)]         = make_pin(PORT_G, 1, PIN_OUTPUT);  // DDS_OSK_PIN          40 (AVR 52)
    a[idx(DdsPin::PDCLK)]       = make_pin(PORT_L, 5, PIN_INPUT );  // DDS_PDCLK_PIN        44 (AVR 40)
    a[idx(DdsPin::TX_ENABLE)]   = make_pin(PORT_D, 7, PIN_OUTPUT);  // DDS_TxENABLE_PIN     38 (AVR 50)
    a[idx(DdsPin::F0)]          = make_pin(PORT_G, 2, PIN_OUTPUT);  // DDS_F0_PIN           39 (AVR 70)
    a[idx(DdsPin::F1)]          = make_pin(PORT_G, 0, PIN_OUTPUT);  // DDS_F1_PIN           41 (AVR 51)

    a[idx(DdsPin::DRHOLD)]      = make_pin(PORT_H, 5, PIN_OUTPUT);  // DDS_DRHOLD_PIN       8  (AVR 17)
    a[idx(DdsPin::PWR_DWN)]     = make_pin(PORT_E, 3, PIN_OUTPUT);  // DDS_PWR_DWN_PIN      5  (AVR5)
    a[idx(DdsPin::DRCTL)]       = make_pin(PORT_H, 4, PIN_OUTPUT);  // DDS_DRCTL_PIN        7  (AVR 16)
    a[idx(DdsPin::DROVER)]      = make_pin(PORT_L, 7, PIN_INPUT );  // DDS_DROVER           42 (AVR 42)
    a[idx(DdsPin::SYNC_CLK)]    = make_pin(PORT_L, 3, PIN_INPUT );  // DDS_SYNC_CLK         46 (AVR 38)   
    a[idx(DdsPin::RAM_SWP_OVR)] = make_pin(PORT_L, 2, PIN_INPUT) ;  // DDS_RAM_SWP_OVR      47 (AVR 37)
    a[idx(DdsPin::PLL_LOCK)]    = make_pin(PORT_L, 4, PIN_INPUT) ;  // DDS_PLL_LOCK         45 (AVR 39)

    return a;}


// =================== -------------------------- ===================
// =================== Create the PinMap For Mega ===================
// =================== -------------------------- ===================

constexpr const std::array<pin_t, kPinCount>& dds_pins() {
    static constexpr auto pins = make_dds_pins_mega();
    return pins;
}

// =================== -------------------------- ===================
// =================== Create the PinMap For Mega ===================
// =================== -------------------------- ===================

static_assert(dds_pins().size() == kPinCount, "DdsPin enum and table out of sync");
constexpr const pin_t& pin(DdsPin id) {
    return dds_pins()[idx(id)];}


