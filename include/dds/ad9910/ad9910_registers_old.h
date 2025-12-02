#pragma once
#include "core_types.h"
#include "registers.h"
#include "core_types.h"
#include <array>
#include <type_traits>


// Fragility report: 
// If we add a >32-bit register (AD9910 RAM registers are 48 bits).
// Those fields must be written as: Field<T> FOO{ T(0x0000FFFFFFFF0000ULL), ... };


namespace ad9910_reg {

    constexpr uint8_t SPI_READ = 0x80;

    // ---- Complete serial addresses ----
    enum class Reg : uint8_t {
        CFR1          = 0x00,
        CFR2          = 0x01,
        CFR3          = 0x02,
        AUX_DAC       = 0x03,
        IO_UPDATE_RATE= 0x04,
        // 0x05, 0x06 reserved in datasheet register map
        FTW           = 0x07,
        POW           = 0x08,
        ASF           = 0x09,
        MULTICHIP_SYNC= 0x0A,
        DR_LIMIT      = 0x0B,
        DR_STEP       = 0x0C,
        DR_RATE       = 0x0D,
        PROFILE0      = 0x0E,
        PROFILE1      = 0x0F,
        PROFILE2      = 0x10,
        PROFILE3      = 0x11,
        PROFILE4      = 0x12,
        PROFILE5      = 0x13,
        PROFILE6      = 0x14,
        PROFILE7      = 0x15,
        RAM           = 0x16
    };

    // ===== CFR1 (32-bit) =====
    // Bit definitions from Table 17 of the datasheet of ad9910
    struct CFR1 : RegValue<4> {

        static constexpr Reg ID = Reg::CFR1;
        using T = storage_t;

        // [31] RAM enable
        static constexpr Field<T> RAM_ENABLE   { 0x80000000u, 31 };
        // [30:29] RAM playback destination
        static constexpr Field<T> RAM_DEST     { 0x60000000u, 29 };
        // [23] Manual OSK external control
        static constexpr Field<T> MANUAL_OSK   { 0x00800000u, 23 };
        // [22] Inverse sinc filter enable
        static constexpr Field<T> INV_SINC     { 0x00400000u, 22 };
        // [20:17] Internal profile control
        static constexpr Field<T> INT_PROFILE  { 0x001E0000u, 17 };
        // [16] Select DDS sine output
        static constexpr Field<T> DDS_SINE     { 0x00010000u, 16 };

        // [15] Load LRR on I/O update
        static constexpr Field<T> LOAD_LRR     { 0x00008000u, 15 };
        // [14] Autoclear digital ramp accumulator
        static constexpr Field<T> AUTOCLR_DRG_ACC   { 0x00004000u, 14 };
        // [13] Autoclear phase accumulator
        static constexpr Field<T> AUTOCLR_PHASE_ACC { 0x00002000u, 13 };
        // [12] Clear digital ramp accumulator
        static constexpr Field<T> CLR_DRG_ACC       { 0x00001000u, 12 };
        // [11] Clear phase accumulator
        static constexpr Field<T> CLR_PHASE_ACC     { 0x00000800u, 11 };
        // [10] Load ARR on I/O update
        static constexpr Field<T> LOAD_ARR          { 0x00000400u, 10 };

        // [9] OSK enable
        static constexpr Field<T> OSK_ENABLE   { 0x00000200u, 9 };
        // [8] Select auto OSK
        static constexpr Field<T> AUTO_OSK     { 0x00000100u, 8 };
        // [7] Digital power-down
        static constexpr Field<T> PD_DIGITAL   { 0x00000080u, 7 };
        // [6] DAC power-down
        static constexpr Field<T> PD_DAC       { 0x00000040u, 6 };
        // [5] REFCLK input power-down
        static constexpr Field<T> PD_REFCLK    { 0x00000020u, 5 };
        // [4] Auxiliary DAC power-down
        static constexpr Field<T> PD_AUX_DAC   { 0x00000010u, 4 };
        // [3] External power-down mode
        static constexpr Field<T> EXT_PD_MODE  { 0x00000008u, 3 };
        // [1] SDIO input-only
        static constexpr Field<T> SDIO_INPUT_ONLY { 0x00000002u, 1 };
        // [0] LSB first
        static constexpr Field<T> LSB_FIRST    { 0x00000001u, 0 };

    };

    // ===== CFR2 (32-bit) =====
    // Bit definitions from Table 19.
    struct CFR2 : RegValue<4> {
        static constexpr Reg ID = Reg::CFR2;
        using T = storage_t;

        // [24] Amplitude scale from profile
        static constexpr Field<T> AMP_SCALE_PROFILE { 0x01000000u, 24 };
        // [23] Internal I/O update (enables I/O update rate register)
        static constexpr Field<T> INT_IO_UPDATE     { 0x00800000u, 23 };
        // [22] SYNC_CLK enable
        static constexpr Field<T> SYNC_CLK_ENABLE   { 0x00400000u, 22 };
        // [21:20] Digital ramp destination
        static constexpr Field<T> DR_DEST           { 0x00300000u, 20 };
        // [19] Digital ramp enable
        static constexpr Field<T> DR_ENABLE         { 0x00080000u, 19 };
        // [18] Digital ramp no-dwell high
        static constexpr Field<T> DR_NODWELL_HIGH   { 0x00040000u, 18 };
        // [17] Digital ramp no-dwell low
        static constexpr Field<T> DR_NODWELL_LOW    { 0x00020000u, 17 };
        // [16] Read effective FTW
        static constexpr Field<T> READ_EFFECTIVE_FTW{ 0x00010000u, 16 };

