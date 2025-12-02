/****************************************************************************
 *   OFICIAL WEB-SITE:  https://gra-afch.com/product-category/rf-units/     *
 ****************************************************************************                                      
 *
 * AD9910 - DDS Power Supply 3.3V - 1.8V
 * Internal PLL up to 1.52 GHz, REF CLK Oscillator 5 - 60Mhz (PLL Enabled)
 * For More pure spectrum Recomendeted exteral oscillator 1 - 1.5 GHz
 * For signal without harmonics and the spur use an RF transformer  
 * ADT2-1T + LPF 7rd order Range: 0.4-450MHZ
 * ADT2-1T-1P + LPF 7rd order Range: 8-600MHZ
 * WBC2-1TLB + LPF 7rd order Range: 0.1(-4dB)-600MHZ(-7dB)
 * CX2147 + LPF 7rd order Range: 0.02(-3dB)-365(-3dB)/ or 0.01(-8dB)-480(-8dB)
 * Fout up 600 Mhz 
 * 17.06.2020
 * Author Grisha Anofriev e-mail: grisha.anofriev@gmail.com
******************************************************************************/

#include <Float64.h>
#define DEFAULT_STEP_VALUE 1000

#include "ad9910.h"
//#include "menuclk.h"
#include "main.h"

#include <math.h>
#include <float.h>

#include <Arduino.h> // This one no

int hspi1=0; // used in hal writing but does nothing

uint32_t DAC_Current;    // needed ? 
uint8_t strBuffer[9];    //={129,165, 15, 255};  needed ? 
uint32_t FTW;            // needed ?
uint32_t *jlob;          //  needed ? 
extern uint32_t Ref_Clk;  // needed ? 

// DOne
void HAL_SPI_Transmit(int *blank, uint8_t *strBuffer, int nums, int pause)
{
  for (int i=0;i<nums; i++)
  {
    SPI.transfer(*(strBuffer+i));
  }
}
// DOne
void HAL_Delay (int del)
{
  delayMicroseconds(del);
}
/******************************************************************************
 * HAL to Arduino
******************************************************************************/
// DOne
void HAL_GPIO_WritePin (int port, int pin, int mode)
{
  digitalWrite(pin, mode);
}

/******************************************************************************
 * Init GPIO for DDS
******************************************************************************/
// DONE
void DDS_GPIO_Init(void)
{
   pinMode(DDS_SPI_SCLK_PIN, OUTPUT); 
   pinMode(DDS_SPI_SDIO_PIN, OUTPUT); 
   pinMode(DDS_SPI_SDO_PIN, INPUT);    
   pinMode(DDS_SPI_CS_PIN, OUTPUT);   
   pinMode(DDS_IO_UPDATE_PIN, OUTPUT); 
   pinMode(DDS_IO_RESET_PIN, OUTPUT);
   pinMode(DDS_MASTER_RESET_PIN, OUTPUT);
   pinMode(DDS_PROFILE_0_PIN, OUTPUT);
   pinMode(DDS_PROFILE_1_PIN, OUTPUT);
   pinMode(DDS_PROFILE_2_PIN, OUTPUT);
   pinMode(DDS_OSK_PIN, OUTPUT);
   pinMode(DDS_TxENABLE_PIN, OUTPUT); 
   pinMode(DDS_F0_PIN, OUTPUT); 
   pinMode(DDS_F1_PIN, OUTPUT);
   pinMode(DDS_DRHOLD_PIN, OUTPUT); 
   pinMode(DDS_PWR_DWN_PIN, OUTPUT); 
   pinMode(DDS_DRCTL_PIN, OUTPUT); 
   
   pinMode(DDS_DROVER, INPUT);
   pinMode(DDS_SYNC_CLK, INPUT);
   pinMode(DDS_RAM_SWP_OVR, INPUT);
   pinMode(DDS_PLL_LOCK, INPUT);
   pinMode(DDS_PDCLK_PIN, INPUT);

  /*Configure GPIO pin Output Level */
  HAL_Delay(50);
  HAL_GPIO_WritePin(DDS_IO_UPDATE_GPIO_PORT, DDS_IO_UPDATE_PIN, GPIO_PIN_RESET);
  HAL_Delay(50);
	HAL_GPIO_WritePin(DDS_MASTER_RESET_GPIO_PORT, DDS_MASTER_RESET_PIN, GPIO_PIN_RESET);
  HAL_Delay(50);
	HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);
  HAL_Delay(50);
	HAL_GPIO_WritePin(DDS_OSK_GPIO_PORT, DDS_OSK_PIN, GPIO_PIN_RESET);                     // OSK = 0
  HAL_Delay(50);
	HAL_GPIO_WritePin(DDS_PROFILE_0_GPIO_PORT, DDS_PROFILE_0_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DDS_PROFILE_1_GPIO_PORT, DDS_PROFILE_1_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DDS_PROFILE_2_GPIO_PORT, DDS_PROFILE_2_PIN, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(DDS_PROFILE_2_GPIO_PORT, DDS_DRHOLD_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DDS_PROFILE_2_GPIO_PORT, DDS_DRCTL_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DDS_PROFILE_2_GPIO_PORT, DDS_PWR_DWN_PIN, GPIO_PIN_RESET);
  HAL_Delay(50);
}

