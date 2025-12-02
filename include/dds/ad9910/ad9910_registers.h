#pragma once
#include "registers.h"
#include "core_types.h"


// ----------------------------------------
//           AD9910 PROFILE HELPER
// ----------------------------------------

// --- Single Tone Profile Helper --- 
template<uint8_t Addr>
struct PROFILE : Register<Addr, 8> {
    static constexpr uint8_t index = Addr - 0x0E;

    using FTW1 = Field<0 , 8>;          // Frequency Tuning Word Byte 1 
    using FTW2 = Field<8 , 8>;          // Frequency Tuning Word Byte 2
    using FTW3 = Field<16, 8>;          // Frequency Tuning Word Byte 3
    using FTW4 = Field<24, 8>;          // Frequency Tuning Word Byte 4
    using POW5 = Field<32, 8>;          // Phase Offset Word Byte 5
    using POW6 = Field<40, 8>;          // Phase Offset Word Byte 6
    using ASF7 = Field<48, 8>;          // Amplitude Scale Factor Byte 7
    using ASF8 = Field<56, 6>;          // Amplitude Scale Factor Byte 8

    static value_type single_tone(uint32_t ftw, uint16_t pow, uint16_t asf){
        PROFILE::shadow.val = 0;

        // FTW (32-bit, LSB first)
        PROFILE::write<FTW1>(static_cast<uint8_t>( ftw        & 0xFFu));
        PROFILE::write<FTW2>(static_cast<uint8_t>((ftw >>  8) & 0xFFu));
        PROFILE::write<FTW3>(static_cast<uint8_t>((ftw >> 16) & 0xFFu));
        PROFILE::write<FTW4>(static_cast<uint8_t>((ftw >> 24) & 0xFFu));

        // POW (16-bit, LSB first)
        PROFILE::write<POW5>(static_cast<uint8_t>( pow        & 0xFFu));
        PROFILE::write<POW6>(static_cast<uint8_t>((pow >>  8) & 0xFFu));

        // ASF (14-bit)
        uint16_t asf14 = asf & 0x3FFFu;  // enforce 14 bits
        PROFILE::write<ASF7>(static_cast<uint8_t>( asf14        & 0xFFu));   // ASF[7:0]
        PROFILE::write<ASF8>(static_cast<uint8_t>((asf14 >>  8) & 0x3Fu));   // ASF[13:8], 6 bits
        return PROFILE::shadow;
    }

};


// ----------------------------------------
//             AD9910 REGISTERS
// ----------------------------------------
namespace ad9910_reg {

    // bit7 = 0 → write ;  bit7 = 1 → read
    constexpr uint8_t SPI_READ = 0x80;

    // ===== CFR1 (32-bit) =====
    struct CFR1 : Register<0x00, 4> {

        using RAM_ENABLE        = Field<31>;    // [31] RAM enable
        using RAM_DEST          = Field<29,2>;  // [30:29] RAM playback destination
        using MANUAL_OSK        = Field<23>;    // [23] Manual OSK external control
        using INV_SINC          = Field<22>;    // [22] Inverse sinc filter enable
        using INT_PROFILE       = Field<17,4>;  // [20:17] Internal profile control
        using DDS_SINE          = Field<16>;    // [16] Select DDS sine output
        using LOAD_LRR          = Field<15>;    // [15] Load LRR on I/O update
        using AUTOCLR_DRG_ACC   = Field<14>;    // [14] Autoclear digital ramp accumulator
        using AUTOCLR_PHASE_ACC = Field<13>;    // [13] Autoclear phase accumulator
        using CLR_DRG_ACC       = Field<12>;    // [12] Clear digital ramp accumulator
        using CLR_PHASE_ACC     = Field<11>;    // [11] Clear phase accumulator
        using LOAD_ARR          = Field<10>;    // [10] Load ARR on I/O update
        using OSK_ENABLE        = Field<9>;     // [9] OSK enable
        using AUTO_OSK          = Field<8>;     // [8] Select auto OSK
        using PD_DIGITAL        = Field<7>;     // [7] Digital power-down
        using PD_DAC            = Field<6>;     // [6] DAC power-down
        using PD_REFCLK         = Field<5>;     // [5] REFCLK input power-down
        using PD_AUX_DAC        = Field<4>;     // [4] Auxiliary DAC power-down
        using EXT_PD_MODE       = Field<3>;     // [3] External power-down mode
        using SDIO_INPUT_ONLY   = Field<1>;     // [1] SDIO input-only
        using LSB_FIRST         = Field<0>;     // [0] LSB first

