#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <array>

using std::size_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;   
using std::int8_t;     
using std::int16_t;    
using std::int32_t;    
using std::int64_t;    



#define TRY_OK(expr, status_var, return_value)   \
    do {                                                \
        if ((status_var) != dds_status_t::DDS_OK)       \
            return (return_value);                      \
        (status_var) = (expr);                          \
        if ((status_var) != dds_status_t::DDS_OK)       \
            return (return_value);                      \
    } while (0)