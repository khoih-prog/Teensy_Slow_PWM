/****************************************************************************************************************************
  Teensy_Slow_PWM.h
  For Teensy boards (Teensy 2.x, Teensy LC, Teensy 3.x, Teensy 4.x, etc.)
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/Teensy_Slow_PWM
  Licensed under MIT license
  
  Now even you use all these new 16 ISR-based timers,with their maximum interval practically unlimited (limited only by
  unsigned long miliseconds), you just consume only one  RP2040-based timer and avoid conflicting with other cores' tasks.
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.

  Version: 1.1.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K.Hoang      28/09/2021 Initial coding for Teensy 2.x, Teensy LC, Teensy 3.x, Teensy 4.x, etc.
  1.1.0   K Hoang      10/11/2021 Add functions to modify PWM settings on-the-fly
*****************************************************************************************************************************/

#pragma once

#ifndef TEENSY_SLOW_PWM_H
#define TEENSY_SLOW_PWM_H

#if !( defined(CORE_TEENSY) || defined(TEENSYDUINO) )
  #error This code is designed to run on Teensy platform! Please check your Tools->Board setting.
#endif

#ifndef TEENSY_SLOW_PWM_VERSION
  #define TEENSY_SLOW_PWM_VERSION       F("Teensy_Slow_PWM v1.1.0")
#endif

#ifndef _PWM_LOGLEVEL_
  #define _PWM_LOGLEVEL_        1
#endif

#include "PWM_Generic_Debug.h"

class TeensyTimerInterrupt;

typedef enum
{
  TEENSY_TIMER_1 = 0,
  TEENSY_TIMER_3 = 1,
  TEENSY_MAX_TIMER
} TeensyTimerNumber;
  
typedef TeensyTimerInterrupt TeensyTimer;

typedef void (*timerCallback)  ();


#if ( defined(__arm__) && defined(TEENSYDUINO) && defined(__IMXRT1062__) )

  // For Teensy 4.0/4.1
   
  #ifndef BOARD_NAME
    #if defined(ARDUINO_TEENSY41)
      #define BOARD_NAME          F("Teensy 4.1")
      #warning Using Teensy 4.1
    #elif defined(ARDUINO_TEENSY_MICROMOD)
      #define BOARD_NAME          F("Teensy MicroMod")
      #warning Using Teensy MicroMod
    #else
      #define BOARD_NAME          F("Teensy 4.0")
      #warning Using Teensy 4.0
    #endif
      
  #endif
  
class TeensyTimerInterrupt;
void ext_isr();

// Ony for Teensy 4.0/4.1
IRQ_NUMBER_t            teensy_timers_irq [TEENSY_MAX_TIMER] = { IRQ_FLEXPWM1_3, IRQ_FLEXPWM2_2 };

TeensyTimerInterrupt*   TeensyTimers      [TEENSY_MAX_TIMER] = { NULL, NULL };

class TeensyTimerInterrupt
{
  private:
    // properties
    //static unsigned short pwmPeriod;
    //static unsigned char clockSelectBits;
    
    uint8_t               _timer       = TEENSY_TIMER_1;
       
    IRQ_NUMBER_t          _timer_IRQ;

    timerCallback         _callback;        // pointer to the callback function
    
    float                 _frequency;       // Timer frequency
    uint32_t              _timerCount;      // count to activate timer
    
    uint32_t              _prescale = 0;
    uint32_t              _realPeriod;
            
  public:
  
    TeensyTimerInterrupt(uint8_t timer = TEENSY_TIMER_1) __attribute__((always_inline))
    {
      if (timer < TEENSY_MAX_TIMER)
        _timer = timer;
      else
      {
        // Error out of range, force to use TEENSY_TIMER_1
        _timer = TEENSY_TIMER_1;
      }
      
      _timer_IRQ  = teensy_timers_irq[_timer];
      
      // Update to use in IRQHandler
      TeensyTimers[_timer] = this;
      
      _callback = NULL;
    }
    
    ~TeensyTimerInterrupt() __attribute__((always_inline))
    {
      TeensyTimers[_timer] = NULL;
    }
    