/******************************************************************************
 * Init SPI, 8bit, Master
 * MODE 3, MSB, 
******************************************************************************/
// 
void DDS_SPI_Init(void)
{
  SPI.begin(); //
  SPI.setDataMode (SPI_MODE0); 
  SPI.setClockDivider(SPI_CLOCK_DIV8); //16MHZ/8=2MHZ
  SPI.setBitOrder(MSBFIRST);
}


/*****************************************************************************************
   Update - data updates from memory
*****************************************************************************************/ 
void DDS_UPDATE(void)
{
	// Required - data updates from memory
   #if DBG==1
   Serial.println("***DDS_UPDATE****");
   #endif
	 HAL_Delay(10);
	 HAL_GPIO_WritePin(DDS_IO_UPDATE_GPIO_PORT, DDS_IO_UPDATE_PIN, GPIO_PIN_RESET); // IO_UPDATE = 0
	 HAL_Delay(10);
	 HAL_GPIO_WritePin(DDS_IO_UPDATE_GPIO_PORT, DDS_IO_UPDATE_PIN, GPIO_PIN_SET); // IO_UPDATE = 1
	 HAL_Delay(10);
	 HAL_GPIO_WritePin(DDS_IO_UPDATE_GPIO_PORT, DDS_IO_UPDATE_PIN, GPIO_PIN_RESET); // IO_UPDATE = 0
	 HAL_Delay(10);
}
 

