/****************************************************************************************************************************
  Teensy_Slow_PWM_ISR.h
  For Teensy boards (Teensy 2.x, Teensy LC, Teensy 3.x, Teensy 4.x, etc.)
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/Teensy_Slow_PWM
  Licensed under MIT license
  
  Now even you use all these new 16 ISR-based timers,with their maximum interval practically unlimited (limited only by
  unsigned long miliseconds), you just consume only one  RP2040-based timer and avoid conflicting with other cores' tasks.
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.

  Version: 1.2.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K.Hoang      28/09/2021 Initial coding for Teensy 2.x, Teensy LC, Teensy 3.x, Teensy 4.x, etc.
  1.1.0   K Hoang      10/11/2021 Add functions to modify PWM settings on-the-fly
  1.2.0   K.Hoang      07/02/2022 Fix `multiple-definitions` linker error. Improve accuracy. Optimize code. Fix bug
*****************************************************************************************************************************/

#pragma once

#ifndef TEENSY_SLOW_PWM_ISR_H
#define TEENSY_SLOW_PWM_ISR_H


#include "Teensy_Slow_PWM_ISR.hpp"
#include "Teensy_Slow_PWM_ISR_Impl.h"

#endif    // TEENSY_SLOW_PWM_ISR_H