    // frequency (in hertz) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    // No params and duration now. To be added in the future by adding similar functions here or to NRF52-hal-timer.c
    bool setFrequency(float frequency, timerCallback callback) __attribute__((always_inline))
    {
      return setInterval((float) (1000000.0f / frequency), callback);
    }
    
    // Interval (in microseconds)
    // For Teensy 4.0/4.1, F_BUS_ACTUAL = 150 MHz => max interval/period is only 55922 us (~17.9 Hz)
    bool setInterval(unsigned long interval, timerCallback callback) __attribute__((always_inline))
    {     
      // This function will be called when time out interrupt will occur
      if (callback) 
      {
          _callback = callback;
      } 
      else
      {
          PWM_LOGERROR(F("TeensyTimerInterrupt: ERROR: NULL callback function pointer."));
          
          return false;
      }
      
      uint32_t period = ((float) F_BUS_ACTUAL / 2000000) * (float) interval;
      uint32_t prescale = 0;

      while (period > 32767)
      {
        period = period >> 1;

        if (++prescale > 7)
        {
          prescale = 7;   
          period = 32767; 
          break;
        }
      }
      
      // when F_BUS is 150 MHz, longest period (_realPeriod) is 55922.3467 us (~17.881939 Hz)
      // 55922.3467 us = (32767 * 2000000) / (150000000 >> 7)
      _realPeriod = (uint32_t) ((period * 2000000.0f) / (F_BUS_ACTUAL >> prescale));
      _prescale   = prescale;
      _timerCount = period;

      if (_timer == TEENSY_TIMER_1)
      {
        PWM_LOGWARN1(F("TEENSY_TIMER_1: , F_BUS_ACTUAL (MHz) ="), F_BUS_ACTUAL/1000000);
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        PWM_LOGWARN1(F("TEENSY_TIMER_3: , F_BUS_ACTUAL (MHz) ="), F_BUS_ACTUAL/1000000);
      }
          
      PWM_LOGWARN3(F("Request interval ="), interval, F(", actual interval (us) ="), _realPeriod);
      PWM_LOGWARN3(F("Prescale ="), _prescale, F(", _timerCount ="), _timerCount);
        
      if (_timer == TEENSY_TIMER_1)
      {
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        FLEXPWM1_FCTRL0    |= FLEXPWM_FCTRL0_FLVL(8); // logic high = fault
        FLEXPWM1_FSTS0      = 0x0008; // clear fault status
        FLEXPWM1_MCTRL     |= FLEXPWM_MCTRL_CLDOK(8);
        FLEXPWM1_SM3CTRL2   = FLEXPWM_SMCTRL2_INDEP;
        FLEXPWM1_SM3CTRL    = FLEXPWM_SMCTRL_HALF | FLEXPWM_SMCTRL_PRSC(prescale);
        FLEXPWM1_SM3INIT    = -period;
        FLEXPWM1_SM3VAL0    = 0;
        FLEXPWM1_SM3VAL1    = period;
        FLEXPWM1_SM3VAL2    = 0;
        FLEXPWM1_SM3VAL3    = 0;
        FLEXPWM1_SM3VAL4    = 0;
        FLEXPWM1_SM3VAL5    = 0;
        FLEXPWM1_MCTRL     |= FLEXPWM_MCTRL_LDOK(8) | FLEXPWM_MCTRL_RUN(8);
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        FLEXPWM2_FCTRL0    |= FLEXPWM_FCTRL0_FLVL(4); // logic high = fault
	      FLEXPWM2_FSTS0      = 0x0008; // clear fault status
	      FLEXPWM2_MCTRL     |= FLEXPWM_MCTRL_CLDOK(4);
	      FLEXPWM2_SM2CTRL2   = FLEXPWM_SMCTRL2_INDEP;
	      FLEXPWM2_SM2CTRL    = FLEXPWM_SMCTRL_HALF | FLEXPWM_SMCTRL_PRSC(prescale);
	      FLEXPWM2_SM2INIT    = -period;
	      FLEXPWM2_SM2VAL0    = 0;
	      FLEXPWM2_SM2VAL1    = period;
	      FLEXPWM2_SM2VAL2    = 0;
	      FLEXPWM2_SM2VAL3    = 0;
	      FLEXPWM2_SM2VAL4    = 0;
	      FLEXPWM2_SM2VAL5    = 0;
	      FLEXPWM2_MCTRL     |= FLEXPWM_MCTRL_LDOK(4) | FLEXPWM_MCTRL_RUN(4);       
      }
      
      attachInterruptVector(_timer_IRQ, &ext_isr);
      
      if (_timer == TEENSY_TIMER_1)
      {
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        FLEXPWM1_SM3STS = FLEXPWM_SMSTS_RF;
        FLEXPWM1_SM3INTEN = FLEXPWM_SMINTEN_RIE;
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        FLEXPWM2_SM2STS = FLEXPWM_SMSTS_RF;
	      FLEXPWM2_SM2INTEN = FLEXPWM_SMINTEN_RIE;
      }
      
      NVIC_ENABLE_IRQ(_timer_IRQ);
      
      return true;
    }
  
