#include "main.h"

#define LOW_FREQ_LIMIT  100000
#define HIGH_FREQ_LIMIT  600000000
// 0 - 20, 64 used for clock settings
#define M_ADR 24
#define K_ADR 28
#define H_ADR 32
#define A_ADR 36

#define MAIN_SETTINGS_FLAG_ADR 100 // defualt settings 
// ADR 101 reserved for clock settings
#define MODULATION_SETTINGS_FLAG_ADR 102 // defualt settings

#define INIT_M 100
#define INIT_K 0
#define INIT_H 0
#define INIT_A 0

//****************************************

int M, K, H, A, MenuPos;

// definitions for get supply voltage
#define VOLTAGE_DIVIDER_FACTOR 0.1754
#define ANALOG_REFERENCE_VOLTAGE 4950
#define INPUT_VOLTAGE_PIN A3 //VIN_MEAS 

Display* display = nullptr;
ClickButton modeButton(MODE_PIN, LOW, CLICKBTN_PULLUP);

bool isPWR_DWN = false;

const int freqBufferSize = 128;
uint32_t bufferedFrequencies[freqBufferSize];


//Definitions for data transmission
const byte BUFFERSIZE = 18;                 //number of characters that can be saved per transmission cycle
const char STARTMARKER = '[';
const char ENDMARKER = ']';
const char STARTMARKERSEQUENCE = '<';
const char ENDMARKERSEQUENCE = '>';
char inputBuffer[BUFFERSIZE];               //array to save received message
int arrWriteIndex = 0;                      //index transmitted with profile data to store in correct position of data_array
int arrReadIndex = 0;                       //index transmitted with profile data to read from correct position of data_array
byte bytesReceived = 0;                     //index counting the received bytes; Reading data from PC stops when bytesReceived == BUFFERSIZE
bool readInProgress = false;                
bool sequenceReadInProgress = false;
bool newDataFromPC = false;
bool dataTransmissionFinished = false;

//Definitions for Manual und Buffered Mode:
const char MANUALMODEMARKER = 'M';
const char BUFFEREDMODEMARKER = 'B';
const char AMPLITUDEMODEMARKER = 'A';
bool transitionToBuffered = true;
bool transitionToManual = false;
bool manualMode = false;
bool bufferedMode = false;
bool amplitudeMode = false;


void setup()
{
  pinMode(TCXO_POWER_PIN, OUTPUT);
  pinMode(TCXO_PATH_PIN, OUTPUT);
  pinMode(EXTERANL_SRC_PATH_PIN, OUTPUT);
  digitalWrite(TCXO_POWER_PIN, LOW);
  digitalWrite(TCXO_PATH_PIN, LOW);
  digitalWrite(EXTERANL_SRC_PATH_PIN, LOW);

  pinMode(TRG_IN_PIN, INPUT);
  pinMode(TRG_LED_PIN, OUTPUT);
  digitalWrite(TRG_LED_PIN, LOW);

  // todo: do we need to set it up here
  display = new Display();
  display->displayHello();
  delay(3000);

  Serial.begin(115200);

  // Serial.println(F("DDS AD9910 Arduino Shield v3.5 by GRA & AFCH. (gra-afch.com)"));
  // Serial.print(F("Firmware v.:"));
  // Serial.println(FIRMWAREVERSION);

  // Serial.print(F("Input voltage = "));
  // Serial.println(getInputVoltage());

  if ((getInputVoltage() < 4) ||
      (getInputVoltage() > 8)) 
      {
        display->displayPowerWarning();
        while(1);
      }

  modeButton.Update();
  if (modeButton.depressed == true) //если при включении была зажата ручка энкодера, то затираем управляющие флаги в EEPROM, которые восстановят заводские значения всех параметров
  {
    EEPROM.write(CLOCK_SETTINGS_FLAG_ADR, 255); //flag that force save default clock settings to EEPROM
    EEPROM.write(MAIN_SETTINGS_FLAG_ADR, 255); //flag that force save default main settings to EEPROM
  }

  LoadMainSettings();
  LoadClockSettings();

  delay(5000); // wait for PLL

  setFrequency(M*1000000L+K*1000L+H);

  //tell the PC we are ready
  Serial.println("<Arduino is ready>");
}