        // [15:14] I/O update rate control
        static constexpr Field<T> IO_UPDATE_RATE_CTRL { 0x0000C000u, 14 };

        // [11] PDCLK enable
        static constexpr Field<T> PDCLK_ENABLE      { 0x00000800u, 11 };
        // [10] PDCLK invert
        static constexpr Field<T> PDCLK_INVERT      { 0x00000400u, 10 };
        // [9] TxEnable invert
        static constexpr Field<T> TXENABLE_INVERT   { 0x00000200u, 9 };

        // [7] Matched latency enable
        static constexpr Field<T> MATCHED_LATENCY   { 0x00000080u, 7 };
        // [6] Data assembler hold last value
        static constexpr Field<T> HOLD_LAST_VALUE   { 0x00000040u, 6 };
        // [5] Sync timing validation disable
        static constexpr Field<T> SYNC_VAL_DISABLE  { 0x00000020u, 5 };
        // [4] Parallel data port enable
        static constexpr Field<T> PAR_DATA_ENABLE   { 0x00000010u, 4 };
        // [3:0] FM gain
        static constexpr Field<T> FM_GAIN           { 0x0000000Fu, 0 };

    };

    // ===== CFR3 (32-bit) =====
    // PLL and REFCLK block, Table 20.
    struct CFR3 : RegValue<4> {
        static constexpr Reg ID = Reg::CFR3;
        using T = storage_t;

        // [29:28] DRV0 – REFCLK_OUT drive strength
        static constexpr Field<T> DRV0      { 0x30000000u, 28 };
        // [26:24] VCO range select
        static constexpr Field<T> VCO_SEL   { 0x07000000u, 24 };
        // [21:19] Charge pump current
        static constexpr Field<T> ICP       { 0x00380000u, 19 };

        // [15] REFCLK input divider bypass
        static constexpr Field<T> REF_DIV_BYPASS { 0x00008000u, 15 };
        // [14] REFCLK input divider ResetB
        static constexpr Field<T> REF_DIV_RESETB { 0x00004000u, 14 };
        // [10] PFD reset
        static constexpr Field<T> PFD_RESET      { 0x00000400u, 10 };
        // [8] PLL enable
        static constexpr Field<T> PLL_ENABLE     { 0x00000100u, 8 };
        // [7:1] N – PLL feedback divider modulus
        static constexpr Field<T> N              { 0x000000FEu, 1 };

    };

    // ===== Aux DAC (FSC), 32-bit, only low byte used =====
    struct AUX_DAC : RegValue<4> {
        static constexpr Reg ID = Reg::AUX_DAC;
        using T = storage_t;

        static constexpr Field<T> FSC { 0x000000FFu, 0 };
    };

    // ===== FTW, POW, ASF, Multichip Sync =====

    struct FTW : RegValue<4> {
        static constexpr Reg ID = Reg::FTW;
        using T = storage_t;

        // 31:0 – frequency tuning word
        static constexpr Field<T> WORD { 0xFFFFFFFFu, 0 };
    };

    struct POW : RegValue<2> {
        static constexpr Reg ID = Reg::POW;
        using T = storage_t;

        // 15:0 – phase offset word
        static constexpr Field<T> WORD { 0x0000FFFFu, 0 };
    };

    struct ASF : RegValue<4> {
        static constexpr Reg ID = Reg::ASF;
        using T = storage_t;

        // [31:16] amplitude ramp rate
        static constexpr Field<T> RAMP_RATE { 0xFFFF0000u, 16 };
        // [15:2] amplitude scale factor
        static constexpr Field<T> SCALE     { 0x0000FFFCu, 2 };
        // [1:0] amplitude step size
        static constexpr Field<T> STEP_SIZE { 0x00000003u, 0 };

    };

    struct MULTICHIP_SYNC : RegValue<4> {
        static constexpr Reg ID = Reg::MULTICHIP_SYNC;
        using T = storage_t;

        // Only the bits you actually need from Table 26 are exposed:
        static constexpr Field<T> SYNC_RX_EN { 0x08000000u, 27 };
        static constexpr Field<T> SYNC_TX_EN { 0x04000000u, 26 };
        static constexpr Field<T> SYNC_TX_POL{ 0x02000000u, 25 };
    };


    // ---- generic modifiers ----
    template<class R>
    constexpr R& set(R& r, Field<typename R::storage_t> f, bool on){
        using T = typename R::storage_t;
        r.v = set_bool<T>(r.v & R::REG_MASK, f, on) & R::REG_MASK;
        return r;
    }

    template<class R>
    constexpr R& write(R& r, Field<typename R::storage_t> f, typename R::storage_t x){
        using T = typename R::storage_t;
        r.v = write_field<T>(r.v & R::REG_MASK, f, x) & R::REG_MASK;
        return r;
    }

    template<class R>
    constexpr typename R::storage_t read(const R& r, Field<typename R::storage_t> f){
        using T = typename R::storage_t;
        return read_field<T>(r.v & R::REG_MASK, f);
    }
}

// usage:
// ad9910_reg::CFR1 r1;
// ad9910::set(r1, ad9910_reg::CFR1::RAM_ENABLE, true);
// ad9910::write(r1, ad9910::CFR1::RAM_DEST, 0x3);
// auto b = r1.bytes();
