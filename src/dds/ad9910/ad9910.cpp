#include <cmath>
#include "dds/dds_base.h"
#include "dds/ad9910/ad9910.h"
#include "dds/ad9910/ad9910_registers.h"
#include "dds/ad9910/ad9910_pins.h"

// ----------------------------------------
//               👩‍⚕️ Helpers 
// ----------------------------------------
static uint16_t calc_ampl_scale_factor(int16_t ampl_db){ //🤔 Check me 
    // 10^((ampl_db + 84.288) / 20) < 1  ⇔  ampl_db <= -85
    // --- Handle edge cases --------------------------------------------------
    if (ampl_db <= -85) { return 0;         }
    if (ampl_db >= 0)   { return 0x3FFFu;   }

    // --- General case -------------------------------------------------------
    static constexpr float kDbOffset   = 84.288f;
    static constexpr float kInvTwenty  = 1.0f / 20.0f;
    static constexpr uint32_t kMaxAsf  = 0x3FFFu;
    const float exponent = (static_cast<float>(ampl_db) + kDbOffset) * kInvTwenty;
    const float asf_f    = std::pow(10.0f, exponent);
    uint32_t asf_u = static_cast<uint32_t>(asf_f);
    if (asf_u > kMaxAsf) { asf_u = kMaxAsf;}
    return static_cast<uint16_t>(asf_u);
}

static bool convert_to_ns(SweepTimeFormat fmt, int64_t duration, uint64_t& ns_out){
    if (duration <= 0) { return false;}
    uint64_t d = static_cast<uint64_t>(duration);
    constexpr uint64_t max64u = UINT64_MAX;
    switch (fmt) {
        case SweepTimeFormat::Seconds: {            // d * 1e9
            const uint64_t mul = 1000000000ull;
            if (d > max64u / mul) return false;
            ns_out = d * mul;
            return true;}
        case SweepTimeFormat::Milliseconds: {       // d * 1e6
            const uint64_t mul = 1000000ull;
            if (d > max64u / mul) return false;
            ns_out = d * mul;
            return true;}
        case SweepTimeFormat::Microseconds: {       // d * 1e3
            const uint64_t mul = 1000ull;
            if (d > max64u / mul) return false;
            ns_out = d * mul;
            return true;}
        case SweepTimeFormat::Nanoseconds:
            ns_out = d;
            return true;
        default:
            return false;
    }
}

static VcoSel pick_vco_sel(uint64_t clk) {
    if (clk < 420000000ULL || clk > 1000000000ULL) return VcoSel::Invalid;
    if (clk <= 510000000ULL) return VcoSel::VCO0; // 370–510
    if (clk <= 590000000ULL) return VcoSel::VCO1; // 420–590
    if (clk <= 700000000ULL) return VcoSel::VCO2; // 500–700
    if (clk <= 880000000ULL) return VcoSel::VCO3; // 600–880
    if (clk <= 950000000ULL) return VcoSel::VCO4; // 700–950
    return VcoSel::VCO5;                          // 820–1000
}

uint32_t AD9910::dds_freq_ftw(uint32_t freq_hz) const{
    const uint32_t sysclk = dds_sysclk_hz();
    if (sysclk == 0 || freq_hz == 0) { return 0;}
    if (freq_hz >= sysclk) { return 0xFFFFFFFFu;}       // If req freq ≥ sysclk, the ideal FTW ≥ 2^32 → saturate
    const uint64_t numerator = (uint64_t)freq_hz << 32; // FTW = round( freq_hz * 2^32 / sysclk )
    const uint64_t rounded   = numerator + (sysclk / 2u);
    return static_cast<uint32_t>(rounded / sysclk);
}

// ----------------------------------------
//             👩 Initializier ✅
// ----------------------------------------
AD9910::AD9910(HWAbstraction& hw,
               const HWAbstraction::spi_config_t& spi_cfg,
               const AD9910Context& ad9910_ctx)
    : DDSBase(hw,
              spi_cfg,
              dds_pins().data(),                  // pointer to first pin_t
              dds_pins().size())   // number of pins
    , ad9910_ctx_(ad9910_ctx)
{}

