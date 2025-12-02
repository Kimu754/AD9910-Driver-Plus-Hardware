#include <unity.h>
#include <registers.h>  
#include <array>    

// Command 
// pio test -e native -f test_registers

// ----------------------------------------
//               FIELD
// ----------------------------------------

// --- TEST : BYTE_INDEX
void test_field_byte_index(){
    using F = Field<62,2>;
    TEST_ASSERT_EQUAL_UINT8(7, F::BYTE_INDEX);
}

// --- TEST : SHIFT
void test_field_shift(){
    using F = Field<10,3>;
    TEST_ASSERT_EQUAL_UINT8(2, F::SHIFT);
}   

// --- TEST ; BYTE_MASK
void test_field_byte_mask() {
    using F = Field<10,3>;
    TEST_ASSERT_EQUAL_UINT8(0x1C, F::BYTE_MASK);
}

// --- TEST : MASK64
void test_field_mask64(){
    using F0 = Field<0,4>;
    TEST_ASSERT_EQUAL_UINT64(0x000000000000000Full, F0::MASK64);
    using F1 = Field<4,4>;
    TEST_ASSERT_EQUAL_UINT64(0x00000000000000F0ull, F1::MASK64);
    using F2 = Field<8,4>;
    TEST_ASSERT_EQUAL_UINT64(0x0000000000000F00ull, F2::MASK64);
    using F3 = Field<12,4>;
    TEST_ASSERT_EQUAL_UINT64(0x000000000000F000ull, F3::MASK64);
    using F4 = Field<16,4>;
    TEST_ASSERT_EQUAL_UINT64(0x00000000000F0000ull, F4::MASK64);
    using F5 = Field<20,4>;
    TEST_ASSERT_EQUAL_UINT64(0x0000000000F00000ull, F5::MASK64);
    using F6 = Field<24,4>;
    TEST_ASSERT_EQUAL_UINT64(0x000000000F000000ull, F6::MASK64);
    using F15 = Field<60,4>;
    TEST_ASSERT_EQUAL_UINT64(0xF000000000000000ull, F15::MASK64);

}

// --- TEST : reset field 
void test_field_clear() {
    using F = Field<12,4>;
    uint64_t v = 0xFFFFFFFFFFFFFFFFull;
    v = F::clear(v);
    TEST_ASSERT_EQUAL_UINT64(0xFFFFFFFFFFFF0FFFull, v);
}

// --- TEST : insert field
void test_field_insert() {
    using F = Field<16,6>;      // byte 2, 0X3F mask, shift 0 

    uint64_t v0 = 0;            // value to insert into  
    uint64_t v1 = 0xFFFFFFFFFFFFFFFFull;

    uint8_t  x  = 0xF5;         // value to insert
    uint8_t  y  = 0x15;         // value to insert
    uint8_t  z  = 0x05;         // value to insert

    v0 = F::insert(x, v0);      // 0x3F & 0xF5 = 0x35
    TEST_ASSERT_EQUAL_UINT64(0x0000000000350000ull, v0);

    v0 = F::insert(y, v0);      // 0x3F & 0x15 = 0x15
    TEST_ASSERT_EQUAL_UINT64(0x0000000000150000ull, v0);

    v1 = F::insert(0x05, v1);   //0x3F & 0x05 = 0x05 + upper bits stay 1 (0xC5)
    TEST_ASSERT_EQUAL_UINT64(0xFFFFFFFFFFC5FFFFull, v1);
}


// --- TEST : extract field
void test_field_extract() {
    using F = Field<16,6>;   // byte 2, 0X3F mask, shift 0 

    // expected from insert(0, 0xF5) -> 0x35 in the field
    uint64_t v0 = 0x0000000000F50000ull;            // Ex. Register value
    TEST_ASSERT_EQUAL_UINT8(0x35, F::extract(v0));  // Field holds 0x35

    // expected from insert(0xFFFFFFFFFFFFFFFF, 0x05)
    // field holds 0x05, upper bits in that byte are 1 (0xC5)
    uint64_t v1 = 0xFFFFFFFFFFC5FFFFull;            // Ex. Register value
    TEST_ASSERT_EQUAL_UINT8(0x05, F::extract(v1));  // Field holds 0x05
}



