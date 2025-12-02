// code for sweeps. may be nice later on...


// Sweep(GetSweepStartFreq(), GetSweepEndFreq(), SweepTime, SweepTimeFormat);

//**********
// Return SweepStartFreq in HZ
//**********
uint32_t GetSweepStartFreq()
{
#if DBG==1
  Serial.print("SweepStartFreqM=");
  Serial.println(SweepStartFreqM);
  Serial.print("SweepStartFreqK=");
  Serial.println(SweepStartFreqK);
  Serial.print("SweepStartFreqH=");
  Serial.println(SweepStartFreqH);
#endif
  return SweepStartFreqM * 1000000UL + SweepStartFreqK * 1000UL + SweepStartFreqH;
}

//**********
// Return SweepEndFreq in HZ
//**********
uint32_t GetSweepEndFreq()
{
  return SweepEndFreqM * 1000000UL + SweepEndFreqK * 1000UL + SweepEndFreqH;
}

void SetSweepStartFreq(uint32_t freq)
{
  SweepStartFreqH = freq % 1000;
  freq = freq / 1000;
  SweepStartFreqK = freq % 1000;
  freq = freq / 1000;
  SweepStartFreqM = freq;
}

void SetSweepEndFreq(uint32_t freq)
{
  SweepEndFreqH = freq % 1000;
  freq = freq / 1000;
  SweepEndFreqK = freq % 1000;
  freq = freq / 1000;
  SweepEndFreqM = freq;
}

bool IsSweepFreqsValid() //проверяет правильность введеных частот для свипа, и в случае обнаружения проблем исправляет их, в том числе меняет местами началььную и конечные частоты если, начальная больше конечной
{
  bool IsValid = true;
  uint32_t tempFreq;

  if (GetSweepStartFreq() > GetSweepEndFreq())
  {
    tempFreq = GetSweepStartFreq();
    SetSweepStartFreq(GetSweepEndFreq());
    SetSweepEndFreq(tempFreq);
    display->displayMessage("SWEEP", "Starting Frequency\r\n Higher Than\r\n Stop Frequency!"); //\r\n
    delay(3000);
    IsValid = false;
  }

  if (GetSweepStartFreq() > MAX_SWEEP_FREQ)
  {
    SetSweepStartFreq(MAX_SWEEP_FREQ);
    display->displayMessage("SWEEP", "Too High\r\n Start Frequency!");
    delay(2500);
    //ModMenuPos=MOD_MENU_SWEEP_START_FREQ_M_INDEX;
    IsValid = false;
  }

  if (GetSweepStartFreq() < MIN_SWEEP_FREQ)
  {
    SetSweepStartFreq(MIN_SWEEP_FREQ);
    display->displayMessage("SWEEP", "Too Low\r\n Start Frequency!");
    delay(2500);
    //ModMenuPos=MOD_MENU_SWEEP_START_FREQ_M_INDEX;
    IsValid = false;
  }

  if (GetSweepEndFreq() > MAX_SWEEP_FREQ)
  {
    SetSweepEndFreq(MAX_SWEEP_FREQ);
    display->displayMessage("SWEEP", "Too High\r\n Stop Frequency!");
    delay(2500);
    //ModMenuPos=MOD_MENU_SWEEP_END_FREQ_M_INDEX;
    IsValid = false;
  }

  if (GetSweepEndFreq() < MIN_SWEEP_FREQ)
  {
    SetSweepEndFreq(MIN_SWEEP_FREQ);
    display->displayMessage("SWEEP", "Too Low\r\n Stop Frequency!");
    delay(2500);
    //ModMenuPos=MOD_MENU_SWEEP_END_FREQ_M_INDEX;
    IsValid = false;
  }

  if (GetSweepStartFreq() == GetSweepEndFreq())
  {
    //SetSweepEndFreq(MIN_SWEEP_FREQ);
    display->displayMessage("SWEEP", "Frequencies\r\n are Equal!");
    delay(2500);
    IsValid = false;
  }
  return IsValid;
}

bool IsSweepTimeTooLong()
{
  //GetSweepStartFreq(), GetSweepEndFreq(), SweepTime, SweepTimeFormat
  uint32_t DeltaFTW = FreqToFTW(GetSweepEndFreq()) - FreqToFTW(GetSweepStartFreq());
  //float GHZ_CoreClock=DDS_Core_Clock/1E9; //незабыть заменит на CalcRealDDSCoreClockFromOffset();
  float GHZ_CoreClock = CalcRealDDSCoreClockFromOffset() / 1E9;
  uint64_t MaxPossibleNanoSweepTime = (4 / GHZ_CoreClock * DeltaFTW) * 0xFFFF; //умножаем на 0xFFFF (максимальное значение StepRate) для того чтобы узнать максиамльно возможное время свипа для заданного интервала частот (при текущей частоте ядра)
#if DBG==1
  Serial.print("MaxPossibleNanoSweepTime=");
  print64(MaxPossibleNanoSweepTime);
  Serial.print("GetSweepTime()=");
  print64(GetSweepTime());
#endif
  if (GetSweepTime() > MaxPossibleNanoSweepTime) //проверяем чтобы не получилось так чтобы введеное время не оказалось дольше максимально возможного для заданного диапазона частот и текущей частоты ядра
  {
    SetSweepTime(MaxPossibleNanoSweepTime);
    display->displayMessage("SWEEP", F("Too Long Time"));
    delay(2500);
    return true;
  }
  else
  {
    return false;
  }
}

uint64_t GetSweepTime() // возвращает время в наносекунадх указанное пользователем в меню
{
  if (SweepTimeFormat == 0) return SweepTime * 1E9;
  else if (SweepTimeFormat == 1) return SweepTime * 1E6;
  else if (SweepTimeFormat == 2) return SweepTime * 1E3;
}

void SetSweepTime(uint64_t NanoSweepTime)
{
  if (NanoSweepTime < 1E3) //1000
  {
    SweepTime = NanoSweepTime;
    SweepTimeFormat = 3; //nS
  } else if (NanoSweepTime < 1E6) //1000000
  {
    SweepTime = NanoSweepTime / 1E3;
    SweepTimeFormat = 2; //uS
  } else if (NanoSweepTime < 1E9) //1000000000
  {
    SweepTime = NanoSweepTime / 1E6;
    SweepTimeFormat = 1; //uS
  } else
  {
    SweepTime = NanoSweepTime / 1E9;
    SweepTimeFormat = 0; //S
  }
}