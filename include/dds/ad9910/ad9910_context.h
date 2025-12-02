// ad9910_context.h
#pragma once 
#include "core_types.h"

struct AD9910Context{
    uint32_t ref_clk_hz         ;   // REFCLK input of the AD9910
    bool     pll_enable         ;   // PLL
    uint16_t pll_mult           ;    
    bool     dac_high_current   ;   // false -> 0x7F (normal), true -> 0xFF (high current)
    bool     allow_overclock    ;   // false = limit SYSCLK to 1.0 GHz, true = allow up to ~1.52 GHz

};


//
// AD9910Context ctx{};
// ctx.ref_clk_hz      = 50'000'000;
// ctx.pll_enable      = true;
// ctx.pll_mult        = 20;
// ctx.dac_high_current= true;
// ctx.allow_overclock = true;  