void loop ()
{
  // todo: this code is horrible. Rework!
  //Wait for new instruction starting with correct MARKER:
  if(Serial.available() > 0) {
    char x = Serial.read();
    if (x==MANUALMODEMARKER) {
      manualMode=true;
      bufferedMode=false;
    } else if (x==BUFFEREDMODEMARKER) {
      manualMode=false;
      bufferedMode=true;
      transitionToBuffered=true;
    } else if (x==AMPLITUDEMODEMARKER){
      //this is not really a mode but makes implementation easier
      manualMode=false;
      bufferedMode=false;
      amplitudeMode=true;
    }
  }
  
  // Start buffered Mode:
  while (bufferedMode == true) {
    
    //transition_to_buffered:
    while (transitionToBuffered == true) { //loop not necessary, left for conceptual reasons
      getDataFromPC(bufferedFrequencies);
      replyToPC();
    }
    
    //Shot ongoing:
    while (dataTransmissionFinished == true) {
      outputBufferedFrequencies();
    }
    
    //transition_to_manual:
    while (transitionToManual == true) {
      sendFinishtoPC();
      transitionToManual = false;
      bufferedMode=false;
      manualMode=false;
    }
  }
  
  //Start manual Mode:
  while (manualMode == true) {
    getDataFromPC(bufferedFrequencies);
    replyToPC();
    
    //Set frequency after all data has been received:
    if (dataTransmissionFinished == true) {
      setFrequency(bufferedFrequencies[0]);
      SaveMainSettings();
      dataTransmissionFinished = false;
      bufferedMode=false; 
      manualMode=false;
    }
  }

  //Start amplitude Mode:
  while(amplitudeMode == true) {
    // todo: remove. we always use 0dbm
    getAmpFromPC();
    replyToPC();
    //Set amplitude after all data has been received:
    if (dataTransmissionFinished == true) {
      //Serial.print(amp);
      dataTransmissionFinished = false;
      amplitudeMode=false;
      }
  }
}

// This function is only used for getting the amplitude from the pc
void getAmpFromPC(){
  
   // receive data from PC and save it into inputBuffer  
  if(Serial.available() > 0) {

    char x = Serial.read();
    // the order of the IF clauses is important!    
      if (x == ENDMARKER) {
        newDataFromPC = true;
        readInProgress = false;
      }

      if (readInProgress == true) {
        
        if (x == ENDMARKERSEQUENCE) {
          sequenceReadInProgress = false;
          inputBuffer[bytesReceived] = 0;
        }
        
        if(sequenceReadInProgress) {
          inputBuffer[bytesReceived] = x;
          bytesReceived ++;
          if (bytesReceived == BUFFERSIZE) {
            bytesReceived = BUFFERSIZE - 1;
          }
        }
    
        if (x == STARTMARKERSEQUENCE) { 
          bytesReceived = 0; 
          sequenceReadInProgress = true;
        }
      }

      if (x == STARTMARKER) {
        readInProgress = true;
        arrReadIndex=0;
      }
        
  }
}

void outputBufferedFrequencies() {
  while(1) {
    if (digitalRead(TRG_IN_PIN))
    {
      setFrequency(bufferedFrequencies[arrWriteIndex]);

      arrWriteIndex++;
      //Stop execution when we are at the end of the array
      //arrReadIndex has to be reduced by one as during data transmission it is increased +1 after last received dataset.
      if (arrWriteIndex >= arrReadIndex-1){
        transitionToManual = true;
        dataTransmissionFinished = false;  //Comment out for test purposes with labscript at home
        arrWriteIndex = 0;
        SaveMainSettings(); // save last setting
      }
      // wait for trigger low
      while (digitalRead(TRG_IN_PIN)) {}
      return;
    }
    // if (Serial.available() > 0)
    // {
    //   char x = Serial.read();
    //   // assume something happened... like abort...
    //   // todo: actually check for an abort marker
    //   transitionToManual = true;
    //   dataTransmissionFinished = false;  //Comment out for test purposes with labscript at home
    //   arrWriteIndex = 0;
    //   // todo: should we now continue with the last frequency?
    //   break;
    // }
  }
}