// --- Sequence Getters (required overrides) --- ✅
const DDSBase::DdsSequence* AD9910::get_seq_setup(size_t& count) const {
    count = seq_setup_.size();
    return seq_setup_.data();
}

// --- AD9910-specifics : Base Overrides --- 🤔 Check me 
dds_status_t AD9910::dds_validate_context() {
    auto &c = ad9910_ctx_;
    uint64_t ref = c.ref_clk_hz;

    // === PLL DISABLED: SYSCLK = REFCLK (PLL bypass) ===
    if (!c.pll_enable) {

        // Datasheet: AD9910 system clock is specified up to 1.0 GHz
        const uint64_t sysclk_max = 1000000000ULL; // 1.0 GHz

        // Require non-zero REFCLK and stay within 1.0 GHz rating
        if (ref == 0 || ref > sysclk_max) return dds_status_t::DDS_INVALID_PARAM;

        this->ref_div2_  = false;       // REFCLK divider not used
        this->sysclk_hz_ = ref;         // SYSCLK = REFCLK in bypass
        this->vco_sel_   = 6;           // 110b = PLL bypass in CFR3

        return dds_status_t::DDS_OK;
    }

    // === PLL ENABLED ===
    uint64_t pll_in = ref;
    bool div2 = false;

    // Try direct REFCLK first, otherwise fall back to /2.
    // Datasheet: PLL reference input must be 3.2 MHz – 60 MHz.
    if (pll_in < 3200000ULL || pll_in > 60000000ULL) {
        pll_in /= 2;
        div2 = true;
    }

    // Still illegal after /2? Then configuration is impossible.
    if (pll_in < 3200000ULL || pll_in > 60000000ULL)
        return dds_status_t::DDS_INVALID_PARAM;

    this->ref_div2_ = div2;

    // PLL multiplier N (CFR3[7:1]), datasheet: 12 ≤ N ≤ 127
    if (c.pll_mult < 12 || c.pll_mult > 127)
        return dds_status_t::DDS_INVALID_PARAM;

    // SYSCLK = pll_in * (N / 2)
    uint64_t sysclk = (pll_in * static_cast<uint64_t>(c.pll_mult)) / 2ULL;

    // Select appropriate VCO band (will also reject anything outside 420–1000 MHz)

    auto vco = pick_vco_sel(sysclk);
    if (vco == VcoSel::Invalid) return dds_status_t::DDS_INVALID_PARAM;
    // Cache for later use
    this->sysclk_hz_ = sysclk;
    this->vco_sel_   = static_cast<uint8_t>(vco);

    return dds_status_t::DDS_OK;
}

dds_status_t AD9910::dds_setup_reg() { 
    dds_status_t s = dds_status_t::DDS_OK;
    // ---- CFR1 ----
    TRY_OK( dds_cfr1_defaults(),     s, s);
    TRY_OK( dds_update_io_pulse(),  s, s);
    // ---- CFR2 ----
    TRY_OK( dds_cfr2_defaults(),    s, s);
    TRY_OK( dds_update_io_pulse(),  s, s);
    // ---- CFR3 ----
    TRY_OK( dds_cfr3_defaults(  ref_div2_,
                                ad9910_ctx_.pll_enable,
                                ad9910_ctx_.pll_mult
                                VcoSel::VCO5, IcpCode::u387) ,    s, s);
    TRY_OK( dds_update_io_pulse(),  s, s);
    // ---- AUX DAC (FSC) ----
    TRY_OK( dds_program_aux_dac(),  s, s);
    TRY_OK( dds_update_io_pulse(),  s, s);
    return s;}

uint64_t AD9910::dds_sysclk_hz() const { // 🤔 Check me
    return sysclk_hz_;  // set in dds_validate_context()
}