/*****************************************************************************************
   F_OUT - Set Frequency in HZ 
   Num_Prof - Single Tone Mode 0..7
   Amplitude_dB - amplitude in dB from 0 to -84 (only negative values)
*****************************************************************************************/
// DONE
void DDS_Fout (uint32_t *F_OUT, int16_t Amplitude_dB, uint8_t Num_Prof)
{
   uint32_t RealDDSCoreClock=CalcRealDDSCoreClockFromOffset();
   //FTW = ((uint32_t)(4294967296.0 *((float)*F_OUT / (float)DDS_Core_Clock)));
   //FTW = round(4294967296.0 *((float)*F_OUT / ((float)DDS_Core_Clock-ClockOffset))); // закомментировано 27.05.2020

   // значение FTW должно быть 4294967296 
    float64_t f64F_OUT=f64(*F_OUT);
    float64_t f64CoreClock=f64(RealDDSCoreClock);
    float64_t TwoPower32=f64(4294967295UL);
    float64_t f64FTW;
    //float64_t Offset=f64(ClockOffset);
    //CoreClock=f64_sub(CoreClock, Offset);
    f64FTW=f64_div(TwoPower32, f64CoreClock);
    f64FTW=f64_mul(f64FTW, f64F_OUT);
   //FTW = 4294967296 * (*F_OUT / (float)tmpCoreClock);
   bool a;
   uint_fast8_t softfloat_roundingMode;
   softfloat_roundingMode=softfloat_round_near_maxMag;
   FTW=f64_to_ui32(f64FTW, softfloat_roundingMode, a);
   jlob = & FTW;
   
   #if DBG==1
   Serial.print(F("*F_OUT="));
   Serial.println(*F_OUT); 
   Serial.print(F("DDS_Core_Clock="));
   Serial.println(DDS_Core_Clock); 
   Serial.print(F("FTW="));
   Serial.println(FTW); 
   #endif
   
   HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET); 	 
	 strBuffer[0] = Num_Prof; // Single_Tone_Profile_#;

   //ASF  - Amplitude 14bit 0...16127
	 strBuffer[1] =  (uint16_t)powf(10,(Amplitude_dB+84.288)/20.0) >> 8;     
	 strBuffer[2] =  (uint16_t)powf(10,(Amplitude_dB+84.288)/20.0);         
	 strBuffer[3] = 0x00;
	 strBuffer[4] = 0x00;

	 strBuffer[5] = *(((uint8_t*)jlob)+ 3);
	 strBuffer[6] = *(((uint8_t*)jlob)+ 2);
	 strBuffer[7] = *(((uint8_t*)jlob)+ 1);
	 strBuffer[8] = *(((uint8_t*)jlob));

   HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 9, 1000);
	 HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);
	 
	 DDS_UPDATE(); 
  
   int Prof=Num_Prof;
   Prof=Prof-14; // address of 0 profile: 0x0E

   //Serial.print("Prof=");
   //Serial.println(Prof);
   
   if (bitRead(Prof, 0)==1) HAL_GPIO_WritePin(DDS_PROFILE_0_GPIO_PORT, DDS_PROFILE_0_PIN, GPIO_PIN_SET);
    else HAL_GPIO_WritePin(DDS_PROFILE_0_GPIO_PORT, DDS_PROFILE_0_PIN, GPIO_PIN_RESET);
   if (bitRead(Prof, 1)==1) HAL_GPIO_WritePin(DDS_PROFILE_1_GPIO_PORT, DDS_PROFILE_1_PIN, GPIO_PIN_SET);
    else HAL_GPIO_WritePin(DDS_PROFILE_1_GPIO_PORT, DDS_PROFILE_1_PIN, GPIO_PIN_RESET);
   if (bitRead(Prof, 2)==1) HAL_GPIO_WritePin(DDS_PROFILE_2_GPIO_PORT, DDS_PROFILE_2_PIN, GPIO_PIN_SET);
    else HAL_GPIO_WritePin(DDS_PROFILE_2_GPIO_PORT, DDS_PROFILE_2_PIN, GPIO_PIN_RESET);

   DDS_UPDATE(); 
   //while (1);
}	


/************************************************************************************************************
 * Функция находит и взвращает такое значение STEP и StepRate при котором значение StepRate было бы целым числом, но при этом значение STEP не превысило бы значение 1000
 * входные параметры это: указатетль на переменную STEP, по адресу этого указателя будет возвращено новое значение STEP, 
 * указатетль на переменную StepRate, по адресу этого указателя будет возвращено новое значение StepRate, 
 * F_mod - частота модуляции
 ************************************************************************************************************/
// NOT SURE NEEDED
 void calcBestStepRate(uint16_t *Step, uint64_t *Step_Rate, uint32_t F_mod)
{
  double T_Step;
  double fStep_Rate;

  uint32_t RealDDSCoreClock=CalcRealDDSCoreClockFromOffset();
  
  T_Step = 1.0/(F_mod * *Step); // necessary time step
  fStep_Rate = (T_Step * (float)RealDDSCoreClock)/4; // the value of M for the register Step_Rate, for the desired sampling rate from RAM

  *Step_Rate=ceil(fStep_Rate);
 
  *Step=(1.0/((*Step_Rate*4)/(float)RealDDSCoreClock))/F_mod;
}