    bool attachInterrupt(float frequency, timerCallback callback) __attribute__((always_inline))
    {
      return setInterval((float) (1000000.0f / frequency), callback);
    }

    // interval (in microseconds) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    // No params and duration now. To be added in the future by adding similar functions here or to NRF52-hal-timer.c
    bool attachInterruptInterval(unsigned long interval, timerCallback callback) __attribute__((always_inline))
    {
      return setInterval(interval, callback);
    }
    
    void detachInterrupt() __attribute__((always_inline))
    {     
      NVIC_DISABLE_IRQ(_timer_IRQ);
          
      if (_timer == TEENSY_TIMER_1)
      {
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        FLEXPWM1_SM3INTEN = 0;
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        FLEXPWM2_SM2INTEN = 0;
      }   
    }

    void disableTimer() __attribute__((always_inline))
    {
      detachInterrupt();
    }
    
    //****************************
    //  Run Control
    //****************************
    void startTimer() __attribute__((always_inline)) __attribute__((always_inline))
    {
      stopTimer();
      resumeTimer();
    }

    void stopTimer() __attribute__((always_inline))
    {
      if (_timer == TEENSY_TIMER_1)
      {          
        PWM_LOGWARN(F("TeensyTimerInterrupt:stopTimer TEENSY_TIMER_1"));
      
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        FLEXPWM1_MCTRL &= ~FLEXPWM_MCTRL_RUN(8);
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:stopTimer TEENSY_TIMER_3"));
    
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        FLEXPWM2_MCTRL &= ~FLEXPWM_MCTRL_RUN(4);    
      }
    }

    void restartTimer() __attribute__((always_inline))
    {
      startTimer();
    }

    void resumeTimer() __attribute__((always_inline))
    {
      
      if (_timer == TEENSY_TIMER_1)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:resumeTimer TEENSY_TIMER_1"));
      
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_RUN(8);
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:resumeTimer TEENSY_TIMER_3"));
       
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        FLEXPWM2_MCTRL |= FLEXPWM_MCTRL_RUN(4);  
      }
    }

    uint32_t getPeriod() __attribute__((always_inline))
    {
      return _timerCount;
    }
    
    uint32_t getPrescale() __attribute__((always_inline))
    {
      return _prescale;
    }
    
    // Real (actual) period in us
    uint32_t getRealPeriod() __attribute__((always_inline))
    {
      return _realPeriod;
    }
  
    timerCallback getCallback() __attribute__((always_inline))
    {
      return _callback;
    }
    
    IRQ_NUMBER_t getTimerIRQn() __attribute__((always_inline))
    {
      return _timer_IRQ;
    }
};