// --- Register programming helpers ---
dds_status_t AD9910::dds_reg_write(uint8_t addr, const uint8_t* data, size_t len){
    if (len == 0 || !data)  return dds_status_t::DDS_INVALID_PARAM;
    const uint8_t cs_i = idx(DdsPin::SPI_CS);
    // 1) CS low
    dds_status_t s = pin_write(cs_i, HWAbstraction::HW_PIN_LOW);
    if (s != dds_status_t::DDS_OK) {
        pin_write(cs_i, HWAbstraction::HW_PIN_HIGH); // try to deassert CS, but keep original error
        return s;}
    // 2) send header (write op: MSB = 0)
    uint8_t header = static_cast<uint8_t>(addr & 0x7Fu);
    s = spi_tx(&header, 1);
    if (s != dds_status_t::DDS_OK) {
        pin_write(cs_i, HWAbstraction::HW_PIN_HIGH);
        return s;}
    TRY_OK( spi_tx(data,len)                               ,s,s); // 3) send payload
    TRY_OK( pin_write(cs_i, HWAbstraction::HW_PIN_HIGH)    ,s,s); // 4) CS high                          
    return s;
}

dds_status_t AD9910::dds_reg_read(uint8_t addr, uint8_t* buf, size_t len){
    if (len == 0 || !buf)  return dds_status_t::DDS_INVALID_PARAM;
    const uint8_t cs_i = idx(DdsPin::SPI_CS);

    // 1) CS low
    dds_status_t s = pin_write(cs_i, HWAbstraction::HW_PIN_LOW);
    if (s != dds_status_t::DDS_OK) {
        pin_write(cs_i, HWAbstraction::HW_PIN_HIGH);
        return s;}

    // 2) send header (read op: MSB = 1)
    uint8_t header = static_cast<uint8_t>(addr | ad9910_reg::SPI_READ);
    s = spi_tx(&header, 1);
    if (s != dds_status_t::DDS_OK) {
        pin_write(cs_i, HWAbstraction::HW_PIN_HIGH);
        return s;}

    TRY_OK( spi_rx(buf,len)                               ,s,s);   //  3) read payload NOTE: no cast here
    TRY_OK( pin_write(cs_i, HWAbstraction::HW_PIN_HIGH)   ,s,s);   // 4) CS high                          
    return s;
}


