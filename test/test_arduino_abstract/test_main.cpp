#include "boards/arduino/models/board_arduino_due.h"

#include <unity.h>


ArduinoDue due;


// --- TEST FUNCTIONS ---
void test_due_begin_returns_ok(void) {
    HWAbstraction::hw_status_t status = due.begin();
    TEST_ASSERT_EQUAL(HWAbstraction::HW_OK, status);
}

void test_due_spi_config(void) {
    HWAbstraction::spi_config_t cfg = {8000000, 0, 0, 0x00};
    TEST_ASSERT_EQUAL(HWAbstraction::HW_OK, due.set_spi_config(cfg));
}


// --- SETUP / LOOP ---
void setup() {
    due.hw_delay_ms(2000); // let serial settle
    UNITY_BEGIN();
    RUN_TEST(test_due_begin_returns_ok);
    RUN_TEST(test_due_spi_config);
    UNITY_END();
}

void loop() {
}


