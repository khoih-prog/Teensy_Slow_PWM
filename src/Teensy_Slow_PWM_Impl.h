/****************************************************************************************************************************
  Teensy_Slow_PWM_Impl.h
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

#ifndef TEENSY_SLOW_PWM_IMPL_H
#define TEENSY_SLOW_PWM_IMPL_H

#if ( defined(__arm__) && defined(TEENSYDUINO) && defined(__IMXRT1062__) )

  // Nothing here yet

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// For Teensy 3.x and LC
#elif defined(__arm__) && defined(TEENSYDUINO) && (defined(KINETISK) || defined(KINETISL))

  void ftm1_isr()
  {
    uint32_t sc = FTM1_SC;
    
  #ifdef KINETISL
    if (sc & 0x80) 
      FTM1_SC = sc;
  #else
    if (sc & 0x80) 
      FTM1_SC = sc & 0x7F;
  #endif

    (*(TeensyTimers[TEENSY_TIMER_1]->getCallback()))();
  }

  void ftm2_isr()
  {
    uint32_t sc = FTM2_SC;
    
  #ifdef KINETISL
    if (sc & 0x80) 
      FTM2_SC = sc;
  #else
    if (sc & 0x80) 
      FTM2_SC = sc & 0x7F;
  #endif

    (*(TeensyTimers[TEENSY_TIMER_3]->getCallback()))();
  }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// For Teensy 2.0 and Teensy++ 2.0

#elif ( defined(ARDUINO_ARCH_AVR) || defined(__AVR__) )

  ISR(TIMER1_OVF_vect)
  {
    (*(TeensyTimers[TEENSY_TIMER_1]->getCallback()))();
  }

  ISR(TIMER3_OVF_vect)
  {
    (*(TeensyTimers[TEENSY_TIMER_3]->getCallback()))();
  }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#else

  #error Not support board
  
#endif


////////////////////////////////////////////////////////////////////////////////



#endif    // TEENSY_SLOW_PWM_IMPL_H