/*****************************************************************************
  Initialization DDS
  * Config SPI, Reset DDS and ReConfig SPI
  * Enable/Disable internal PLL, mux, div, charge pump current, Set VCO
  
    Input Parametr:
  * PLL - 1 enable, if 0 disable
  * Divider - This input REF CLOCK divider by 2, if 1 - Divider by 2, if 0 - Divider OFF
  * 
  * Set Current Output - 0..255, 127 - Default equal to 0 dB 
  * (если выключено, то в функцию нужно передать (или записать в управляющий регистр) 127, а если включено, то 255, и если включено, то также нужно в главном меню к значению ДБ прибавлять +4 dBM)
*****************************************************************************/
void DDS_Init(bool PLL, bool Divider, uint32_t Ref_Clk)
 {
   #if DBG==1
   Serial.println(F("DDS_Init"));
   Serial.print(F("PLL="));
   Serial.println(PLL);
   Serial.print(F("Divider="));
   Serial.println(Divider);
   Serial.print(F("Ref_Clk="));
   Serial.println(Ref_Clk);
   Serial.print(F("DDS_Core_Clock="));
   Serial.println(DDS_Core_Clock);
   #endif
   
   DDS_GPIO_Init();
   
   // It is very important for DDS AD9910 to set the initial port states
   HAL_GPIO_WritePin(DDS_MASTER_RESET_GPIO_PORT, DDS_MASTER_RESET_PIN, GPIO_PIN_SET);   // RESET = 1
   HAL_Delay(10);
   HAL_GPIO_WritePin(DDS_MASTER_RESET_GPIO_PORT, DDS_MASTER_RESET_PIN, GPIO_PIN_RESET); // RESET = 0
   HAL_GPIO_WritePin(DDS_IO_UPDATE_GPIO_PORT, DDS_IO_UPDATE_PIN, GPIO_PIN_RESET);       // IO_UPDATE = 0   
   HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);               // CS = 1
   HAL_GPIO_WritePin(DDS_OSK_GPIO_PORT, DDS_OSK_PIN, GPIO_PIN_SET);                     // OSK = 1
   HAL_GPIO_WritePin(DDS_PROFILE_0_GPIO_PORT, DDS_PROFILE_0_PIN, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(DDS_PROFILE_1_GPIO_PORT, DDS_PROFILE_1_PIN, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(DDS_PROFILE_2_GPIO_PORT, DDS_PROFILE_2_PIN, GPIO_PIN_RESET);
   
   DDS_SPI_Init();
   
   HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
   strBuffer[0] = CFR1_addr;
   strBuffer[1] = 0;// RAM_enable;//RAM_Playback_Amplitude;// | RAM_enable;//0x00; 
   strBuffer[2] = 0;//Inverse_sinc_filter_enable;//0; //Continuous_Profile_0_1; //0;//0x80;//0x00;
   strBuffer[3] = 0; //OSK_enable | Select_auto_OSK;//0x00;
   strBuffer[4] = SDIO_input_only ;
   HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 5, 1000);
   HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);
   
   DDS_UPDATE();
   
   HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
   strBuffer[0] = CFR2_addr;
   strBuffer[1] = Enable_amplitude_scale_from_single_tone_profiles;//1;//0x00;
   strBuffer[2] = 0;//SYNC_CLK_enable;// | Read_effective_FTW;
   strBuffer[3] = 0;//0x08;//PDCLK_enable;
   strBuffer[4] = Sync_timing_validation_disable;// | Parallel_data_port_enable;
   HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 5, 1000);
   HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);
   
   DDS_UPDATE();

  switch (PLL)
  {
    case false:
      /******************* External Oscillator 60 - 1000Mhz (Overclock up to 1500Mhz) ***************/ 
      HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
      strBuffer[0] = CFR3_addr;
      strBuffer[1] = 0;//DRV0_REFCLK_OUT_High_output_current;//
      strBuffer[2] = 0;
      if (Divider) strBuffer[3] = REFCLK_input_divider_ResetB;
        else strBuffer[3] = REFCLK_input_divider_ResetB | REFCLK_input_divider_bypass;
      strBuffer[4] = 0; // SYSCLK= REF_CLK * N
      HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 5, 1000);
      HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);
      DDS_UPDATE();
      //**************************
    break;
    case true:
      /******************* External Oscillator TCXO 3.2 - 60 MHz ***********************************************/ 
      HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
      strBuffer[0] = CFR3_addr;
      if (DDS_Core_Clock<=1000000000) strBuffer[1] = VCO5; // bilo VCO3
        else strBuffer[1] = VCO5;  // | DRV0_REFCLK_OUT_High_output_current;
      strBuffer[2] = Icp387uA;   // Icp212uA, Icp237uA, Icp262uA, Icp287uA, Icp312uA, Icp337uA, Icp363uA, Icp387uA 
      strBuffer[3] = REFCLK_input_divider_ResetB | PLL_enable; // REFCLK_input_divider_bypass; //
      //strBuffer[4]=((uint32_t)DDS_Core_Clock/Ref_Clk)*2; // multiplier for PLL
      strBuffer[4]=round((float)DDS_Core_Clock/(float)Ref_Clk)*2; // multiplier for PLL
      //strBuffer[4]=round(((float)DDS_Core_Clock-ClockOffset)/(float)Ref_Clk)*2; // multiplier for PLL
      HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 5, 1000);
      HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);
      DDS_UPDATE();
    /**********************/
    break;
  }
   
   HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
   strBuffer[0] = FSC_addr;
   strBuffer[1] = 0;
   strBuffer[2] = 0;
   strBuffer[3] = 0;
   if (DACCurrentIndex==0) strBuffer[4] = 0x7F; //DAC current Normal
    else strBuffer[4] = 0xFF;// Max carrent 255 = 31mA //DAC current HI
   HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 5, 1000);
   HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);
   
   DDS_UPDATE();
}                

