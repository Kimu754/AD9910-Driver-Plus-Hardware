#include <Arduino.h>
#include <SPI.h>
#include <unity.h>
#include "functions_spi.h"


// ---- Arduino entrypoints ----
void setUp() { /* not used */ }
void tearDown() { /* not used */ }

void setup() {
  Serial.begin(115200);
  wait_usb();

  UNITY_BEGIN();
  RUN_TEST(test_spi_init);
  RUN_TEST(test_spi_transfer_runs_quickly);
  RUN_TEST(test_spi_cleanup);
  UNITY_END();
}

void loop() {
  // not used by Unity
}