void ext_isr()
{
  if (TeensyTimers[TEENSY_TIMER_1])
  {
    ///////////// TEENSY_TIMER_1 code ////////////////////////     
    FLEXPWM1_SM3STS = FLEXPWM_SMSTS_RF;
    (*(TeensyTimers[TEENSY_TIMER_1]->getCallback()))();
  }
  else if (TeensyTimers[TEENSY_TIMER_3])
  {
    ///////////// TEENSY_TIMER_3 code ////////////////////////
    FLEXPWM2_SM2STS = FLEXPWM_SMSTS_RF;
    (*(TeensyTimers[TEENSY_TIMER_3]->getCallback()))();
  }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// For Teensy 3.x and LC
#elif defined(__arm__) && defined(TEENSYDUINO) && (defined(KINETISK) || defined(KINETISL))

  #ifdef BOARD_NAME
      #undef BOARD_NAME
  #endif
    
  #if defined(__MK66FX1M0__)
    // For Teensy 3.6
    #warning Using Teensy 3.6
    #define BOARD_NAME  F("Teensy 3.6")
  #elif defined(__MK64FX512__)
    // For Teensy 3.5
    #warning Using Teensy 3.5
    #define BOARD_NAME  F("Teensy 3.5")
  #elif defined(__MK20DX256__)
    // For Teensy 3.2/3.1
    #warning Using Teensy 3.2/3.1
    #define BOARD_NAME  F("Teensy 3.2/3.1")
  #elif defined(__MK20DX128__)
    // For Teensy 3.0
    #warning Using Teensy 3.0
    #define BOARD_NAME  F("Teensy 3.0")
  #elif defined(__MKL26Z64__)
    // For Teensy LC
    #warning Using Teensy LC
    #define BOARD_NAME  F("Teensy LC")
  #endif

  #if defined(KINETISK)
    #define F_TIMER F_BUS
  #elif defined(KINETISL)
    #define F_TIMER (F_PLL/2)
  #endif

  // Teensy 3.0 has only TEENSY_TIMER_1 and IRQ_FTM1, So force to use TEENSY_TIMER_1 and IRQ_FTM1
  #if defined(IRQ_FTM2)
    IRQ_NUMBER_t          teensy_timers_irq [TEENSY_MAX_TIMER] = { IRQ_FTM1, IRQ_FTM2 };
  #else
    IRQ_NUMBER_t          teensy_timers_irq [TEENSY_MAX_TIMER] = { IRQ_FTM1, IRQ_FTM1 };
  #endif  

  TeensyTimerInterrupt*   TeensyTimers      [TEENSY_MAX_TIMER] = { NULL, NULL };

class TeensyTimerInterrupt
{
  // Use only 15 bit resolution.  From K66 reference manual, 45.5.7 page 1200:
  //   The CPWM pulse width (duty cycle) is determined by 2 x (CnV - CNTIN) and the
  //   period is determined by 2 x (MOD - CNTIN). See the following figure. MOD must be
  //   kept in the range of 0x0001 to 0x7FFF because values outside this range can produce
  //   ambiguous results.
#undef  TIMER_RESOLUTION
#define TIMER_RESOLUTION    32768
   
  private:
   
    uint8_t               _timer       = TEENSY_TIMER_1;
       
    IRQ_NUMBER_t          _timer_IRQ;

    timerCallback         _callback;        // pointer to the callback function
    
    float                 _frequency;       // Timer frequency
    uint32_t              _timerCount;      // count to activate timer
    
    uint32_t              _prescale = 0;
    uint32_t              _realPeriod;
            
  public:
  
    TeensyTimerInterrupt(uint8_t timer = TEENSY_TIMER_1) __attribute__((always_inline))
    {
    
#if defined(IRQ_FTM2)
    
      if (timer < TEENSY_MAX_TIMER)
        _timer = timer;
      else
      {
        // Error out of range, force to use TEENSY_TIMER_1
        _timer = TEENSY_TIMER_1;
      }
#else
      // Teensy 3.0 has only TEENSY_TIMER_1 and IRQ_FTM1, So force to use TEENSY_TIMER_1 and IRQ_FTM1
      _timer = TEENSY_TIMER_1;
      
#endif
      
      _timer_IRQ  = teensy_timers_irq[_timer];
      
      // Update to use in IRQHandler
      TeensyTimers[_timer] = this;
      
      _callback = NULL;
    }
    
    ~TeensyTimerInterrupt() __attribute__((always_inline))
    {
      TeensyTimers[_timer] = NULL;
    }
    
    // frequency (in hertz) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    // No params and duration now. To be added in the future by adding similar functions here or to NRF52-hal-timer.c
    bool setFrequency(float frequency, timerCallback callback) __attribute__((always_inline))
    {
      return setInterval((float) (1000000.0f / frequency), callback);
    }
    
    // Interval (in microseconds)
    bool setInterval(unsigned long interval, timerCallback callback) __attribute__((always_inline))
    {     
      // This function will be called when time out interrupt will occur
      if (callback) 
      {
          _callback = callback;
      } 
      else
      {
          PWM_LOGERROR(F("TeensyTimerInterrupt: ERROR: NULL callback function pointer."));
          
          return false;
      }
      
      uint32_t period   = ((float) F_TIMER / 2000000) * (float) interval;
      uint32_t prescale = 0;
      
#if 1

      while (period >= TIMER_RESOLUTION ) 
      {
		    period = period >> 1;
		    
		    if (++prescale > 7) 
		    {
			    prescale = 7;
			    period   = (TIMER_RESOLUTION - 1);
			    break;
		    }
	    }

#else

      if (period < TIMER_RESOLUTION) 
      {
	      prescale = 0;
      } 
      else if (period < TIMER_RESOLUTION * 2) 
      {
	      prescale = 1;
	      period = period >> 1;
      } 
      else if (period < TIMER_RESOLUTION * 4) 
      {
	      prescale = 2;
	      period = period >> 2;
      } 
      else if (period < TIMER_RESOLUTION * 8) 
      {
	      prescale = 3;
	      period = period >> 3;
      } 
      else if (period < TIMER_RESOLUTION * 16) 
      {
	      prescale = 4;
	      period = period >> 4;
      } 
      else if (period < TIMER_RESOLUTION * 32) 
      {
	      prescale = 5;
	      period = period >> 5;
      } 
      else if (period < TIMER_RESOLUTION * 64) 
      {
	      prescale = 6;
	      period = period >> 6;
      } 
      else if (period < TIMER_RESOLUTION * 128) 
      {
	      prescale = 7;
	      period = period >> 7;
      } 
      else 
      {
	      prescale = 7;
	      period = TIMER_RESOLUTION - 1;
      }
      
#endif

      _realPeriod = (uint32_t) ((period * 2000000.0f) / (F_TIMER >> prescale));
	    _prescale = prescale;
	    _timerCount = period;

      if (_timer == TEENSY_TIMER_1)
      {
        PWM_LOGWARN1(F("TEENSY_TIMER_1: , F_TIMER (MHz) ="), F_TIMER/1000000);
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        PWM_LOGWARN1(F("TEENSY_TIMER_3: , F_TIMER (MHz) ="), F_TIMER/1000000);
      }
          
      PWM_LOGWARN3(F("Request interval ="), interval, F(", actual interval (us) ="), _realPeriod);
      PWM_LOGWARN3(F("Prescale ="), _prescale, F(", _timerCount ="), _timerCount);
      
	    
	    if (_timer == TEENSY_TIMER_1)
      {
        ///////////// TEENSY_TIMER_1 code ////////////////////////   
	    
        uint32_t sc = FTM1_SC;
        
        FTM1_SC = 0;
        FTM1_MOD = _realPeriod;
        FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_CPWMS | _prescale | (sc & FTM_SC_TOIE);
        
        attachInterruptVector(_timer_IRQ, &ftm1_isr);
            
        FTM1_SC |= FTM_SC_TOIE;
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        ///////////// TEENSY_TIMER_3 code //////////////////////// 
          
        uint32_t sc = FTM2_SC;
        
	      FTM2_SC = 0;
	      FTM2_MOD = _realPeriod;
	      FTM2_SC = FTM_SC_CLKS(1) | FTM_SC_CPWMS | _prescale | (sc & FTM_SC_TOIE);
	      
	      attachInterruptVector(_timer_IRQ, &ftm2_isr);
        
        FTM2_SC |= FTM_SC_TOIE;
      }
           
      NVIC_ENABLE_IRQ(_timer_IRQ);
      
      return true;
    }
       
    bool attachInterrupt(float frequency, timerCallback callback) __attribute__((always_inline))
    {
      return setInterval((float) (1000000.0f / frequency), callback);
    }

    // interval (in microseconds) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    // No params and duration now. To be added in the future by adding similar functions here or to NRF52-hal-timer.c
    bool attachInterruptInterval(unsigned long interval, timerCallback callback) __attribute__((always_inline))
    {
      return setInterval(interval, callback);
    }
    
    void detachInterrupt() __attribute__((always_inline))
    {     
      NVIC_DISABLE_IRQ(_timer_IRQ);
          
      if (_timer == TEENSY_TIMER_1)
      {
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        FTM1_SC &= ~FTM_SC_TOIE;
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        FTM2_SC &= ~FTM_SC_TOIE;
      }
      
      NVIC_DISABLE_IRQ(_timer_IRQ);
    }

    void disableTimer() __attribute__((always_inline))
    {
      detachInterrupt();
    }
    
    //****************************
    //  Run Control
    //****************************
    void startTimer() __attribute__((always_inline))
    {
      stopTimer();
      
      if (_timer == TEENSY_TIMER_1)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:startTimer TEENSY_TIMER_1"));
           
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        FTM1_CNT = 0;
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:startTimer TEENSY_TIMER_3"));
      
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        FTM2_CNT = 0;
      }
      
      resumeTimer();
    }

    void stopTimer() __attribute__((always_inline))
    {
      if (_timer == TEENSY_TIMER_1)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:stopTimer TEENSY_TIMER_1"));
            
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        FTM1_SC = FTM1_SC & (FTM_SC_TOIE | FTM_SC_CPWMS | FTM_SC_PS(7));
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:stopTimer TEENSY_TIMER_3"));
    
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        FTM2_SC = FTM2_SC & (FTM_SC_TOIE | FTM_SC_CPWMS | FTM_SC_PS(7));  
      }
    }

    void restartTimer() __attribute__((always_inline))
    {
      startTimer();
    }

    void resumeTimer() __attribute__((always_inline))
    {
      
      if (_timer == TEENSY_TIMER_1)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:resumeTimer TEENSY_TIMER_1"));
     
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        FTM1_SC = (FTM1_SC & (FTM_SC_TOIE | FTM_SC_PS(7))) | FTM_SC_CPWMS | FTM_SC_CLKS(1);
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:resumeTimer TEENSY_TIMER_3"));
        
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        FTM2_SC = (FTM2_SC & (FTM_SC_TOIE | FTM_SC_PS(7))) | FTM_SC_CPWMS | FTM_SC_CLKS(1);
      }
    }


    uint32_t getPeriod() __attribute__((always_inline))
    {
      return _timerCount;
    }
    
    uint32_t getPrescale() __attribute__((always_inline))
    {
      return _prescale;
    }
    
    // Real (actual) period in us
    uint32_t getRealPeriod() __attribute__((always_inline))
    {
      return _realPeriod;
    }
   
    timerCallback getCallback() __attribute__((always_inline))
    {
      return _callback;
    }
    
    IRQ_NUMBER_t getTimerIRQn() __attribute__((always_inline))
    {
      return _timer_IRQ;
    }
};


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

  #ifdef BOARD_NAME
      #undef BOARD_NAME
  #endif
    
  // For Teensy 2.0 and Teensy++ 2.0
  #warning Using Teensy 2.0 or Teensy++ 2.0
  #define BOARD_NAME  "Teensy 2.0 or Teensy++ 2.0"
  
  #if defined(KINETISK)
    #define F_TIMER F_BUS
  #elif defined(KINETISL)
    #define F_TIMER (F_PLL/2)
  #endif
  
  #define TIMER_RESOLUTION    65536UL  // Timer1 and Timer 3 are 16 bit timers

  TeensyTimerInterrupt*   TeensyTimers      [TEENSY_MAX_TIMER] = { NULL, NULL };