// // ----------------------------------------
// //                REGVALUE
// // ----------------------------------------

// // --- Helper 
template<std::size_t N>
static void assert_array_eq(const std::array<std::uint8_t, N>& got,
                            const std::uint8_t (&exp)[N]){
    for (std::size_t i = 0; i < N; ++i) {
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(exp[i], got[i], "byte mismatch");
    }
}


// // ----------------- pack_be tests -----------------

static void test_pack_be_1byte_basic(void)
{
    {   // Zero Case 
        std::uint8_t exp[1] = { 0x00 };
        auto a = pack_be<1>(0x0000000000000000ull);
        assert_array_eq(a, exp);
    }
    {
        std::uint8_t exp[1] = { 0xFF };
        auto a = pack_be<1>(0xFFABFF45FFCCFFFFull);
        assert_array_eq(a, exp);
    }
    {
        std::uint8_t exp[1] = { 0xAB };
        auto a = pack_be<1>(0x00000000000000ABull);
        assert_array_eq(a, exp);
    }
}

static void test_pack_be_2byte_patterns(void)
{
    {
        std::uint8_t exp[2] = { 0x00, 0x00 };
        auto a = pack_be<2>(0x0000000000000000ull);
        assert_array_eq(a, exp);
    }
    {
        std::uint8_t exp[2] = { 0xFF, 0xFF };
        auto a = pack_be<2>(0x000000000000FFFFull);
        assert_array_eq(a, exp);
    }
    {
        std::uint8_t exp[2] = { 0x12, 0x34 };
        auto a = pack_be<2>(0x0000000000001234ull);
        assert_array_eq(a, exp);
    }
    {
        // higher bits must not affect result
        std::uint8_t exp[2] = { 0x12, 0x34 };
        auto a = pack_be<2>(0xDEADBEAF00001234ull);
        assert_array_eq(a, exp);
    }
}

static void test_pack_be_3byte_misaligned(void)
{
    {
        std::uint8_t exp[3] = { 0x00, 0x00, 0x01 };
        auto a = pack_be<3>(0x0000000000000001ull);
        assert_array_eq(a, exp);
    }
    {
        std::uint8_t exp[3] = { 0x12, 0x34, 0x56 };
        auto a = pack_be<3>(0x0000000000123456ull);
        assert_array_eq(a, exp);
    }
    {
        std::uint8_t exp[3] = { 0xAA, 0xBB, 0xCC };
        auto a = pack_be<3>(0x1122AABBCCull);  // extra high bits
        assert_array_eq(a, exp);
    }
}