/*************************************************************************
 * Freq Out, freq_output in Hz 0 HZ - 450MHZ, 
 * amplitude_dB_output - from 0 to -84 (negative)
 ************************************************************************/
void SingleProfileFreqOut(uint32_t freq_output, int16_t amplitude_dB_output) 
{
  #if DBG==1
  Serial.println(F("****SingleProfileFreqOut***"));
  Serial.print(F("freq_output="));
  Serial.println(freq_output);
  Serial.print(F("amplitude_dB_output="));
  Serial.println(amplitude_dB_output);
  #endif
  //*** RAM Disable ***
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
  strBuffer[0] = CFR1_addr;
  strBuffer[1] = 0; // RAM Disable
  strBuffer[2] = 0;//Inverse_sinc_filter_enable;//0;//
  strBuffer[3] = 0;//OSK Disable
  strBuffer[4] = SDIO_input_only ;
  HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 5, 1000);
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);  
  DDS_UPDATE();
  
  //********* Digital Ramp disable*******
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
  strBuffer[0] = CFR2_addr;
  strBuffer[1] = Enable_amplitude_scale_from_single_tone_profiles;//1;//0x00;
  strBuffer[2] = 0;//SYNC_CLK_enable;// | Read_effective_FTW; // Digital Ramp disable
  strBuffer[3] = 0;//PDCLK_enable;
  strBuffer[4] = Sync_timing_validation_disable;// | Parallel_data_port_enable;
  HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 5, 1000);
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);  
  DDS_UPDATE();
  
  DDS_Fout(&freq_output, amplitude_dB_output, Single_Tone_Profile_0);
}


/**************
 * Return Real Core Clock based on ClockOffset 
 */
uint32_t CalcRealDDSCoreClockFromOffset()
{
  int N=DDS_Core_Clock/Ref_Clk;
  uint32_t RCC = (Ref_Clk+ClockOffset)*N;
  #if DBG==1
  Serial.print("DDS_Core_Clock=");
  Serial.println(DDS_Core_Clock);
  Serial.print("Ref_Clk=");
  Serial.println(Ref_Clk);
  Serial.print("N=");
  Serial.println(N); 
  Serial.print("RCC=");
  Serial.println(RCC);
  #endif
  return RCC;
}

/*************************************************************************************
 * Digital Ramp Generator Enable
 **********************************************************************************/
void DigitalRamp(uint32_t FTWStart, uint32_t FTWEnd, uint32_t FTWStepSize, uint16_t StepRate, bool continuous)
{

  int Amplitude_dB=0;

   HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);   
   strBuffer[0] = Single_Tone_Profile_0; //Num_Prof; // Single_Tone_Profile_#;

   //ASF  - Amplitude 14bit 0...16127
   strBuffer[1] =  (uint16_t)powf(10,(Amplitude_dB+84.288)/20.0) >> 8;     
   strBuffer[2] =  (uint16_t)powf(10,(Amplitude_dB+84.288)/20.0);       
   strBuffer[3] = 0; //0xFF;
   strBuffer[4] = 0; //0xFF;
   strBuffer[5] = 0; //0xFF;
   strBuffer[6] = 0; //0xFF;
   strBuffer[7] = 0; //0xFF;
   strBuffer[8] = 0; //0xFF;

   HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 9, 1000);
   HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);
   DDS_UPDATE(); 

    HAL_GPIO_WritePin(DDS_PROFILE_0_GPIO_PORT, DDS_PROFILE_0_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DDS_PROFILE_1_GPIO_PORT, DDS_PROFILE_1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DDS_PROFILE_2_GPIO_PORT, DDS_PROFILE_2_PIN, GPIO_PIN_RESET);
   