class TeensyTimerInterrupt
{
  // Use only 15 bit resolution.  From K66 reference manual, 45.5.7 page 1200:
  //   The CPWM pulse width (duty cycle) is determined by 2 x (CnV - CNTIN) and the
  //   period is determined by 2 x (MOD - CNTIN). See the following figure. MOD must be
  //   kept in the range of 0x0001 to 0x7FFF because values outside this range can produce
  //   ambiguous results.
  #undef  TIMER_RESOLUTION
  #define TIMER_RESOLUTION    32768
   
  private:
   
    uint8_t               _timer       = TEENSY_TIMER_1;
       
    timerCallback         _callback;        // pointer to the callback function
    
    float                 _frequency;       // Timer frequency
    uint32_t              _timerCount;      // count to activate timer
    
    uint32_t              _prescale = 0;
    uint32_t              _realPeriod;
            
  public:
  
    TeensyTimerInterrupt(uint8_t timer = TEENSY_TIMER_1) __attribute__((always_inline))
    {   
      if (timer < TEENSY_MAX_TIMER)
        _timer = timer;
      else
      {
        // Error out of range, force to use TEENSY_TIMER_1
        _timer = TEENSY_TIMER_1;
      }
           
      // Update to use in IRQHandler
      TeensyTimers[_timer] = this;
      
      _callback = NULL;
    }
    
