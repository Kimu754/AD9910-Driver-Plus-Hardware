# pragma once
#include <cstdint>
#include <cstddef>
#include <array>
#include <type_traits>

// ✅ TESTED : test > test_registers


// ----------------------------------------
//               FIELD
// ----------------------------------------
// Defines a compile-time bitfield located within one byte, using BIT and WIDTH to derive mask and position.
// Provides read, write, and reset helpers that operate on a 64-bit register value with zero runtime computation
template<unsigned BIT, uint8_t WIDTH = 1>
struct Field {

    static_assert(  BIT < 64,                             "max 64 bits total"             ); // Register size
    static_assert(  WIDTH >= 1 && WIDTH <= 8,             "field width 1..8"              ); // Field size 
    static_assert(  (BIT / 8) == ((BIT + WIDTH - 1) / 8), "field must not cross a byte"   ); // Field within a byte   

    // Computed once at compile-time; no runtime recomputation.
    static constexpr uint8_t BYTE_INDEX = BIT / 8;
    static constexpr uint8_t SHIFT      = BIT % 8;
    static constexpr uint8_t BYTE_MASK  = uint8_t(((1u << WIDTH) - 1u) << SHIFT);
    static constexpr uint64_t MASK64 = uint64_t(BYTE_MASK) << (BYTE_INDEX * 8u);

    // read/write use these precomputed constants
    static uint64_t clear(uint64_t v) { return v & ~MASK64;}
    static uint8_t extract(uint64_t v) { return uint8_t((v & MASK64) >> (BYTE_INDEX * 8u));}
    static uint64_t insert(uint8_t x, uint64_t v) {
        v &= ~MASK64;
        v |= uint64_t(x & BYTE_MASK) << (BYTE_INDEX * 8u);
        return v;
    }    

};


// ----------------------------------------
//               PACK_BYTES
// ----------------------------------------
// It takes a 64-bit integer (v) and outputs a byte array of length N, 
// arranged in big-endian order
// Example :   IN : unit64 = byte7 byte6 ... byte1 byte 0  , Len = 4
// out[0] = byte 3 (MSB)
// out[1] = byte 2
// out[2] = byte 1
// out[3] = byte 0 (LSB)
// Used mainly in ReValue
template<size_t N>
std::array<uint8_t, N> pack_be(uint64_t v) {
    std::array<uint8_t, N> out{};
    for (size_t i = 0; i < N; i++) {
        size_t shift = (N - 1 - i) * 8;
        out[i] = static_cast<uint8_t>((v >> shift) & 0xFFu);
    }
    return out;
}

// ----------------------------------------
//               REGVALUE
// ----------------------------------------
// --- Helper To deal with different register adresses lengths
// Stores a register value in a 64-bit integer, 
// Masks it to the correct width, 
template<size_t LEN_BYTES>
struct RegValue {
    static_assert(LEN_BYTES >= 1 && LEN_BYTES <= 8, "LEN_BYTES must be 1..8"); 
    using storage_t = uint64_t; // Container for the bits
    static constexpr storage_t REG_MASK =
        (LEN_BYTES == 8)
            ? 0xFFFFFFFFFFFFFFFFull
            : ((storage_t(1) << (LEN_BYTES * 8)) - 1ull);

    storage_t val = 0;
    RegValue() = default;
    explicit RegValue(storage_t raw_val) : val(raw_val & REG_MASK) {}

};

// ----------------------------------------
//              Register
// ----------------------------------------
// Template representing a hardware register at a specific address with a defined length in bytes.
template<std::uintptr_t Addr, std::size_t LEN_BYTES>
struct Register {
    static constexpr std::uintptr_t address = Addr;
    using value_type = RegValue<LEN_BYTES>;
    static value_type shadow;

    template<typename FieldT>
    static void clear(value_type &v = shadow) {
        v.val = FieldT::clear(v.val) & value_type::REG_MASK;
    }

    template<typename FieldT>
    static uint8_t extract(const value_type &v = shadow) {
        return FieldT::extract(v.val);
    }

    template<typename FieldT>
    static void insert(uint8_t x , value_type &v = shadow) {
        v.val = FieldT::insert(x, v.val) & value_type::REG_MASK;
    }

    // Use for single-bit fields
    template<typename FieldT>
    static void set( bool on, value_type &v = shadow) {
        insert<FieldT>(on ? 0xFF : 0,v);
    }

    static std::array<uint8_t, LEN_BYTES> bytes(const value_type &v = shadow) {
        return pack_be<LEN_BYTES>(v.val & value_type::REG_MASK);
    }

};

template<std::uintptr_t Addr, std::size_t LEN_BYTES>
typename Register<Addr, LEN_BYTES>::value_type
Register<Addr, LEN_BYTES>::shadow{};


