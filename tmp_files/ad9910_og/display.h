#ifndef __DISPLAY_H
#define DISPLAY_H

#include "common.h"

#include <Wire.h>
#include <Adafruit_SSD1306.h>

class Display {
public:
    Display() {
        this->display = Adafruit_SSD1306(128, 64, &Wire);
        this->display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        this->display.clearDisplay();
    }

    void displayHello() {
        this->display.clearDisplay();
        this->display.cp437(true);
        this->display.setTextSize(2);
        this->display.setTextColor(WHITE); // Draw white text
        this->display.setCursor(0, 0);
        this->display.print("Hint:");
        this->display.setTextSize(1);
        this->display.setCursor(62, 0);
        this->display.print("Firmware");
        this->display.setCursor(62, 8);
        this->display.print("ver.: ");
        this->display.print(FIRMWAREVERSION);
        this->display.setCursor(0, 16);

        this->display.setTextSize(2);
        this->display.setCursor(6, 16);
        this->display.print("Push");
        this->display.setCursor(58, 16);
        this->display.print("&");
        this->display.setCursor(74, 16);
        this->display.print("hold");
        this->display.setCursor(22, 33);
        this->display.print("ENCODER");
        this->display.setCursor(17, 49);
        this->display.print("to SETUP");

        this->display.display();
    }

    void displayMainMenu(int M, int K, int H, int A) {
        this->display.clearDisplay();
        this->display.setTextSize(2);      // Normal 1:1 pixel scale
        this->display.setTextColor(WHITE); // Draw white text
        this->display.setCursor(0, 1);     // Start at top-left corner
        this->display.cp437(true);         // Use full 256 char 'Code Page 437' font

        this->display.println(F("DDS"));
        this->display.setCursor(40, 1);
        this->display.print(F("9910"));

        this->display.setTextSize(1);      // Normal 1:1 pixel scale
        this->display.setCursor(88, 8);
        this->display.print("v3");
        this->display.setCursor(104, 0);
        this->display.print(F("APQ"));
        this->display.setCursor(104, 8);
        this->display.print(F("TUDa"));

        this->display.setTextColor(WHITE); // Draw white text
        this->display.setCursor(0, 16);     // Start at top-left corner

        this->display.println(F("Frequency, [Hz]:"));

        // this->display.setCursor(100, 16);     
        // this->display.print(F("RF ON"));

        this->display.setTextSize(2);      // Normal 1:1 pixel scale
        this->display.setTextColor(WHITE);
        this->display.setCursor(1, 26);
        this->display.print(PreZero(M));

        this->display.setTextColor(WHITE);
        this->display.setCursor(34, 26);
        this->display.print(F("."));

        this->display.setTextColor(WHITE);

        this->display.setCursor(45, 26);
        this->display.print(PreZero(K));

        this->display.setTextColor(WHITE);
        this->display.setCursor(78, 26);
        this->display.print(F("."));

        this->display.setCursor(89, 26);
        this->display.setTextColor(WHITE);
        this->display.println(PreZero(H));

        this->display.setTextSize(1);      // Normal 1:1 pixel scale
        this->display.setTextColor(WHITE); // Draw white text
        this->display.setCursor(0, 42);     // Start at top-left corner

        this->display.print(F("Amplitude"));
        this->display.setCursor(52, 43);
        this->display.print(F(":"));

        this->display.setTextSize(2);      // Normal 1:1 pixel scale
        this->display.setTextColor(WHITE); // Draw white text
        this->display.setCursor(0, 50);     // Start at top-left corner

        this->display.setTextColor(WHITE);
        if ((A) > 0) this->display.print(F("-"));
        this->display.setCursor(12, 50);
        this->display.print(PreZero2(abs(A)));
        this->display.setTextSize(1);
        this->display.setTextColor(WHITE);
        this->display.setCursor(36, 56);
        this->display.print(F("dBm"));

        this->display.setCursor(62, 42); // Start at top-left corner
        this->display.print(F("RF on"));

        this->display.drawLine(57, 42, 57, 64, WHITE);

        this->display.setTextColor(WHITE);
        this->display.setCursor(62, 56);
        this->display.print(F("Local Osc"));

        this->display.display();
    }

