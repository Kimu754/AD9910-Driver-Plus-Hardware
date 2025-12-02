#pragma once

#include "core_types.h"
#include "boards/board_abstraction.h"
#include "pins.h"
#include "registers.h"


// ------------------------ Types ------------------------- 
enum class dds_status_t : int32_t {     DDS_OK = 0,
                                        DDS_ERROR = -1,
                                        DDS_HW_ERROR = -2,
                                        DDS_INVALID_PARAM = -3,
                                        DDS_TIMEOUT = -4,
                                        DDS_NOT_INITIALIZED = -5
                                    };

enum class SweepTimeFormat : uint8_t {  Seconds = 0,
                                        Milliseconds = 1,
                                        Microseconds = 2,
                                        Nanoseconds = 3
                                    };

// ------------------ DDS Base Class ------------------
class DDSBase {
public:

    DDSBase(HWAbstraction& hw,
                    const HWAbstraction::spi_config_t& spi_cfg,
                    const pin_t* pins,
                    size_t pins_count)
        : hw_(hw),
        spi_cfg_(spi_cfg),
        pins_(pins),
        pins_count_(pins_count),
        pin_indices_(pins_count, PIN_U8_UNKNOWN)
    {}

    virtual ~DDSBase() = default;
    DDSBase(const DDSBase&) = delete;
    DDSBase& operator=(const DDSBase&) = delete;
    DDSBase(DDSBase&&) = default;
    DDSBase& operator=(DDSBase&&) = default;

    // ------------ Base Struct For running sequences -------------
    struct DdsSequence {
        uint8_t pin_index;                      // Board pin number  
        HWAbstraction::hw_pin_value_t level;    // High / Low
        uint16_t delay_us;                      // delay after write in micro second
    };

    // ------------ To implement by subclasses ------------

    // --- DDS Register Protocol ---
    virtual dds_status_t dds_reg_write(uint8_t addr,
                                   const uint8_t* data,
                                   size_t len) = 0;

    virtual dds_status_t dds_reg_read(uint8_t addr,
                                    uint8_t* buf,
                                    size_t len) = 0;


    // --- DDS Functionalities ---
    virtual dds_status_t dds_freq_single(uint8_t profile_index,
                                     uint32_t freq_hz,
                                     int16_t amplitude_db) = 0;
    
    virtual dds_status_t dds_freq_sweep(uint32_t start_hz,
                                        uint32_t stop_hz,
                                        uint32_t duration,
                                        SweepTimeFormat fmt,
                                        bool continuous) = 0;

    virtual uint32_t dds_freq_ftw(uint32_t freq_hz) const = 0;
    
    // ------------ Implemented Functions ------------
    dds_status_t dds_init();     // ✅ calls sub steps init
    
    // --- Accessors ---
    bool is_initialized() const {return initialized_ ;} // ✅
    
    
protected:
    // --- Accessors ---
    HWAbstraction&              hw_;           // ✅
    HWAbstraction::spi_config_t spi_cfg_;      // ✅
    const pin_t*                pins_;         // ✅
    size_t                      pins_count_;   // ✅
    std::vector<uint8_t>        pin_indices_;  // ✅

    void set_initialized(bool v = true) { initialized_ = v; }   // ✅

    // --- Hardware helpers ---
    static dds_status_t from_hw(HWAbstraction::hw_status_t hs); // ✅
    HWAbstraction&       hw()       { return hw_; }         // ✅
    const HWAbstraction& hw() const { return hw_; }         // ✅

    // --- GPIO helpers ---
    dds_status_t pin_read(uint8_t pin_i, HWAbstraction::hw_pin_value_t* val);  // ✅
    dds_status_t pin_write(uint8_t pin_i, HWAbstraction::hw_pin_value_t val);  // ✅

    // --- SPI helpers ---
    dds_status_t spi_tx(const uint8_t* data, size_t len);  // ✅
    dds_status_t spi_rx(uint8_t* data, size_t len);        // ✅

    // --- Sequences Helpers (implemented by Subclasses) ---
    dds_status_t dds_seq_run(const DdsSequence* seq, size_t count);           // ✅🤔
    virtual const DdsSequence* get_seq_setup(size_t& count) const = 0;

    // --- Initialization sub-steps --- 
    virtual dds_status_t dds_validate_context() = 0;  // ✅ device-specific context
    dds_status_t dds_attach_board(const pin_t* pins, size_t pins_count); // ✅
    dds_status_t dds_setup_pins();                   // ✅ 🤔 runs the setup sequence
    dds_status_t spi_init();                         // ✅
    virtual dds_status_t dds_setup_reg()  = 0;       // ✅ 🤔 initializes device-specific registers 

    bool initialized_ = false;  
};