    ~TeensyTimerInterrupt() __attribute__((always_inline))
    {
      TeensyTimers[_timer] = NULL;
    }
    
    // frequency (in hertz) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    // No params and duration now. To be added in the future by adding similar functions here or to NRF52-hal-timer.c
    bool setFrequency(float frequency, timerCallback callback) __attribute__((always_inline))
    {
      return setInterval((float) (1000000.0f / frequency), callback);
    }
    
    // Interval (in microseconds)
    bool setInterval(unsigned long interval, timerCallback callback) __attribute__((always_inline))
    {     
      // This function will be called when time out interrupt will occur
      if (callback) 
      {
          _callback = callback;
      } 
      else 
      {
          PWM_LOGERROR(F("TeensyTimerInterrupt: ERROR: NULL callback function pointer."));
          
          return false;
      }
      
      uint32_t period   = ((float) F_CPU / 2000000) * (float) interval;
      uint32_t prescale = 0;
      
      if (_timer == TEENSY_TIMER_1)
      {
        ///////////// TEENSY_TIMER_1 code ////////////////////////
        if (period < TIMER_RESOLUTION) 
        {
		      prescale = _BV(CS10);
	      } 
	      else if (period < TIMER_RESOLUTION * 8) 
	      {
		      prescale = _BV(CS11);
		      period = period >> 3;
	      } 
	      else if (period < TIMER_RESOLUTION * 64) 
	      {
		      prescale = _BV(CS11) | _BV(CS10);
		      period = period >> 6;
	      } 
	      else if (period < TIMER_RESOLUTION * 256) 
	      {
		      prescale = _BV(CS12);
		      period = period >> 8;
	      } 
	      else if (period < TIMER_RESOLUTION * 1024) 
	      {
		      prescale = _BV(CS12) | _BV(CS10);
		      period = period >> 10;
	      } 
	      else 
	      {
		      prescale = _BV(CS12) | _BV(CS10);
		      period = TIMER_RESOLUTION - 1;
	      }
	      
	      ICR1 = period;
	      TCCR1B = _BV(WGM13) | prescale;
	    }
	    else if (_timer == TEENSY_TIMER_3)
      {
        ///////////// TEENSY_TIMER_3 code //////////////////////// 
        if (period < TIMER_RESOLUTION) 
        {
		      prescale = _BV(CS30);
	      } 
	      else if (period < TIMER_RESOLUTION * 8) 
	      {
		      prescale = _BV(CS31);
		      period = period >> 3;
	      } 
	      else if (period < TIMER_RESOLUTION * 64) 
	      {
		      prescale = _BV(CS31) | _BV(CS30);
		      period = period >> 6;
	      } 
	      else if (period < TIMER_RESOLUTION * 256) 
	      {
		      prescale = _BV(CS32);
		      period = period >> 8;
	      } 
	      else if (period < TIMER_RESOLUTION * 1024) 
	      {
		      prescale = _BV(CS32) | _BV(CS30);
		      period = period >> 10;
	      } 
	      else 
	      {
		      prescale = _BV(CS32) | _BV(CS30);
		      period = TIMER_RESOLUTION - 1;
	      }
	      
	      ICR3 = period;
	      TCCR3B = _BV(WGM33) | prescale;
      }

      _realPeriod = (uint32_t) ((period * 2000000.0f) / (F_CPU >> prescale));
	    _prescale   = prescale;
	    _timerCount = period;

      if (_timer == TEENSY_TIMER_1)
      {
        PWM_LOGWARN1(F("TEENSY_TIMER_1: , F_CPU (MHz) ="), F_CPU/1000000);
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        PWM_LOGWARN1(F("TEENSY_TIMER_3: , F_CPU (MHz) ="), F_CPU/1000000);
      }
          
      PWM_LOGWARN3(F("Request interval ="), interval, F(", actual interval (us) ="), _realPeriod);
      PWM_LOGWARN3(F("Prescale ="), _prescale, F(", _timerCount ="), _timerCount);
	    
	    // Interrupt attach and enable code
	    if (_timer == TEENSY_TIMER_1)
      {
        ///////////// TEENSY_TIMER_1 code ////////////////////////   	    
        TIMSK1 = _BV(TOIE1);
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        ///////////// TEENSY_TIMER_3 code ////////////////////////          
        TIMSK3 = _BV(TOIE3);
      }
                
      return true;
    }
       