////
  
  //*** RAM Disable
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
  strBuffer[0] = CFR1_addr;
  strBuffer[1] = 0;// RAM_enable; 
  strBuffer[2] = 0; 
  strBuffer[3] = Autoclear_digital_ramp_accumulator;// БЫЛО 0//OSK Disable // Autoclear_digital_ramp_accumulator (нужен чтобы програмно можно был перезапустить DRG c начала);
  strBuffer[4] = SDIO_input_only ;
  HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 5, 1000);
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);  
  DDS_UPDATE();


//*** digital Ramp LIMITS enable
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
  strBuffer[0] = Digital_Ramp_Limit;
  
  strBuffer[1] = FTWEnd>>24; // upper limit
  strBuffer[2] = FTWEnd>>16; // upper limit
  strBuffer[3] = FTWEnd>>8;  // upper limit
  strBuffer[4] = FTWEnd;     // upper limit
  
  strBuffer[5] = FTWStart>>24; // lower limit
  strBuffer[6] = FTWStart>>16; // lower limit
  strBuffer[7] = FTWStart>>8;  // lower limit
  strBuffer[8] = FTWStart;     // lower limit
  
  HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 9, 1000);
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);  
  DDS_UPDATE();

  //*** digital  Ramp Step Size enable
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
  strBuffer[0] = Digital_Ramp_Step_Size;
  
  //**** Dicrement ****
  strBuffer[1] = FTWStepSize>>24; 
  strBuffer[2] = FTWStepSize>>16;  
  strBuffer[3] = FTWStepSize>>8; 
  strBuffer[4] = FTWStepSize; 

   //**** Increment ****
  strBuffer[5] = FTWStepSize>>24; 
  strBuffer[6] = FTWStepSize>>16;
  strBuffer[7] = FTWStepSize>>8;
  strBuffer[8] = FTWStepSize; 
  
  HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 9, 1000);
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);  
  DDS_UPDATE();

  //*** digital  Ramp Step Rate enable
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
  strBuffer[0] = Digital_Ramp_Rate;
  
  strBuffer[1] = StepRate>>8; // Negative Slope Rate
  strBuffer[2] = StepRate; // Negative Slope Rate
  strBuffer[3] = StepRate>>8; // Positive Slope Rate
  strBuffer[4] = StepRate; // Positive Slope Rate
  
  HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 5, 1000);
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);  
  DDS_UPDATE();
  
 //*** digital Ramp Generator enable
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
  strBuffer[0] = CFR2_addr;
  strBuffer[1] = 0; 
  if (continuous == true) strBuffer[2] = Digital_Ramp_Destination_Frequency | Digital_ramp_enable | Digital_ramp_no_dwell_high | Digital_ramp_no_dwell_low; // Digital_ramp_no_dwell_high - не останавливатся вверху, Digital_ramp_no_dwell_low - не остановливатся внизу
    else strBuffer[2] =                  Digital_Ramp_Destination_Frequency | Digital_ramp_enable;// | Digital_ramp_no_dwell_high | Digital_ramp_no_dwell_low;  // Digital_ramp_no_dwell_high - не останавливатся вверху, Digital_ramp_no_dwell_low - не остановливатся внизу
  strBuffer[3] = 0;//
  strBuffer[4] = 0;// ;
  HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 5, 1000);
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);  
  pinMode(DDS_DRCTL_PIN, OUTPUT); //????????
  digitalWrite(DDS_DRCTL_PIN, HIGH); //??????
  DDS_UPDATE();
  #if DBG==1
  Serial.print("CFR2=");
  Serial.println(strBuffer[2]);
  Serial.println("End of Digital Ramp Function");
  #endif
}


