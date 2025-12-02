/*                                        *******************************************
 *                                        *             GRA-AFCH.COM                *
 *                                        *******************************************                                      
 */

#ifndef __MAIN_H
#define __MAIN_H

#include "common.h"
#include "display.h"
#include "ad9910.h"
#include "menuclk.h"
#include <EEPROM.h>
#include <ClickButton.h>

#define DBG 0 //debug messsages 0 - disabled, 1 - enabled

#define MODE_PIN    2
#define RFOUT_PIN   3
#define A_PIN       18
#define B_PIN       19

#define CLOCK_FROM_INTERNAL_TCXO    0
#define CLOCK_FROM_EXTERANL_SRC     1
#define TCXO_POWER_PIN              A2 //REF_LED
#define TCXO_PATH_PIN               A0 //V1
#define EXTERANL_SRC_PATH_PIN       A1 //V2

#define TRG_IN_PIN 48
#define TRG_LED_PIN A4


extern ClickButton modeButton;

extern int M, K, H, A, MenuPos;

extern void selectClockSrcPath(uint8_t path);
void LoadMainSettings();
float getInputVoltage();

void getAmpFromPC();
void outputBufferedFrequencies();
void getDataFromPC(uint32_t* data_array);
void replyToPC();
void sendFinishtoPC();
void setFrequency(uint32_t fq);
void SaveMainSettings();

#endif