        // --- Helpers
        static value_type defaults() {
            CFR1::shadow.val = 0;
            CFR1::set  <SDIO_INPUT_ONLY >(true);  // SDIO Input Only;   
            return CFR1::shadow;}

        static value_type drg_setup() {
            CFR1::shadow.val = 0;
            CFR1::set  <AUTOCLR_DRG_ACC >(true);
            CFR1::set  <SDIO_INPUT_ONLY >(true);
            return CFR1::shadow;}

        static value_type drg_basic() {
            CFR1::set  <RAM_ENABLE      >(false);
            CFR1::insert<RAM_DEST        >(0u);
            CFR1::set  <OSK_ENABLE      >(false);
            CFR1::set  <AUTOCLR_DRG_ACC >(true);
            CFR1::set  <SDIO_INPUT_ONLY >(true);
            return CFR1::shadow;}

    };

    // ===== CFR2 (32-bit) =====
    struct CFR2 : Register<0x01, 4> {

        using AMP_SCALE_PROFILE   = Field<24>;      // [24] Amplitude scale from profile
        using INT_IO_UPDATE       = Field<23>;      // [23] Internal I/O update (enables I/O update rate register)
        using SYNC_CLK_ENABLE     = Field<22>;      // [22] SYNC_CLK enable
        using DR_DEST             = Field<20,2>;    // [21:20] Digital ramp destination
        using DR_ENABLE           = Field<19>;      // [19] Digital ramp enable
        using DR_NODWELL_HIGH     = Field<18>;      // [18] Digital ramp no-dwell high
        using DR_NODWELL_LOW      = Field<17>;      // [17] Digital ramp no-dwell low
        using READ_EFFECTIVE_FTW  = Field<16>;      // [16] Read effective FTW
        using IO_UPDATE_RATE_CTRL = Field<14,2>;    // [15:14] I/O update rate control
        using PDCLK_ENABLE        = Field<11>;      // [11] PDCLK enable
        using PDCLK_INVERT        = Field<10>;      // [10] PDCLK invert
        using TXENABLE_INVERT     = Field<9>;       // [9] TxEnable invert
        using MATCHED_LATENCY     = Field<7>;       // [7] Matched latency enable
        using HOLD_LAST_VALUE     = Field<6>;       // [6] Data assembler hold last value
        using SYNC_VAL_DISABLE    = Field<5>;       // [5] Sync timing validation disable
        using PAR_DATA_ENABLE     = Field<4>;       // [4] Parallel data port enable
        using FM_GAIN             = Field<0,4>;     // [3:0] FM gain

        // --- Helpers
        static value_type defaults() {
            CFR2::shadow.val = 0;     
            CFR2::set<AMP_SCALE_PROFILE>(true);     // Enable_amplitude_scale_from_single_tone_profiles
            CFR2::set<SYNC_VAL_DISABLE >(true);     // Sync_timing_validation_disable
            return CFR2::shadow;}

        static value_type drg_freq_enable(bool continuous) {
            CFR2::shadow.val = 0;         
            CFR2::set  <DR_ENABLE        >(true);
            // CFR2::write<DR_DEST>(0u); // Freq destination (Already set becuase of shadow.val = 0)
            CFR2::set  <DR_NODWELL_HIGH  >(continuous);
            CFR2::set  <DR_NODWELL_LOW   >(continuous);
            return CFR2::shadow;}
    };

    // ===== CFR3 (32-bit) =====
    struct CFR3 : Register<0x02, 4> {

        using DRV0            = Field<28,2>;    // [29:28] DRV0 – REFCLK_OUT drive strength
        using VCO_SEL         = Field<24,3>;    // [26:24] VCO range select
        using ICP             = Field<19,3>;    // [21:19] Charge pump current
        using REF_DIV_BYPASS  = Field<15>;      // [15] REFCLK input divider bypass
        using REF_DIV_RESETB  = Field<14>;      // [14] REFCLK input divider ResetB
        using PFD_RESET       = Field<10>;      // [10] PFD reset
        using PLL_ENABLE      = Field<8>;       // [8] PLL enable
        using NPLL            = Field<1,7>;     // [7:1] N – PLL feedback divider modulus
        
