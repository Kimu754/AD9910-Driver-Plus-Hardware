#include <Arduino.h>
#include <SPI.h>
#include <unity.h>

// ---- helpers ----
void wait_usb() {
  // Due needs this for native USB
  while (!Serial) { /* wait */ }
  delay(200);
}

// ---- tests ----
void test_spi_init() {
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  // If we got here, init did not hang
  TEST_PASS();
}

void test_spi_transfer_runs_quickly() {
  // With no slave, data is undefined. We only assert timing.
  const uint16_t N = 200;  // number of bytes to transfer
  uint32_t t0 = micros();
  for (uint16_t i = 0; i < N; i++) {
    (void)SPI.transfer((uint8_t)i);
  }
  uint32_t dt = micros() - t0;

  // Loose upper bound so it’s robust on different boards/builds.
  // Expect well under 200 ms for 200 transfers at 1 MHz.
  TEST_ASSERT_MESSAGE(dt < 200000UL, "SPI transfers too slow or stalled");
}

void test_spi_cleanup() {
  SPI.endTransaction();
  SPI.end();
  TEST_PASS();
}