    void displayMessage(String Title, String Message) {
        this->display.clearDisplay();
        this->display.cp437(true);
        this->display.setTextSize(2);
        this->display.setTextColor(WHITE); // Draw white text
        this->display.setCursor(0, 0);
        this->display.print(Title);
        this->display.setTextSize(1);
        this->display.setCursor(5, 28);
        this->display.print(Message);

        this->display.display();
    }

    void displayPowerWarning() {
        static const unsigned char PROGMEM image_alert_bicubic_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,0x00,0x00,0x07,0xe0,0x00,0x00,0x00,0x00,0x07,0xe0,0x00,0x00,0x00,0x00,0x0f,0xf0,0x00,0x00,0x00,0x00,0x0e,0x70,0x00,0x00,0x00,0x00,0x1c,0x38,0x00,0x00,0x00,0x00,0x3c,0x3c,0x00,0x00,0x00,0x00,0x38,0x1c,0x00,0x00,0x00,0x00,0x78,0x1e,0x00,0x00,0x00,0x00,0x70,0x0e,0x00,0x00,0x00,0x00,0xf0,0x0f,0x00,0x00,0x00,0x00,0xe3,0xc7,0x00,0x00,0x00,0x01,0xc3,0xc3,0x80,0x00,0x00,0x03,0xc3,0xc3,0xc0,0x00,0x00,0x03,0x83,0xc1,0xc0,0x00,0x00,0x07,0x83,0xc1,0xe0,0x00,0x00,0x07,0x03,0xc0,0xe0,0x00,0x00,0x0e,0x03,0xc0,0xf0,0x00,0x00,0x0e,0x03,0xc0,0x70,0x00,0x00,0x1c,0x03,0xc0,0x38,0x00,0x00,0x3c,0x03,0xc0,0x3c,0x00,0x00,0x38,0x03,0xc0,0x1c,0x00,0x00,0x78,0x03,0xc0,0x1e,0x00,0x00,0x70,0x03,0xc0,0x0e,0x00,0x00,0xe0,0x03,0xc0,0x07,0x00,0x01,0xe0,0x03,0xc0,0x07,0x80,0x01,0xc0,0x03,0xc0,0x03,0x80,0x03,0xc0,0x01,0x80,0x03,0xc0,0x03,0x80,0x00,0x00,0x01,0xc0,0x07,0x80,0x00,0x00,0x01,0xe0,0x07,0x00,0x01,0x80,0x00,0xe0,0x0e,0x00,0x03,0xc0,0x00,0x70,0x1e,0x00,0x03,0xc0,0x00,0x78,0x1c,0x00,0x01,0x80,0x00,0x38,0x3c,0x00,0x00,0x00,0x00,0x3c,0x38,0x00,0x00,0x00,0x00,0x1c,0x38,0x00,0x00,0x00,0x00,0x1c,0x3f,0xff,0xff,0xff,0xff,0xfc,0x1f,0xff,0xff,0xff,0xff,0xf8,0x01,0xff,0xff,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        this->display.clearDisplay();
        this->display.setTextColor(1);
        this->display.setTextSize(2);
        this->display.setCursor(23, 1);
        this->display.print("WARNING");
        this->display.setTextWrap(false);
        this->display.setCursor(43, 17);
        this->display.print("7.5 VDC");
        this->display.setTextSize(1);
        this->display.setCursor(49, 35);
        this->display.print("POWER SUPPLY");
        this->display.setTextSize(2);
        this->display.setCursor(72, 45);
        this->display.print("USB");
        this->display.drawBitmap(1, 11, image_alert_bicubic_bits, 48, 48, 1);
        this->display.setTextSize(1);
        this->display.setCursor(55, 48);
        this->display.print("or");
        this->display.display();
    }

private:
    Adafruit_SSD1306 display;

    static String PreZero(int Digit)
    {
        if ((Digit < 100) && (Digit >= 10)) return "0" + String(Digit);
        if (Digit < 10) return "00" + String(Digit);
        return String(Digit);
    }

    static String PreZero2(int Digit)
    {
        if (Digit < 10) return "0" + String(Digit);
        return String(Digit);
    }
};

#endif