//SweepTimeFormat: 0 - Seconds, 1 - Milliseconds (mS), 2 - MicroSeconds (uS), 3 - NanoSeconds
void Sweep(uint32_t StartSweepFreq, uint32_t StopSweepFreq, uint16_t SweepTime, uint8_t SweepTimeFormat, bool continuous)
{
  #if DBG==1
  Serial.print("StartSweepFreq=");
  Serial.println(StartSweepFreq);
  Serial.print("StopSweepFreq=");
  Serial.println(StopSweepFreq);
  Serial.print("SweepTime=");
  Serial.println(SweepTime);
  Serial.print("SweepTimeFormat=");
  Serial.println(SweepTimeFormat);
  #endif
  //float64_t NanoSweepTime=f64((uint32_t)SweepTime); //F64_div, f64_mul
  uint64_t NanoSweepTime;
  uint64_t CaluclatedNanoSweepTime;
  uint32_t FTWStart=FreqToFTW(StartSweepFreq);
  uint32_t FTWEnd=FreqToFTW(StopSweepFreq);
  uint32_t DeltaFTW=FTWEnd-FTWStart;
  uint32_t FTWStepSize=1;
  uint16_t StepRate=1;
  
  if (SweepTimeFormat==0) NanoSweepTime=SweepTime*1E9;
    else if (SweepTimeFormat==1) NanoSweepTime=SweepTime*1E6;
      else if (SweepTimeFormat==2) NanoSweepTime=SweepTime*1E3;
      
  float GHZ_CoreClock=DDS_Core_Clock/1E9; //незабыть заменит на realDDSCoreClock
  CaluclatedNanoSweepTime=(4/GHZ_CoreClock*DeltaFTW*FTWStepSize);
  
  if (CaluclatedNanoSweepTime<NanoSweepTime) 
  {
    uint32_t StepRateMultiplier=round(NanoSweepTime/float(CaluclatedNanoSweepTime));
    if (StepRateMultiplier<=0xFFFF) StepRate=StepRate*StepRateMultiplier;
      else StepRate=0xFFFF;
  }
  #if DBG==1
  Serial.print("StepRate=");
  Serial.println((uint32_t)StepRate);
  #endif
  if (CaluclatedNanoSweepTime>NanoSweepTime)
  {
    uint32_t FTWMultiplier=round(CaluclatedNanoSweepTime/float(NanoSweepTime));
    FTWStepSize=FTWStepSize*FTWMultiplier;
  }
  #if DBG==1
  Serial.print("FTWStepSize=");
  Serial.println((uint32_t)FTWStepSize);
  #endif
  DigitalRamp(FTWStart, FTWEnd, FTWStepSize, StepRate, continuous);
}

// DONE
uint32_t FreqToFTW(uint32_t Freq)
{
    uint32_t RealDDSCoreClock=CalcRealDDSCoreClockFromOffset();
    bool a;
    
    float64_t f64Freq=f64(Freq);
    float64_t f64CoreClock=f64(RealDDSCoreClock);
    float64_t TwoPower32=f64(4294967295UL);
    float64_t f64FTW;
    
    f64FTW=f64_div(TwoPower32, f64CoreClock);
    f64FTW=f64_mul(f64FTW, f64Freq);
   
    uint_fast8_t softfloat_roundingMode;
    softfloat_roundingMode=softfloat_round_near_maxMag;
    return f64_to_ui32(f64FTW, softfloat_roundingMode, a);
}

// DONE
void RestartDRG()
{
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_RESET);    
  strBuffer[0] = CFR2_addr;
  strBuffer[1] = 0; 
  strBuffer[2] = Digital_Ramp_Destination_Frequency | Digital_ramp_enable;// | Digital_ramp_no_dwell_high | Digital_ramp_no_dwell_low;  // Digital_ramp_no_dwell_high - не останавливатся вверху, Digital_ramp_no_dwell_low - не остановливатся внизу
  strBuffer[3] = 0;//
  strBuffer[4] = 0;// ;
  HAL_SPI_Transmit(&hspi1, (uint8_t*)strBuffer, 5, 1000);
  HAL_GPIO_WritePin(DDS_SPI_CS_GPIO_PORT, DDS_SPI_CS_PIN, GPIO_PIN_SET);  
  
  HAL_GPIO_WritePin(DDS_IO_UPDATE_GPIO_PORT, DDS_IO_UPDATE_PIN, GPIO_PIN_RESET); // IO_UPDATE = 0
	//  HAL_Delay(10);
	HAL_GPIO_WritePin(DDS_IO_UPDATE_GPIO_PORT, DDS_IO_UPDATE_PIN, GPIO_PIN_SET); // IO_UPDATE = 1
	//  HAL_Delay(10);
	HAL_GPIO_WritePin(DDS_IO_UPDATE_GPIO_PORT, DDS_IO_UPDATE_PIN, GPIO_PIN_RESET); // IO_UPDATE = 0
}