// ----------------------------------------
//               🛠️ Doing 
// ----------------------------------------
dds_status_t AD9910::dds_freq_sweep(uint32_t start_hz,
                                    uint32_t stop_hz,
                                    uint32_t duration,
                                    SweepTimeFormat fmt,
                                    bool continuous){

    if (!is_initialized()) return dds_status_t::DDS_NOT_INITIALIZED;
    if ( duration == 0 || start_hz == 0 || stop_hz == 0 || stop_hz <= start_hz) 
        return dds_status_t::DDS_INVALID_PARAM;

    // --- convert duration + fmt → nanoseconds ---
    uint64_t desired_ns = 0;
    if (!convert_to_ns(fmt, duration, desired_ns)) return dds_status_t::DDS_INVALID_PARAM;


    // 1) FTWs
    const uint32_t ftw_start = dds_freq_ftw(start_hz);
    const uint32_t ftw_end   = dds_freq_ftw(stop_hz);
    const uint32_t delta_ftw = ftw_end - ftw_start;
    if (delta_ftw == 0  ) return dds_status_t::DDS_INVALID_PARAM;



    // --------------------------------------------
    //  
    // -------------------------------------------- 
    // 2) DRG timing
    uint64_t sysclk = dds_sysclk_hz();
    if (sysclk == 0) return dds_status_t::DDS_INVALID_PARAM;


    const long double core_ghz = static_cast<long double>(sysclk) / 1.0e9L;
    if (core_ghz <= 0.0L) return dds_status_t::DDS_INVALID_PARAM;

    uint32_t ftw_step  = 1u;
    uint16_t step_rate = 1u;

    const long double base_ns =
        (4.0L / core_ghz) *
        static_cast<long double>(delta_ftw) *
        static_cast<long double>(ftw_step);

    if (base_ns <= 0.0L) return dds_status_t::DDS_INVALID_PARAM;

    if (base_ns < desired_ns) {
        long double mult = desired_ns / base_ns;
        uint32_t sr = static_cast<uint32_t>(std::llround(mult));
        if (sr == 0)     sr = 1;
        if (sr > 0xFFFF) sr = 0xFFFF;
        step_rate = static_cast<uint16_t>(sr);
    } else if (base_ns > desired_ns) {
        long double mult = base_ns / desired_ns;
        uint32_t fs = static_cast<uint32_t>(std::llround(mult));
        if (fs == 0)         fs = 1;
        if (fs > delta_ftw)  fs = delta_ftw;
        ftw_step = fs;
    }

    if (ftw_step == 0 || step_rate == 0) return dds_status_t::DDS_INVALID_PARAM;


    // --------------------------------------------
    //  
    // -------------------------------------------- 



    dds_status_t s = dds_status_t::DDS_OK; 
    // --- 3) Program CFR1 for digital ramp (OG DigitalRamp CFR1) ---
    TRY_OK ( dds_drg_cfr1_sweep()  ,s,s)

    // --- 4) Program DR_LIMIT, DR_STEP, DR_RATE (OG DigitalRamp) ---
    TRY_OK ( dds_drg_program_range(ftw_start,ftw_end,ftw_step,step_rate)  ,s,s);

    // --- 5) Enable DRG in CFR2 (frequency destination) ---
    TRY_OK( dds_drg_enable_freq(continuous)  ,s,s);

    // --- 6) Select profile 0 and arm DRCTL (OG does this) ---
    TRY_OK( pin_write(idx(DdsPin::PROFILE0), HWAbstraction::HW_PIN_LOW),  s, s ); // PROFILE[2:0] = 000 → profile 0
    TRY_OK( pin_write(idx(DdsPin::PROFILE1), HWAbstraction::HW_PIN_LOW),  s, s );
    TRY_OK( pin_write(idx(DdsPin::PROFILE2), HWAbstraction::HW_PIN_LOW),  s, s );
    TRY_OK( pin_write(idx(DdsPin::DRCTL),    HWAbstraction::HW_PIN_HIGH), s, s );  // DRCTL high = run DRG

    // --- 7) Apply all changes ---
    return dds_update_io_pulse();
}







dds_status_t AD9910::dds_freq_single(   uint8_t profile_index, 
                                        uint32_t freq_hz, 
                                        int16_t amplitude_db){
                                            
                                return dds_status_t::DDS_OK;
}

template<typename Profile>
dds_status_t AD9910::set_profile(){
    dds_status_t s;
    const uint8_t profile_index = Profile::index;
    const uint8_t bits = profile_index & 0x07;
    auto setp = [this](DdsPin pin, bool high) {
        return pin_write(idx(pin),
                         high ? HWAbstraction::HW_PIN_HIGH
                              : HWAbstraction::HW_PIN_LOW); };

    if ((s = setp(DdsPin::PROFILE0, (bits & 0x01) != 0)) != dds_status_t::DDS_OK) return s;
    if ((s = setp(DdsPin::PROFILE1, (bits & 0x02) != 0)) != dds_status_t::DDS_OK) return s;
    if ((s = setp(DdsPin::PROFILE2, (bits & 0x04) != 0)) != dds_status_t::DDS_OK) return s;
    return dds_status_t::DDS_OK;
}


// 🤔 Check me 
template<typename Profile> 
dds_status_t AD9910::dds_freq_out(uint32_t f_out, int16_t ampl_db){

    if (!is_initialized()) return dds_status_t::DDS_NOT_INITIALIZED;
    if (f_out == 0) return dds_status_t::DDS_INVALID_PARAM;
    dds_status_t s = dds_status_t::DDS_OK;

    // 1. RealDDSCoreClock=CalcRealDDSCoreClockFromOffset(); // 🤔 Check me 

    // 2. calculate ftw
    const uint32_t ftw = dds_freq_ftw(f_out);  // 🤔 Check me 
    // 3. pow -> 0
    // 4. calculate amplitude conversion -> asf
    const uint16_t asf = calc_ampl_scale_factor(ampl_db);

    auto r = PROFILE::single_tone(ftw,0,asf);
    auto b = PROFILE::bytes();
    TRY_OK( dds_reg_write(PROFILE::address, b.data(), b.size()) ,s,s);
    TRY_OK( dds_update_io_pulse()  ,s,s);
    TRY_OK( set_profile<PROFILE>() ,s,s);
    TRY_OK( dds_update_io_pulse()  ,s,s);
    return s;
}