// Get frequency data from PC:
void getDataFromPC(uint32_t* data_array) {
  
  // receive data from PC and save it into inputBuffer  
  if(Serial.available() > 0) {

    char x = Serial.read();
    // the order of the IF clauses is important!    
      if (x == ENDMARKER) {
        newDataFromPC = true;
        readInProgress = false;
      }

      if (readInProgress == true) {
        
        uint32_t frequency;
        if (x == ENDMARKERSEQUENCE) {
          sequenceReadInProgress = false;
          inputBuffer[bytesReceived] = 0;
          frequency = atol(inputBuffer);     // convert inputBuffer string to an frequency integer 
          data_array[arrReadIndex] = frequency;
          arrReadIndex +=1;
        }
        
        if(sequenceReadInProgress) {
          inputBuffer[bytesReceived] = x;
          bytesReceived ++;
          if (bytesReceived == BUFFERSIZE) {
            bytesReceived = BUFFERSIZE - 1;
          }
        }
    
        if (x == STARTMARKERSEQUENCE) { 
          bytesReceived = 0; 
          sequenceReadInProgress = true;
        }
      }

      if (x == STARTMARKER) {
        readInProgress = true;
        arrReadIndex=0;
      }
        
  }
}

void replyToPC() {
  if (newDataFromPC) {
    newDataFromPC = false;
    Serial.print("<");
    Serial.print("Profiles received: ");
    Serial.print(arrReadIndex);
    Serial.println(">");
    dataTransmissionFinished = true;
  }
}

void sendFinishtoPC() {
  Serial.print("<");
  Serial.print("Shot finished");
  Serial.println(">");
}

void setFrequency(uint32_t fq) {
  SingleProfileFreqOut(fq, A * -1);

  M = fq / 1000000L;
  K = (fq-M*1000000L) / 1000L;
  H = fq-M*1000000L-K*1000L;
  display->displayMainMenu(M, K, H, A);
}

/*****************************************************************

 * **************************************************************/

void SaveMainSettings()
{
  EEPROM.put(M_ADR, M);
  EEPROM.put(K_ADR, K);
  EEPROM.put(H_ADR, H);
  EEPROM.put(A_ADR, A);
  EEPROM.write(MAIN_SETTINGS_FLAG_ADR, 55);
}

/**************************************************************************

 *************************************************************************/
void LoadMainSettings()
{
  if (EEPROM.read(MAIN_SETTINGS_FLAG_ADR) != 55)
  {
    M = INIT_M;
    K = INIT_K;
    H = INIT_H;
    A = INIT_A;
    SaveMainSettings();
#if DBG==1
    Serial.println(F("Loading init values"));
    Serial.print("M=");
    Serial.println(M);
    Serial.print("K=");
    Serial.println(K);
    Serial.print("H=");
    Serial.println(H);
    Serial.print("A=");
    Serial.println(A);
#endif
  }
  else
  {
    EEPROM.get(M_ADR, M);
    EEPROM.get(K_ADR, K);
    EEPROM.get(H_ADR, H);
    EEPROM.get(A_ADR, A);
#if DBG==1
    Serial.println(F("Value from EEPROM"));
    Serial.print("M=");
    Serial.println(M);
    Serial.print("K=");
    Serial.println(K);
    Serial.print("H=");
    Serial.println(H);
    Serial.print("A=");
    Serial.println(A);
#endif
  }
}

float getInputVoltage()
{
  uint16_t voltage=0; //mV
  analogReference(DEFAULT); 
  delay(10);
  voltage = map(analogRead(INPUT_VOLTAGE_PIN), 0, 1023, 0, ANALOG_REFERENCE_VOLTAGE); //4.95
  return float(voltage) / 1000.0 / VOLTAGE_DIVIDER_FACTOR;
}

void selectClockSrcPath(uint8_t path)
{
  switch (path)
  {
    case CLOCK_SOURCE_XO_INDEX:
      digitalWrite(TCXO_POWER_PIN, HIGH);
      digitalWrite(TCXO_PATH_PIN, LOW);
      digitalWrite(EXTERANL_SRC_PATH_PIN, LOW);
    break;
    case CLOCK_SOURCE_TCXO_INDEX:
      digitalWrite(TCXO_POWER_PIN, HIGH);
      digitalWrite(EXTERANL_SRC_PATH_PIN, LOW);
      digitalWrite(TCXO_PATH_PIN, HIGH);
    break;
    case CLOCK_SOURCE_EXT_TCXO_INDEX:
      digitalWrite(TCXO_POWER_PIN, LOW);
      digitalWrite(TCXO_PATH_PIN, LOW);
      digitalWrite(EXTERANL_SRC_PATH_PIN, HIGH);
    break;
    case CLOCK_SOURCE_EXT_OSC_INDEX:
      digitalWrite(TCXO_POWER_PIN, LOW);
      digitalWrite(TCXO_PATH_PIN, LOW);
      digitalWrite(EXTERANL_SRC_PATH_PIN, HIGH);
    break;    
  }
}

