# include <registers.h>
# include "dds/ad9910/ad9910.h"
# include "dds/ad9910/ad9910_registers.h"


// ----------------------------------------
//                 CFR1
// ----------------------------------------
dds_status_t AD9910::dds_cfr1_defaults(){ 
    auto r      = ad9910_reg::CFR1::defaults();
    auto b      = ad9910_reg::CFR1::bytes(r);
    return dds_reg_write(ad9910_reg::CFR1::address, b.data(), b.size());
}

dds_status_t AD9910::dds_cfr1_drg_setup(){
    auto r      = ad9910_reg::CFR1::drg_setup();
    auto b      = ad9910_reg::CFR1::bytes(r);
    return dds_reg_write(ad9910_reg::CFR1::address, b.data(), b.size());
}

dds_status_t AD9910::dds_cfr1_drg_basic(){
    auto r      = ad9910_reg::CFR1::drg_basic();
    auto b      = ad9910_reg::CFR1::bytes(r);
    return dds_reg_write(ad9910_reg::CFR1::address, b.data(), b.size());
}


// ----------------------------------------
//                 CFR2
// ----------------------------------------
dds_status_t AD9910::dds_cfr2_defaults(){
    auto r      = ad9910_reg::CFR2::defaults();
    auto b      = ad9910_reg::CFR2::bytes(r);
    return dds_reg_write(ad9910_reg::CFR2::address, b.data(), b.size());
}

dds_status_t AD9910::dds_cfr2_drg_freq_enable(bool continuous){
    auto r      = ad9910_reg::CFR2::drg_freq_enable(continuous);
    auto b      = ad9910_reg::CFR2::bytes(r);
    return dds_reg_write(ad9910_reg::CFR2::address, b.data(), b.size());
}

// ----------------------------------------
//                 CFR3
// ----------------------------------------
// --- Specfic register writers ----
dds_status_t AD9910::dds_cfr3_defaults( bool ref_div2       ,
                                        bool pll_enable     , 
                                        uint8_t pll_mult    ,
                                        VcoSel vco_sel  = VcoSel::VCO5  ,  // GRA & AFCH default 
                                        IcpCode icp     = IcpCode::u387 ){ // GRA & AFCH default

    auto r = pll_enable
                ? ad9910_reg::CFR3::default_pll_on (ref_div2,pll_mult,vco_sel,icp)                  
                : ad9910_reg::CFR3::default_pll_off(ref_div2);
    auto bytes = ad9910_reg::CFR3::bytes(r);
    return dds_reg_write(ad9910_reg::CFR3::address, bytes.data(), bytes.size());
}


// ----------------------------------------
//                  AUX
// ----------------------------------------
dds_status_t AD9910::dds_aux_dac_fsc(bool high_current) {
    auto r      = ad9910_reg::AUX_DAC::defaults(high_current);
    auto b      = ad9910_reg::AUX_DAC::bytes(r);
    return dds_reg_write(ad9910_reg::AUX_DAC::address, b.data(), b.size());
}

// ----------------------------------------
//                 DR_LIMIT
// ----------------------------------------
dds_status_t AD9910::dds_drg_set_limit(uint32_t lower, uint32_t upper){
    auto r      = ad9910_reg::DR_LIMIT::set_limit(lower, upper);
    auto b      = ad9910_reg::DR_LIMIT::bytes(r);
    return dds_reg_write(ad9910_reg::DR_LIMIT::address, b.data(), b.size());
}

// ----------------------------------------
//                 DR_STEP
// ----------------------------------------
dds_status_t AD9910::dds_drg_set_step(uint32_t incr , uint32_t decr){
    auto r      = ad9910_reg::DR_STEP::set_step(incr, decr);
    auto b      = ad9910_reg::DR_STEP::bytes(r);
    return dds_reg_write(ad9910_reg::DR_STEP::address, b.data(), b.size());
}

// ----------------------------------------
//                 DR_RATE
// ----------------------------------------
dds_status_t AD9910::dds_drg_set_rate(uint16_t pos, uint16_t neg){
    auto r      = ad9910_reg::DR_RATE::set_rate(pos, neg);
    auto b      = ad9910_reg::DR_RATE::bytes(r);
    return dds_reg_write(ad9910_reg::DR_RATE::address, b.data(), b.size());
}

// ----------------------------------------
//               🛠️ Doing 
// ----------------------------------------

namespace {
    // pack FTW range for DR_LIMIT (0x0B): [upper][lower], each 32-bit BE
    inline std::array<uint8_t, 8>
    make_drg_limit_payload(uint32_t ftw_start, uint32_t ftw_end)
    {
        auto up = pack_be<4>(ftw_end);
        auto lo = pack_be<4>(ftw_start);

        std::array<uint8_t, 8> buf{};
        buf[0] = up[0]; buf[1] = up[1]; buf[2] = up[2]; buf[3] = up[3];
        buf[4] = lo[0]; buf[5] = lo[1]; buf[6] = lo[2]; buf[7] = lo[3];
        return buf;
    }

    // pack FTW step size for DR_STEP (0x0C): [dec][inc], both 32-bit BE
    inline std::array<uint8_t, 8>
    make_drg_step_payload(uint32_t ftw_step)
    {
        auto step = pack_be<4>(ftw_step);
        std::array<uint8_t, 8> buf{};
        // negative slope
        buf[0] = step[0]; buf[1] = step[1]; buf[2] = step[2]; buf[3] = step[3];
        // positive slope
        buf[4] = step[0]; buf[5] = step[1]; buf[6] = step[2]; buf[7] = step[3];
        return buf;
    }

    // pack step rate for DR_RATE (0x0D): [neg_hi][neg_lo][pos_hi][pos_lo]
    inline std::array<uint8_t, 4>
    make_drg_rate_payload(uint16_t step_rate)
    {
        std::array<uint8_t, 4> buf{};
        uint8_t hi = static_cast<uint8_t>((step_rate >> 8) & 0xFF);
        uint8_t lo = static_cast<uint8_t>( step_rate       & 0xFF);
        buf[0] = hi; buf[1] = lo; // negative slope
        buf[2] = hi; buf[3] = lo; // positive slope
        return buf;
    }

} 


dds_status_t AD9910::dds_drg_program_range(uint32_t ftw_start,
                                           uint32_t ftw_end,
                                           uint32_t ftw_step,
                                           uint16_t step_rate)
{   
    dds_status_t s = dds_status_t::DDS_OK;

    // DR_LIMIT: upper = ftw_end, lower = ftw_start (8 bytes)
    std::uint64_t v =
        (static_cast<std::uint64_t>(ftw_end)  << 32) |
            static_cast<std::uint64_t>(ftw_start);

    auto b = pack_be<8>(v);
    TRY_OK( dds_reg_write(ad9910_reg::DR_LIMIT::address, b.data(), b.size())  ,s,s);
    
    // DR_STEP: neg = ftw_step, pos = ftw_step (8 bytes)
    std::uint64_t v =
        (static_cast<std::uint64_t>(ftw_step) << 32) |
            static_cast<std::uint64_t>(ftw_step);

    auto b = pack_be<8>(v);
    TRY_OK( dds_reg_write(ad9910_reg::DR_STEP::address, b.data(), b.size())  ,s,s);

    // DR_RATE: neg = step_rate, pos = step_rate (4 bytes)
    std::uint32_t v =
        (static_cast<std::uint32_t>(step_rate) << 16) |
            static_cast<std::uint32_t>(step_rate);

    auto b = pack_be<4>(v);
    return dds_reg_write(ad9910_reg::DR_RATE::address, b.data(), b.size());
}




