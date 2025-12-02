#include "dds/ad9910/ad9910_context.h"

// This Context mirrors the parameters from GRA & AFCH
AD9910Context ctx;
ctx.ref_clk_hz       = 100000000;          // Ref_Clk
ctx.pll_enable       = true;               // PLL on
ctx.pll_mult         = 20;                 // round(1e9 / 1e8) * 2
ctx.dac_high_current = false;              // DACCurrentIndex == 0


// Expected SPI config FOR 
HWAbstraction::spi_config_t{
    2000000u,   // spi_clock_hz (DIV8 at 16 MHz)
    0,          // bit_order: 0 = MSB first
    0,          // mode: SPI mode 0
    0x00        // read_dummy
}


// TODO REST ...