    bool attachInterrupt(float frequency, timerCallback callback) __attribute__((always_inline))
    {
      return setInterval((float) (1000000.0f / frequency), callback);
    }

    // interval (in microseconds) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    // No params and duration now. To be added in the future by adding similar functions here or to NRF52-hal-timer.c
    bool attachInterruptInterval(unsigned long interval, timerCallback callback) __attribute__((always_inline))
    {
      return setInterval(interval, callback);
    }
    
    void detachInterrupt() __attribute__((always_inline))
    {              
      if (_timer == TEENSY_TIMER_1)
      {
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        TIMSK1 = 0;
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        TIMSK3 = 0;
      }
    }

    void disableTimer() __attribute__((always_inline))
    {
      detachInterrupt();
    }
    
    //****************************
    //  Run Control
    //****************************
    void startTimer() __attribute__((always_inline))
    {     
      if (_timer == TEENSY_TIMER_1)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:startTimer TEENSY_TIMER_1"));
            
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        TCCR1B  = 0;
	      TCNT1   = 0;
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:startTimer TEENSY_TIMER_3"));
      
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        TCCR3B  = 0;
	      TCNT3   = 0;
      }
      
      resumeTimer();
    }

    void stopTimer() __attribute__((always_inline))
    {
      if (_timer == TEENSY_TIMER_1)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:stopTimer TEENSY_TIMER_1"));
          
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        TCCR1B = _BV(WGM13);
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:stopTimer TEENSY_TIMER_3"));
      
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        TCCR3B = _BV(WGM33);
      }
    }

    void restartTimer() __attribute__((always_inline))
    {
      startTimer();
    }

    void resumeTimer() __attribute__((always_inline))
    {
      
      if (_timer == TEENSY_TIMER_1)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:resumeTimer TEENSY_TIMER_1"));
   
        ///////////// TEENSY_TIMER_1 code ////////////////////////     
        TCCR1B = _BV(WGM13) | _prescale;
      }
      else if (_timer == TEENSY_TIMER_3)
      {
        PWM_LOGWARN(F("TeensyTimerInterrupt:resumeTimer TEENSY_TIMER_3"));
        
        ///////////// TEENSY_TIMER_3 code ////////////////////////
        TCCR3B = _BV(WGM33) | _prescale;
      }
    }

    uint32_t getPeriod() __attribute__((always_inline))
    {
      return _timerCount;
    }
    
    uint32_t getPrescale() __attribute__((always_inline))
    {
      return _prescale;
    }
    
    // Real (actual) period in us
    uint32_t getRealPeriod() __attribute__((always_inline))
    {
      return _realPeriod;
    }
   
    timerCallback getCallback() __attribute__((always_inline))
    {
      return _callback;
    }
};

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

#include "Teensy_Slow_PWM_ISR.h"


#endif    // TEENSY_SLOW_PWM_H

