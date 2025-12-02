#pragma once

#include "core_types.h"
#include "boards/board_abstraction.h"
#include "dds/dds_base.h"
#include "ad9910_context.h"
#include "ad9910_pins.h"
#include "ad9910_registers.h"


// Expected SPI CONFIG
// HWAbstraction::spi_config_t{
//     2'000'000u, // spi_clock_hz (DIV8 at 16 MHz)
//     0,          // bit_order: 0 = MSB first
//     0,          // mode: SPI mode 0
//     0x00        // read_dummy
// }

class AD9910 final : public DDSBase {
public:
    
    // --- Constructor ---
    explicit AD9910(HWAbstraction& hw,
                const HWAbstraction::spi_config_t& spi_cfg,
                const AD9910Context& ad9910_ctx);

    // --- AD9910-specifics : Base Overrides ---
    uint32_t dds_freq_ftw(uint32_t freq_hz) const override;
    dds_status_t dds_freq_single(uint8_t profile_index,
                                     uint32_t freq_hz,
                                     int16_t amplitude_db) override; 

    dds_status_t dds_freq_sweep(uint32_t start_hz,
                                uint32_t stop_hz,
                                uint32_t duration,
                                SweepTimeFormat fmt,
                                bool continuous) override;

    // --- AD9910-specifics : Extra Functionalities ---
    void calc_best_step_rate(uint16_t& step,
                                uint64_t& step_rate,
                                uint32_t f_mod_hz) const;



    
    
protected:
    const AD9910Context     ad9910_ctx_    ;
    uint64_t dds_sysclk_hz() const;
    dds_status_t dds_restart_drg();
    bool ref_div2_;   
    uint64_t sysclk_hz_ = 0; // cached system clock
    bool drg_continuous_ = false;   // remember last DRG mode
    uint8_t pll_mlt_ = 0;        // cached PLL multiplier

    // --- AD9910-specifics : Sequence getters ---
    const DdsSequence* get_seq_setup(size_t& count) const override;
    const DdsSequence* get_seq_update(size_t& count) const override;

    dds_status_t dds_digital_ramp(uint32_t ftw_start,
                                            uint32_t ftw_end,
                                            uint32_t ftw_step,
                                            uint16_t step_rate,
                                            bool continuous);
    // --- AD9910-specifics : Init Function substeps ---
    dds_status_t dds_validate_context() override; // 🤔
    dds_status_t dds_setup_reg() override; // 
 
    // --- AD9910-specifics : Register programming helpers ----
    dds_status_t dds_reg_write(uint8_t addr,
                           const uint8_t* data,
                           size_t len) override; // ✅🤔 

    dds_status_t dds_reg_read(uint8_t addr,
                            uint8_t* buf,
                            size_t len) override; // ✅🤔


    // Typed convenience wrappers (not overrides) // ✅🤔
    dds_status_t dds_reg_write(ad9910_reg::Reg id,
                            const uint8_t* data,
                            size_t len) {
        return dds_reg_write(static_cast<uint8_t>(id), data, len);
    }

    dds_status_t dds_reg_read(ad9910_reg::Reg id,
                            uint8_t* buf,
                            size_t len) {
        return dds_reg_read(static_cast<uint8_t>(id), buf, len);
    }

    // Special registers functions
    dds_status_t dds_program_cfr1();    // ✅🤔
    dds_status_t dds_program_cfr2();    // ✅🤔
    dds_status_t dds_program_cfr3();        // NEW
    dds_status_t dds_update_io_pulse(); // ✅🤔
    dds_status_t dds_program_aux_dac();     // NEW (FSC)

    // --- AD9910-specifics : Sequences ---
    static constexpr std::array<DdsSequence, 19> seq_setup_ = {{
        // Actually we should wait here 50 // TODO 
        {  idx(DdsPin::IO_UPDATE),    HWAbstraction::HW_PIN_LOW, 50 },
        {  idx(DdsPin::MASTER_RESET), HWAbstraction::HW_PIN_LOW, 50 },
        {  idx(DdsPin::IO_RESET),     HWAbstraction::HW_PIN_LOW, 50 },
        {  idx(DdsPin::SPI_CS),       HWAbstraction::HW_PIN_LOW, 50 },
        {  idx(DdsPin::OSK),          HWAbstraction::HW_PIN_LOW, 50 },
        {  idx(DdsPin::PROFILE0),     HWAbstraction::HW_PIN_LOW, 0  },
        {  idx(DdsPin::PROFILE1),     HWAbstraction::HW_PIN_LOW, 0  },
        {  idx(DdsPin::PROFILE2),     HWAbstraction::HW_PIN_LOW, 0  },
        {  idx(DdsPin::DRHOLD),       HWAbstraction::HW_PIN_LOW, 0  },
        {  idx(DdsPin::DRCTL),        HWAbstraction::HW_PIN_LOW, 0  },
        {  idx(DdsPin::PWR_DWN),      HWAbstraction::HW_PIN_LOW, 50 },

        // -------------------------- (Second part not in the OG DDS_GPIO_Init )

        { idx(DdsPin::MASTER_RESET), HWAbstraction::HW_PIN_HIGH, 10 },
        { idx(DdsPin::MASTER_RESET), HWAbstraction::HW_PIN_LOW,  0  },
        { idx(DdsPin::IO_UPDATE),    HWAbstraction::HW_PIN_LOW,  0  },
        { idx(DdsPin::SPI_CS),       HWAbstraction::HW_PIN_HIGH, 0  },
        { idx(DdsPin::OSK),          HWAbstraction::HW_PIN_HIGH, 0  },
        { idx(DdsPin::PROFILE0),     HWAbstraction::HW_PIN_LOW,  0  },
        { idx(DdsPin::PROFILE1),     HWAbstraction::HW_PIN_LOW,  0  },
        { idx(DdsPin::PROFILE2),     HWAbstraction::HW_PIN_LOW,  0  },

    }};

};