        // --- ICP SETTINGS
        enum class IcpCode : uint8_t { // Charge pump current codes (3-bit field)
            u212 = 0x00,  // 212 µA
            u237 = 0x08,  // 237 µA
            u262 = 0x10,  // 262 µA
            u287 = 0x18,  // 287 µA
            u312 = 0x20,  // 312 µA
            u337 = 0x28,  // 337 µA
            u363 = 0x30,  // 363 µA
            u387 = 0x38   // 387 µA
        }
        static void set_icp(IcpCode icp) { write<ICP>(static_cast<uint8_t>(icp));}
        
        // --- VCO SETTINGS
        enum class VcoSel : uint8_t {
            VCO0 = 0,  // 370–510 MHz
            VCO1 = 1,  // 420–590 MHz
            VCO2 = 2,  // 500–700 MHz
            VCO3 = 3,  // 600–880 MHz
            VCO4 = 4,  // 700–950 MHz
            VCO5 = 5,  // 820–1150 MHz
            Invalid = 0xFF
        };
        static void set_vco(VcoSel vco_sel){ write<VCO_SEL_FIELD>(static_cast<uint8_t>(vco_sel));}

        // --- Helpers
        static value_type default_pll_off(bool ref_div2) {
            CFR3::shadow.val = 0;     
            CFR3::set<REF_DIV_RESETB>(true);         // REFCLK divider reset always = 1
            CFR3::set<REF_DIV_BYPASS>(!ref_div2);    // REFCLK divider bypass       = !ref_div2
            return CFR3::shadow;}                   
        
        static value_type default_pll_on(   bool ref_div2       ,
                                            uint8_t pll_mult    ,
                                            VcoSel vco_sel      , 
                                            IcpCode icp ) {
            CFR3::shadow.val = 0; 
            CFR3::set<REF_DIV_RESETB>(true);        // REFCLK divider reset always = 1
            CFR3::set<PLL_ENABLE>(true);            // Enable PLL
            set_icp(icp);                           // Set max charge pump current
            set_vco(vco_sel);                       // Set VCO range
            CFR3::write<NPLL>(pll_mult);            // Set PLL multiplier N
            return CFR3::shadow;}
    };

    // ===== Aux DAC (FSC), 32-bit, only low byte used =====
    struct AUX_DAC : Register<0x03, 4> {
        using FSC = Field<0,8>;     // [7:0] full-scale current control

        // --- Helpers
        static value_type defaults(bool current_high){
            AUX_DAC::shadow.val = 0;
            AUX_DAC::insert<FSC>(current_high ? 0xFFu : 0x7Fu);
            return AUX_DAC::shadow;
        }

    };

    // ===== FTW (32-bit) =====
    struct FTW : Register<0x07, 4> {

        // 31:0 – frequency tuning word, exposed as four bytes
        using WORD_BYTE3 = Field<24,8>; // MSB
        using WORD_BYTE2 = Field<16,8>;
        using WORD_BYTE1 = Field<8,8>;
        using WORD_BYTE0 = Field<0,8>;  // LSB
    };

    // ===== POW (16-bit) =====
    struct POW : Register<0x08, 2> {

        // 15:0 – phase offset word, exposed as two bytes
        using WORD_MSB = Field<8,8>;
        using WORD_LSB = Field<0,8>;
    };

    // ===== ASF (32-bit) =====
    struct ASF : Register<0x09, 4> {

        // [31:16] amplitude ramp rate (split into two bytes)
        using RAMP_RATE_MSB = Field<24,8>;
        using RAMP_RATE_LSB = Field<16,8>;

        // [15:2] amplitude scale factor (14 bits, split)
        using SCALE_MSB     = Field<8,8>;       // [15:8]
        using SCALE_LSB     = Field<2,6>;       // [7:2]
        using STEP_SIZE     = Field<0,2>;       // [1:0] amplitude step size
    };

    // ===== Multichip Sync (32-bit) =====
    struct MULTICHIP_SYNC : Register<0x0A, 4> {

        using SYNC_RX_EN = Field<27>;
        using SYNC_TX_EN = Field<26>;
        using SYNC_TX_POL= Field<25>;
    };


    // ===== DR_LIMIT (64-bit) =====
    struct DR_LIMIT : Register<0x0B, 8> {
        // LOWER limit (LSB first)
        using LOWER1 = Field<0 , 8>;   // bits 7:0
        using LOWER2 = Field<8 , 8>;   // bits 15:8
        using LOWER3 = Field<16, 8>;   // bits 23:16
        using LOWER4 = Field<24, 8>;   // bits 31:24
        // UPPER limit (next 32 bits)
        using UPPER1 = Field<32, 8>;   // bits 39:32
        using UPPER2 = Field<40, 8>;   // bits 47:40
        using UPPER3 = Field<48, 8>;   // bits 55:48
        using UPPER4 = Field<56, 8>;   // bits 63:56