// ----------------------------------------
//               ✅ Checked 
// ----------------------------------------

// ✅ Checked 
dds_status_t AD9910::dds_digital_ramp(  uint32_t ftw_start,
                                        uint32_t ftw_end,
                                        uint32_t ftw_step,
                                        uint16_t step_rate,
                                        bool continuous){

    if (!is_initialized()) return dds_status_t::DDS_NOT_INITIALIZED;
    dds_status_t s = dds_status_t::DDS_OK;

    // 1) Program PROFILE0 amplitude (ASF)
    uint16_t asf = calc_ampl_scale_factor(0);   //Amplitude_dB=0

    auto r = ad9910_reg::PROFILE0::single_tone(0,0,asf); 
    auto b = ad9910_reg::PROFILE0::bytes();
    TRY_OK( dds_reg_write(ad9910_reg::PROFILE0::address, b.data(), b.size()) ,s,s)

    // Select profile 0 on the pins (OG: PROFILE0/1/2 = 0)
    TRY_OK( pin_write(idx(DdsPin::PROFILE0), HWAbstraction::HW_PIN_LOW),    s, s);
    TRY_OK( pin_write(idx(DdsPin::PROFILE1), HWAbstraction::HW_PIN_LOW),    s, s);
    TRY_OK( pin_write(idx(DdsPin::PROFILE2), HWAbstraction::HW_PIN_LOW),    s, s);

    // 2) CFR1: autoclear digital ramp accumulator, SDIO input-only
    TRY_OK( dds_cfr1_drg_setup() ,s,s);                                              

    // 3) DR_LIMIT: upper = ftw_end, lower = ftw_start 
    TRY_OK( dds_drg_set_limit(ftw_start, ftw_end) ,s,s);

    // 4) DR_STEP: neg = ftw_step, pos = ftw_step (8 bytes)
    TRY_OK( dds_drg_set_step(ftw_step) ,s,s);                                            

    // 5) DR_RATE: neg = step_rate, pos = step_rate (4 bytes)
    TRY_OK( dds_drg_set_rate(step_rate) ,s,s);

    // 6) CFR2: enable DRG on frequency destination
    TRY_OK( dds_drg_enable_freq(drg_continuous_) ,s,s);

    // 7) DRCTL high (start DRG), IO_UPDATE
    TRY_OK( hw_.hw_pin_mode(idx(DdsPin::DRCTL), pin_mode_t::PIN_OUTPUT), s,s);
    TRY_OK( pin_write(idx(DdsPin::DRCTL), HWAbstraction::HW_PIN_HIGH) ,s,s);
    return dds_update_io_pulse();
}

// ✅ Checked 
dds_status_t AD9910::dds_restart_drg() {
    if (!is_initialized()) return dds_status_t::DDS_NOT_INITIALIZED;
    dds_status_t s = dds_drg_enable_freq(false); // disable continuous mode
    if (s != dds_status_t::DDS_OK) return s;
    return dds_update_io_pulse();
}

// ✅ Checked 
dds_status_t AD9910::dds_update_io_pulse() {
    dds_status_t s = dds_status_t::DDS_OK;
    TRY_OK( pin_write(idx(DdsPin::IO_UPDATE), HWAbstraction::HW_PIN_LOW)       ,s,s);
    hw().hw_delay_us(10);
    TRY_OK(     pin_write(idx(DdsPin::IO_UPDATE), HWAbstraction::HW_PIN_HIGH)  ,s,s);
    hw().hw_delay_us(10);
    TRY_OK( pin_write(idx(DdsPin::IO_UPDATE), HWAbstraction::HW_PIN_LOW)       ,s,s);
    return s;
}




