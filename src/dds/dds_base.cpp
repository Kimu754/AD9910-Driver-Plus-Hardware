# include "dds/dds_base.h"

const bool high   = ad9910_ctx_.dac_high_current;

// --- Hardware conversion helper ---
dds_status_t DDSBase::from_hw(HWAbstraction::hw_status_t hs) {
    return hs == HWAbstraction::HW_OK ? dds_status_t::DDS_OK : dds_status_t::DDS_HW_ERROR;}

// --- Sequence Run ---
dds_status_t DDSBase::dds_seq_run(const DdsSequence* seq, size_t count){
    for (size_t i = 0; i < count; ++i) {
        const auto& step = seq[i];

        if (step.pin_index >= pin_indices_.size())
            return dds_status_t::DDS_INVALID_PARAM;

        auto st = pin_write(step.pin_index, step.level);
        if (st != dds_status_t::DDS_OK)
            return st;

        if (step.delay_us)
            hw_.hw_delay_us(step.delay_us);
    }
    return dds_status_t::DDS_OK;
}


dds_status_t DDSBase::dds_setup_pins() {
    if (pin_indices_.empty())
        return dds_status_t::DDS_NOT_INITIALIZED;

    for (uint8_t ix : pin_indices_) {
        if (ix == PIN_U8_UNKNOWN)
            return dds_status_t::DDS_NOT_INITIALIZED;
    }

    size_t count = 0;
    const DdsSequence* seq = get_seq_setup(count);
    return dds_seq_run(seq, count);
}


// --- GPIO HAL-backed methods ---
dds_status_t DDSBase::pin_write(uint8_t pin_i, HWAbstraction::hw_pin_value_t val) {
    if (pin_i >= pin_indices_.size()) return dds_status_t::DDS_INVALID_PARAM;
    uint8_t idx = pin_indices_[pin_i];
    if (idx == PIN_U8_UNKNOWN) return dds_status_t::DDS_HW_ERROR;
    auto hs = hw_.hw_pin_write(idx, val);
    return from_hw(hs);
}

dds_status_t DDSBase::pin_read(uint8_t pin_i, HWAbstraction::hw_pin_value_t* val) {
    if (!val) return dds_status_t::DDS_INVALID_PARAM;
    if (pin_i >= pin_indices_.size()) return dds_status_t::DDS_INVALID_PARAM;

    uint8_t idx = pin_indices_[pin_i];
    if (idx == PIN_U8_UNKNOWN) return dds_status_t::DDS_HW_ERROR;

    auto hs = hw_.hw_pin_read(idx, val);
    return from_hw(hs);
    } 

// --- SPI HAL-backed methods --- 
dds_status_t DDSBase::spi_tx(const uint8_t* data, size_t len) {
    if (len == 0) return dds_status_t::DDS_OK;
    if (!data)     return dds_status_t::DDS_INVALID_PARAM;
    if (len > 0xFFFFu) return dds_status_t::DDS_INVALID_PARAM;
    auto hs = hw_.hw_spi_write(data, static_cast<uint16_t>(len));
    return from_hw(hs);
}

dds_status_t DDSBase::spi_rx(uint8_t* data, size_t len) {
    if (len == 0) return dds_status_t::DDS_OK;
    if (!data)     return dds_status_t::DDS_INVALID_PARAM;
    if (len > 0xFFFFu) return dds_status_t::DDS_INVALID_PARAM;
    auto hs = hw_.hw_spi_read(data, static_cast<uint16_t>(len));
    return from_hw(hs);
}

// --- Initialization methods ---
dds_status_t DDSBase::dds_attach_board(const pin_t* pins, size_t pins_count) {
    if (!pins || pins_count == 0) return dds_status_t::DDS_INVALID_PARAM;
    if (pins_count != pin_indices_.size()) return dds_status_t::DDS_INVALID_PARAM;

    for (size_t i = 0; i < pins_count; ++i) {
        const pin_t& p = pins[i];

        auto hs = hw_.hw_pin_attach(p);
        if (hs != HWAbstraction::HW_OK) return from_hw(hs);

        uint8_t idx = hw_.pin_to_index(p);
        if (idx == PIN_U8_UNKNOWN) return dds_status_t::DDS_HW_ERROR;

        pin_indices_[i] = idx;}

    return dds_status_t::DDS_OK;
}


dds_status_t DDSBase::spi_init() {
    const auto hs = hw_.hw_spi_init(spi_cfg_);
    return (hs == HWAbstraction::HW_OK) ? dds_status_t::DDS_OK
                                        : dds_status_t::DDS_HW_ERROR;
}

dds_status_t DDSBase::dds_init() {
    if (is_initialized()) return dds_status_t::DDS_OK; 
    if (auto s = dds_validate_context(); s != dds_status_t::DDS_OK) return s;   // Validate device-specific context  (device-specific)
    if (auto s = dds_attach_board(pins_, pins_count_); s != dds_status_t::DDS_OK) return s;  // Set Pins mode Outout/Input        (semi-generic : needs pins)
    if (auto s = dds_setup_pins(); s != dds_status_t::DDS_OK) return s;         // Set Pins level High/Low           (semi-generic : needs sequence )
    if (auto s = spi_init(); s != dds_status_t::DDS_OK) return s;               // Initialzes Boards SPI             (generic)
    if (auto s = dds_setup_reg(); s != dds_status_t::DDS_OK) return s;          // Setup the Boards Registers        (device-specific)
    set_initialized(true);
    return dds_status_t::DDS_OK;
}

