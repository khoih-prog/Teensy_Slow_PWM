/****************************************************************************************************************************
  multiFileProject.ino
  
  For nRF52-based boards using Adafruit_nRF52_Arduino core
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/nRF52_Slow_PWM
  Licensed under MIT license
*****************************************************************************************************************************/

// To demo how to include files in multi-file Projects

#if !( defined(CORE_TEENSY) || defined(TEENSYDUINO) )
  #error This code is designed to run on Teensy platform! Please check your Tools->Board setting.
#endif

#define TEENSY_SLOW_PWM_VERSION_MIN_TARGET      F("Teensy_Slow_PWM v1.2.0")
#define TEENSY_SLOW_PWM_VERSION_MIN             1002000

#include "multiFileProject.h"

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "Teensy_Slow_PWM.h"

void setup() 
{
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("\nStart multiFileProject");
  Serial.println(TEENSY_SLOW_PWM_VERSION);

#if defined(TEENSY_SLOW_PWM_VERSION_MIN)
  if (TEENSY_SLOW_PWM_VERSION_INT < TEENSY_SLOW_PWM_VERSION_MIN)
  {
    Serial.print("Warning. Must use this example on Version equal or later than : ");
    Serial.println(TEENSY_SLOW_PWM_VERSION_MIN_TARGET);
  }
#endif
}

void loop() 
{
  // put your main code here, to run repeatedly:
}