static void test_pack_be_8byte_full(void)
{
    {
        std::uint8_t exp[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
        auto a = pack_be<8>(0x0000000000000000ull);
        assert_array_eq(a, exp);
    }
    {
        std::uint8_t exp[8] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
        auto a = pack_be<8>(0xFFFFFFFFFFFFFFFFull);
        assert_array_eq(a, exp);
    }
    {
        std::uint8_t exp[8] = { 0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF };
        auto a = pack_be<8>(0x0123456789ABCDEFull);
        assert_array_eq(a, exp);
    }
}

// // ----------------- RegValue tests -----------------
// --- Helpers 
// Example register + value: 4 bytes
using Reg1 = Register<0x00, 1>;
using Reg2 = Register<0x00, 2>;
using Reg3 = Register<0x00, 3>;
using Reg4 = Register<0x00, 4>;
using Reg5 = Register<0x00, 5>;
using Reg8 = Register<0x00, 8>;
using Reg9 = Register<0x00, 8>;
using Reg10= Register<0x00, 8>;

using F_BIT0   = Field<0>;      // single bit at bit 0
using F_MODE   = Field<8,4>;    // 4-bit field at byte 1
using F_BYTE2  = Field<16,8>;   // full byte at bit 16 (byte index 2)


// ----------------- RegValue / Register::bytes tests -----------------
static void test_regvalue_1byte_mask_and_bytes(void)
{
    RegValue<1> r;

    r.val = 0x0000000000000000ull;
    {
        std::uint8_t exp[1] = { 0x00 };
        auto b = Reg1::bytes(r);
        assert_array_eq(b, exp);
    }

    r.val = 0xFFFFFFFFFFFFFFFFull;   // check mask cuts to lowest 8 bits
    {
        std::uint8_t exp[1] = { 0xFF };
        auto b = Reg1::bytes(r);
        assert_array_eq(b, exp);
    }

    r.val = 0x1234567890AB00CDull;
    {
        std::uint8_t exp[1] = { 0xCD };  // LSB
        auto b = Reg1::bytes(r);
        assert_array_eq(b, exp);
    }
}

static void test_regvalue_2byte_mask_and_bytes(void)
{
    RegValue<2> r;

    r.val = 0x0000000000000000ull;
    {
        std::uint8_t exp[2] = { 0x00, 0x00 };
        auto b = Reg2::bytes(r);
        assert_array_eq(b, exp);
    }

    r.val = 0xFFFFFFFFFFFFFFFFull;
    {
        std::uint8_t exp[2] = { 0xFF, 0xFF };
        auto b = Reg2::bytes(r);
        assert_array_eq(b, exp);
    }

    r.val = 0x1234567890AB1234ull;
    {
        // only lowest 16 bits (0x1234) should appear
        std::uint8_t exp[2] = { 0x12, 0x34 };
        auto b = Reg2::bytes(r);
        assert_array_eq(b, exp);
    }
}

static void test_regvalue_3byte_mask_and_bytes(void)
{
    RegValue<3> r;

    r.val = 0x0000000000000000ull;
    {
        std::uint8_t exp[3] = { 0x00, 0x00, 0x00 };
        auto b = Reg3::bytes(r);
        assert_array_eq(b, exp);
    }

    r.val = 0xFFFFFFFFFFFFFFFFull;
    {
        std::uint8_t exp[3] = { 0xFF, 0xFF, 0xFF };
        auto b = Reg3::bytes(r);
        assert_array_eq(b, exp);
    }

    r.val = 0xAA11223344ull;  // high bits must be masked off
    {
        // REG_MASK = 0x00FFFFFF -> v & mask = 0x00223344
        std::uint8_t exp[3] = { 0x22, 0x33, 0x44 };
        auto b = Reg3::bytes(r);
        assert_array_eq(b, exp);
    }
}

static void test_regvalue_8byte_full_width(void)
{
    RegValue<8> r;

    r.val = 0x0000000000000000ull;
    {
        std::uint8_t exp[8] = { 0,0,0,0,0,0,0,0 };
        auto b = Reg8::bytes(r);
        assert_array_eq(b, exp);
    }

    r.val = 0xFFFFFFFFFFFFFFFFull;
    {
        std::uint8_t exp[8] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
        auto b = Reg8::bytes(r);
        assert_array_eq(b, exp);
    }

    r.val = 0x0123456789ABCDEFull;
    {
        std::uint8_t exp[8] = { 0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF };
        auto b = Reg8::bytes(r);
        assert_array_eq(b, exp);
    }
}

static void test_regvalue_various_lengths_random(void)
{
    const std::uint64_t val = 0xDEADBEEF11223344ull;

    {
        RegValue<1> r;
        r.val = val;
        std::uint8_t exp[1] = { 0x44 };
        auto b = Reg1::bytes(r);
        assert_array_eq(b, exp);
    }
    {
        RegValue<2> r;
        r.val = val;
        std::uint8_t exp[2] = { 0x33, 0x44 };
        auto b = Reg2::bytes(r);
        assert_array_eq(b, exp);
    }
    {
        RegValue<4> r;
        r.val = val;
        std::uint8_t exp[4] = { 0x11, 0x22, 0x33, 0x44 };
        auto b = Reg4::bytes(r);
        assert_array_eq(b, exp);
    }
    {
        RegValue<5> r;
        r.val = val;
        // REG_MASK = 0x000000FFFFFFFFFF, v & mask = 0x000000EF11223344
        std::uint8_t exp[5] = { 0xEF, 0x11, 0x22, 0x33, 0x44 };
        auto b = Reg5::bytes(r);
        assert_array_eq(b, exp);
    }
}


// --- TEST: Register::insert/extract<FieldT> via RV4
void test_regvalue_field_extract_insert(void)
{
    // set bit0 = 1  => raw pattern 0b00000001
    Reg9::insert<F_BIT0>(0xF1);
    TEST_ASSERT_EQUAL_UINT8(0x01, Reg9::extract<F_BIT0>());
    TEST_ASSERT_EQUAL_UINT64(0x0000000000000001ull, Reg9::shadow.val);

    Reg9::insert<F_MODE>(0x08);
    TEST_ASSERT_EQUAL_UINT8(0x01, Reg9::extract<F_BIT0>());
    TEST_ASSERT_EQUAL_UINT8(0x08, Reg9::extract<F_MODE>());
    TEST_ASSERT_EQUAL_UINT64(0x0000000000000801ull, Reg9::shadow.val);

    Reg9::insert<F_BYTE2>(0xAB);
    TEST_ASSERT_EQUAL_UINT8(0xAB, Reg9::extract<F_BYTE2>());
    TEST_ASSERT_EQUAL_UINT64(0x0000000000AB0801ull, Reg9::shadow.val);
}


// --- TEST: Register::clear<FieldT> via RV4
void test_regvalue_field_clear(void)
{
    // set fields using raw bit patterns
    Reg10::insert<F_BIT0>(0x01);   
    Reg10::insert<F_MODE>(0x0C);    
    Reg10::insert<F_BYTE2>(0xAA); 

    TEST_ASSERT_EQUAL_UINT8(0x01, Reg10::extract<F_BIT0>());
    TEST_ASSERT_EQUAL_UINT8(0x0C, Reg10::extract<F_MODE>());
    TEST_ASSERT_EQUAL_UINT8(0xAA, Reg10::extract<F_BYTE2>());

    Reg10::clear<F_MODE>();

    TEST_ASSERT_EQUAL_UINT8(0x01, Reg10::extract<F_BIT0>());   // unchanged
    TEST_ASSERT_EQUAL_UINT8(0x00, Reg10::extract<F_MODE>());   // cleared
    TEST_ASSERT_EQUAL_UINT8(0xAA, Reg10::extract<F_BYTE2>());  // unchanged
}

void test_reg_value_set(){

    using RegA = Register<0x00, 2>; 
    using F_ENABLE = Field<3>; // single bit at bit 3

    RegA::set<F_ENABLE>(true);
    TEST_ASSERT_EQUAL_UINT16(0x0008, RegA::shadow.val);
    RegA::set<F_ENABLE>(false);
    TEST_ASSERT_EQUAL_UINT16(0x0000, RegA::shadow.val);
}

// ----------------------------------------
//                MAIN BODY
// ----------------------------------------
void setUp()   {}
void tearDown(){}

int main(int, char**) {
    UNITY_BEGIN();

    // --- FIELDS
    RUN_TEST(test_field_byte_index);
    RUN_TEST(test_field_shift);
    RUN_TEST(test_field_byte_mask);
    RUN_TEST(test_field_mask64);
    RUN_TEST(test_field_clear);
    RUN_TEST(test_field_insert);
    RUN_TEST(test_field_extract);

    // // --- PACK_BE
    RUN_TEST(test_pack_be_1byte_basic);
    RUN_TEST(test_pack_be_2byte_patterns);
    RUN_TEST(test_pack_be_3byte_misaligned);
    RUN_TEST(test_pack_be_8byte_full);

    // --- REGVALUE
    RUN_TEST(test_regvalue_1byte_mask_and_bytes);
    RUN_TEST(test_regvalue_2byte_mask_and_bytes);
    RUN_TEST(test_regvalue_3byte_mask_and_bytes);
    RUN_TEST(test_regvalue_8byte_full_width);
    RUN_TEST(test_regvalue_various_lengths_random);

    RUN_TEST(test_regvalue_field_extract_insert);
    RUN_TEST(test_regvalue_field_clear);
    RUN_TEST(test_reg_value_set);


    return UNITY_END();
}