// MAYBE SOMEDAY
// void freq_out_profile(uint8_t prof_num, uint32_t ftw, uint16_t pow, uint16_t asf)
// {
//     switch(prof_num)
//     {
//         case 0: return freq_out<ad9910_reg::PROFILE0>(ftw, pow, asf);
//         case 1: return freq_out<ad9910_reg::PROFILE1>(ftw, pow, asf);
//         case 2: return freq_out<ad9910_reg::PROFILE2>(ftw, pow, asf);
//         case 3: return freq_out<ad9910_reg::PROFILE3>(ftw, pow, asf);
//         case 4: return freq_out<ad9910_reg::PROFILE4>(ftw, pow, asf);
//         case 5: return freq_out<ad9910_reg::PROFILE5>(ftw, pow, asf);
//         case 6: return freq_out<ad9910_reg::PROFILE6>(ftw, pow, asf);
//         case 7: return freq_out<ad9910_reg::PROFILE7>(ftw, pow, asf);
//     }


// NOT SURE NEEDED 
void AD9910::calc_best_step_rate(uint16_t& step,
                                 uint64_t& step_rate,
                                 uint32_t f_mod_hz) const{
    // Guard invalid inputs
    const uint64_t sysclk = dds_sysclk_hz();
    if (step == 0 || f_mod_hz == 0 || sysclk == 0) {
        step_rate = 0; step = 0; return; }

    // ---- OG formula, directly matched ----
    // T_step      = 1 / (F_mod * Step)
    // fStep_Rate  = (T_step * sysclk) / 4
    // Step_Rate   = ceil(fStep_Rate)
    // Step        = (sysclk) / (4 * Step_Rate * F_mod)
    // ---------------------------------------

    long double t_step =
        1.0L / (static_cast<long double>(f_mod_hz) *
                static_cast<long double>(step));

    long double f_step_rate = (t_step * static_cast<long double>(sysclk)) / 4.0L;

    // ceil() like OG
    step_rate = static_cast<uint64_t>(std::ceil(f_step_rate));

    if (step_rate == 0) { step = 0; return;}

    long double eff_step =
        static_cast<long double>(sysclk) /
        (4.0L * static_cast<long double>(step_rate) *
               static_cast<long double>(f_mod_hz));

    // Clamp to uint16_t range
    if (eff_step <= 0.0L) {
        step = 0;
    } else if (eff_step > 65535.0L) {
        step = 65535;
    } else {
        step = static_cast<uint16_t>(eff_step);
    }
}

// Another verion
bool AD9910::calc_best_step_rate(uint16_t& step,
                                 uint64_t& step_rate,
                                 uint32_t f_mod_hz) const
{
    const uint64_t sysclk = dds_sysclk_hz();
    if (step == 0 || f_mod_hz == 0 || sysclk == 0) {
        step_rate = 0;
        step      = 0;
        return false;
    }

    // T_step = 1 / (F_mod * Step)
    const double t_step =
        1.0 / (static_cast<double>(f_mod_hz) *
               static_cast<double>(step));

    // fStep_Rate = (T_step * sysclk) / 4
    const double f_step_rate =
        (t_step * static_cast<double>(sysclk)) / 4.0;

    if (f_step_rate <= 0.0) {
        step_rate = 0;
        step      = 0;
        return false;
    }

    step_rate = static_cast<uint64_t>(std::ceil(f_step_rate));

    // if HW step_rate is e.g. 16-bit, clamp here:
    // if (step_rate > 0xFFFFu) step_rate = 0xFFFFu;

    // Step = sysclk / (4 * Step_Rate * F_mod)
    const double eff_step =
        static_cast<double>(sysclk) /
        (4.0 * static_cast<double>(step_rate) *
               static_cast<double>(f_mod_hz));

    if (eff_step <= 0.0) {
        step_rate = 0;
        step      = 0;
        return false;
    }

    double s = eff_step;
    if (s > 65535.0) s = 65535.0;
    step = static_cast<uint16_t>(s);

    return true;
}