        static value_type set_limit(uint32_t lower, uint32_t upper) {
            shadow.val = 0;
            // LOWER (LSB first)
            write<LOWER1>(static_cast<uint8_t>( lower        & 0xFFu));
            write<LOWER2>(static_cast<uint8_t>((lower >>  8) & 0xFFu));
            write<LOWER3>(static_cast<uint8_t>((lower >> 16) & 0xFFu));
            write<LOWER4>(static_cast<uint8_t>((lower >> 24) & 0xFFu));
            // UPPER (LSB first within the upper half)
            write<UPPER1>(static_cast<uint8_t>( upper        & 0xFFu));
            write<UPPER2>(static_cast<uint8_t>((upper >>  8) & 0xFFu));
            write<UPPER3>(static_cast<uint8_t>((upper >> 16) & 0xFFu));
            write<UPPER4>(static_cast<uint8_t>((upper >> 24) & 0xFFu));

            return shadow;
        }
    };

    // ===== DR_STEP (64-bit) =====
    struct DR_STEP : Register<0x0C, 8> {
        // Increment step size (LSB first, bits 31:0)
        using INC1 = Field<0 , 8>;   // bits 7:0
        using INC2 = Field<8 , 8>;   // bits 15:8
        using INC3 = Field<16, 8>;   // bits 23:16
        using INC4 = Field<24, 8>;   // bits 31:24
        // Decrement step size (bits 63:32)
        using DEC1 = Field<32, 8>;   // bits 39:32
        using DEC2 = Field<40, 8>;   // bits 47:40
        using DEC3 = Field<48, 8>;   // bits 55:48
        using DEC4 = Field<56, 8>;   // bits 63:56

        static value_type set_step(uint32_t inc, uint32_t dec) {
            shadow.val = 0;
            // Increment step size (LSB first)
            write<INC1>(static_cast<uint8_t>( inc        & 0xFFu));
            write<INC2>(static_cast<uint8_t>((inc >>  8) & 0xFFu));
            write<INC3>(static_cast<uint8_t>((inc >> 16) & 0xFFu));
            write<INC4>(static_cast<uint8_t>((inc >> 24) & 0xFFu));
            // Decrement step size (LSB first within upper 32 bits)
            write<DEC1>(static_cast<uint8_t>( dec        & 0xFFu));
            write<DEC2>(static_cast<uint8_t>((dec >>  8) & 0xFFu));
            write<DEC3>(static_cast<uint8_t>((dec >> 16) & 0xFFu));
            write<DEC4>(static_cast<uint8_t>((dec >> 24) & 0xFFu));

            return shadow;
        }
    };

    // ===== DR_RATE (32-bit) =====
    struct DR_RATE : Register<0x0D, 4> {
        // Positive slope rate (bits 15:0), LSB first
        using POS1 = Field<0 , 8>;   // bits 7:0
        using POS2 = Field<8 , 8>;   // bits 15:8
        // Negative slope rate (bits 31:16)
        using NEG1 = Field<16, 8>;   // bits 23:16
        using NEG2 = Field<24, 8>;   // bits 31:24

        static value_type set_rate(uint16_t pos, uint16_t neg) {
            shadow.val = 0;
            // positive slope
            write<POS1>(static_cast<uint8_t>( pos        & 0xFFu));
            write<POS2>(static_cast<uint8_t>((pos >>  8) & 0xFFu));
            // negative slope
            write<NEG1>(static_cast<uint8_t>( neg        & 0xFFu));
            write<NEG2>(static_cast<uint8_t>((neg >>  8) & 0xFFu));
            return shadow;
        }
    };

    // ===== SINGLE TONE PROFILES (64-bit) =====
    struct PROFILE0 : PROFILE<0x0E> {};
    struct PROFILE1 : PROFILE<0x0F> {};
    struct PROFILE2 : PROFILE<0x10> {};
    struct PROFILE3 : PROFILE<0x11> {};
    struct PROFILE4 : PROFILE<0x12> {};
    struct PROFILE5 : PROFILE<0x13> {};
    struct PROFILE6 : PROFILE<0x14> {};
    struct PROFILE7 : PROFILE<0x15> {};

} // namespace ad9910_reg
