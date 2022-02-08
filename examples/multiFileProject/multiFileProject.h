/****************************************************************************************************************************
  multiFileProject.h
  
  For nRF52-based boards using Adafruit_nRF52_Arduino core
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/nRF52_Slow_PWM
  Licensed under MIT license
*****************************************************************************************************************************/

// To demo how to include files in multi-file Projects

#pragma once

#define USING_MICROS_RESOLUTION       true    //false 

// Default is true, uncomment to false
//#define CHANGING_PWM_END_OF_CYCLE     false

// Can be included as many times as necessary, without `Multiple Definitions` Linker Error
#include "Teensy_Slow_PWM.hpp"
