### sources
- product website : https://gra-afch.com/catalog/rf-units/dds-ad9910-v3-shield-for-arduino-rf-signal-generator-am-fm-sweep-600-mhz-1-5-ghz-core-clock-low-spurs-low-harmonic/

- Opensource Firmware available on our GitHub repository:  https://github.com/afch/DDS-AD9910-Arduino-Shield
- Used code HW3.x
- DDS AD9910 Firmware compilation and uploading to Arduino Mega Tutorial: youtube.com/watch?v=Zn1xLlvlVXE
- Video review of DDS AD9910 Shield on our YouTube channel: youtube.com/watch?v=GNM5LJT_cgw



### TODOS and MAYBES 

**TODOS**
1. Repair cfr3
2. make a helper for the single tone amplitude

**MAYBES**
1. Right Now, The Code is still hybrid, in the meaning that although there is an abstraction layer implemented for the MCU Hardware, we still rely on HAL on some level. This dependency can at some point be rethought by addressing the MCU through registers directly, which will allow for some efficiency gain. The current architecture paved the path for such implementations, however due to a higher code error probability, we omit this idea for futur improvments based on a finished and tested code base.

2. Closely realted to the firt point, is pin handeling architecture which could be improved to an array of pin handlers. Currently we use the DdsPin class which then calls a pin_t from which we take then only half of the information, becasue the rest is not needed at run time anyway. Clearly, This can be improved.


### Remember 
- On STM32, if a pin is never configured as output, writing to it later may not work correctly.
- On Arduino, uninitialized pins default to INPUT, so writing could work differently.
- AD9910 Big Endian

### WEIRD Behaviour of OG CODE to investigate

- SPI WRITE FORMAT
OG sends the following 9 bytes:
[0] = profile #
[1] = ASF high byte
[2] = ASF low byte
[3] = 0
[4] = 0
[5] = FTW byte3
[6] = FTW byte2
[7] = FTW byte1
[8] = FTW byte0
This is not a real AD9910 register write.
This is a bizarre custom packet into a custom firmware block, not the AD9910 protocol.

This line in the OG code 
if (DDS_Core_Clock<=1000000000) strBuffer[1] = VCO5; // bilo VCO3
else strBuffer[1] = VCO5;  // | DRV0_REFCLK_OUT_High_output_current;




 



