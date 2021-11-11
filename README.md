# Teensy_Slow_PWM Library

[![arduino-library-badge](https://www.ardu-badge.com/badge/Teensy_Slow_PWM.svg?)](https://www.ardu-badge.com/Teensy_Slow_PWM)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/Teensy_Slow_PWM.svg)](https://github.com/khoih-prog/Teensy_Slow_PWM/releases)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/khoih-prog/Teensy_Slow_PWM/blob/main/LICENSE)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/Teensy_Slow_PWM.svg)](http://github.com/khoih-prog/Teensy_Slow_PWM/issues)

---
---

## Table of Contents

* [Why do we need this Teensy_Slow_PWM library](#why-do-we-need-this-Teensy_Slow_PWM-library)
  * [Features](#features)
  * [Why using ISR-based PWM is better](#why-using-isr-based-pwm-is-better)
  * [Currently supported Boards](#currently-supported-boards)
  * [Important Notes about ISR](#important-notes-about-isr)
* [Changelog](changelog.md)
* [Prerequisites](#prerequisites)
* [Installation](#installation)
  * [Use Arduino Library Manager](#use-arduino-library-manager)
  * [Manual Install](#manual-install)
  * [VS Code & PlatformIO](#vs-code--platformio)
* [Packages' Patches](#packages-patches)
  * [1. For Teensy boards](#1-for-teensy-boards) 
* [HOWTO Fix `Multiple Definitions` Linker Error](#howto-fix-multiple-definitions-linker-error)
* [Usage](#usage)
  * [1. Init Hardware Timer](#1-init-hardware-timer)
  * [2. Set PWM Frequency, dutycycle, attach irqCallbackStartFunc and irqCallbackStopFunc functions](#2-Set-PWM-Frequency-dutycycle-attach-irqCallbackStartFunc-and-irqCallbackStopFunc-functions)
* [Examples](#examples)
  * [ 1. ISR_8_PWMs_Array](examples/ISR_8_PWMs_Array)
  * [ 2. ISR_8_PWMs_Array_Complex](examples/ISR_8_PWMs_Array_Complex)
  * [ 3. ISR_8_PWMs_Array_Simple](examples/ISR_8_PWMs_Array_Simple)
  * [ 4. ISR_Changing_PWM](examples/ISR_Changing_PWM)
  * [ 5. ISR_Modify_PWM](examples/ISR_Modify_PWM)
* [Example ISR_8_PWMs_Array_Complex](#Example-ISR_8_PWMs_Array_Complex)
* [Debug Terminal Output Samples](#debug-terminal-output-samples)
  * [1. ISR_8_PWMs_Array_Complex on Teensy 4.1](#1-ISR_8_PWMs_Array_Complex-on-Teensy-41)
  * [2. ISR_8_PWMs_Array on Teensy 4.1](#2-isr_8_pwms_array-on-Teensy-41)
  * [3. ISR_8_PWMs_Array_Simple on Teensy 4.1](#3-ISR_8_PWMs_Array_Simple-on-Teensy-41)
  * [4. ISR_Modify_PWM on Teensy 4.1](#4-ISR_Modify_PWM-on-Teensy-41)
  * [5. ISR_Changing_PWM on Teensy 4.1](#5-ISR_Changing_PWM-on-Teensy-41)
* [Debug](#debug)
* [Troubleshooting](#troubleshooting)
* [Issues](#issues)
* [TO DO](#to-do)
* [DONE](#done)
* [Contributions and Thanks](#contributions-and-thanks)
* [Contributing](#contributing)
* [License](#license)
* [Copyright](#copyright)

---
---

### Why do we need this [Teensy_Slow_PWM library](https://github.com/khoih-prog/Teensy_Slow_PWM)

### Features

This library enables you to use ISR-based PWM channels on **Teensy boards, such as Teensy 2.x, Teensy LC, Teensy 3.x, Teensy 4.x, Teensy MicroMod, etc.**, etc. using [Teensyduno core](https://www.pjrc.com/teensy/td_download.html) to create and output PWM any GPIO pin. Because this library doesn't use the powerful purely hardware-controlled PWM with many limitations, the maximum PWM frequency is currently limited at **500Hz**, which is still suitable for many real-life applications. Now you can change the PWM settings on-the-fly

---

This library enables you to use Interrupt from Hardware Timers on Teensy boards to create and output PWM to pins. It now supports 16 ISR-based synchronized PWM channels, while consuming only 1 Hardware Timer. PWM interval can be very long (uint64_t microsecs / millisecs). The most important feature is they're ISR-based PWM channels. Therefore, their executions are not blocked by bad-behaving functions or tasks. This important feature is absolutely necessary for mission-critical tasks. These hardware PWM channels, using interrupt, still work even if other functions are blocking. Moreover, they are much more precise (certainly depending on clock frequency accuracy) than other software PWM using millis() or micros(). That's necessary if you need to measure some data requiring better accuracy.

As **Hardware Timers are rare, and very precious assets** of any board, this library now enables you to use up to **16 ISR-based synchronized PWM channels, while consuming only 1 Hardware Timer**. Timers' interval is very long (**ulong millisecs**).

---

You'd certainly experienced that if using other Hardware Timer Libraries, such as [**TimerOne**](https://github.com/PaulStoffregen/TimerOne) or [**TimerThree**](https://github.com/PaulStoffregen/TimerThree), the interval is short, in milliseconds range.

For example, Teensy 4.x, with **super-high clock frequency of 600MHz and Timer1 and Timer3 clock of 150MHz, the maximum interval / frequency is only 55922.3467 us / 17.881939 Hz**. This [**Teensy_Slow_PWM library**](https://github.com/khoih-prog/Teensy_Slow_PWM) will provide you up to 16 super-long (**ulong millisecs**) ISR-based PWM-channels for each used Timer1 or Timer3.

For Teensy 4.x, this library will be expanded to use other available hardware timers, such as **FTM, GPT, QUAD, PIT**, in addition to current **Timer1 and Timer3**.

---

Now with these new **16 ISR-based PWM-channels**, the maximum interval is **practically unlimited** (limited only by unsigned long miliseconds) while **the accuracy is nearly perfect** compared to software PWM channels. 

The most important feature is they're ISR-based PWM channels. Therefore, their executions are **not blocked by bad-behaving functions / tasks**. This important feature is absolutely necessary for mission-critical tasks. 

The [**ISR_8_PWMs_Array_Complex**](examples/ISR_8_PWMs_Array_Complex) example will demonstrate the nearly perfect accuracy, compared to software PWM, by printing the actual period / duty-cycle in `microsecs` of each of PWM-channels.

Being ISR-based PWM, their executions are not blocked by bad-behaving functions / tasks, such as connecting to WiFi, Internet or Blynk services. You can also have many `(up to 16)` PWM channels to use.

This non-being-blocked important feature is absolutely necessary for mission-critical tasks.

You'll see `software-based` SimpleTimer is blocked while system is connecting to WiFi / Internet / Blynk, as well as by blocking task 
in loop(), using delay() function as an example. The elapsed time then is very unaccurate

---

#### Why using ISR-based PWM is better

Imagine you have a system with a **mission-critical** function, measuring water level and control the sump pump or doing something much more important. You normally use a software timer to poll, or even place the function in loop(). But what if another function is **blocking** the loop() or setup().

So your function **might not be executed, and the result would be disastrous.**

You'd prefer to have your function called, no matter what happening with other functions (busy loop, bug, etc.).

The correct choice is to use a Hardware Timer with **Interrupt** to call your function.

These hardware PWM channels, using interrupt, still work even if other functions are blocking. Moreover, they are much more **precise** (certainly depending on clock frequency accuracy) than other software PWM channels using millis() or micros(). That's necessary if you need to measure some data requiring better accuracy.

Functions using normal software PWM channels, relying on loop() and calling millis(), won't work if the loop() or setup() is blocked by certain operation. For example, certain function is blocking while it's connecting to WiFi or some services.

The catch is **your function is now part of an ISR (Interrupt Service Routine), and must be lean / mean, and follow certain rules.** More to read on:

[**HOWTO Attach Interrupt**](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/)

---

### Currently supported Boards

1. **Teensy boards** such as :

  - **Teensy 4.1, Teensy MicroMod, Teensy 4.0**
  - **Teensy 3.6, 3.5, 3.2/3.1, 3.0**
  - **Teensy LC**
  - **Teensy++ 2.0 and Teensy 2.0**

---

#### Important Notes about ISR

1. Inside the attached function, **delay() won’t work and the value returned by millis() will not increment.** Serial data received while in the function may be lost. You should declare as **volatile any variables that you modify within the attached function.**

2. Typically global variables are used to pass data between an ISR and the main program. To make sure variables shared between an ISR and the main program are updated correctly, declare them as volatile.

---
---

## Prerequisites

 1. [`Arduino IDE 1.8.16+` for Arduino](https://www.arduino.cc/en/Main/Software)
 2. [`Teensy Core 1.55+`](https://www.pjrc.com/teensy/td_download.html) for Teensy.
 
 3. To use with certain example
   - [`SimpleTimer library`](https://github.com/jfturcot/SimpleTimer) for [ISR_8_PWMs_Array_Complex example](examples/ISR_8_PWMs_Array_Complex).
---
---

## Installation

### Use Arduino Library Manager

The best and easiest way is to use `Arduino Library Manager`. Search for [**Teensy_Slow_PWM**](https://github.com/khoih-prog/Teensy_Slow_PWM), then select / install the latest version.
You can also use this link [![arduino-library-badge](https://www.ardu-badge.com/badge/Teensy_Slow_PWM.svg?)](https://www.ardu-badge.com/Teensy_Slow_PWM) for more detailed instructions.

### Manual Install

Another way to install is to:

1. Navigate to [**Teensy_Slow_PWM**](https://github.com/khoih-prog/Teensy_Slow_PWM) page.
2. Download the latest release `Teensy_Slow_PWM-master.zip`.
3. Extract the zip file to `Teensy_Slow_PWM-master` directory 
4. Copy whole `Teensy_Slow_PWM-master` folder to Arduino libraries' directory such as `~/Arduino/libraries/`.

### VS Code & PlatformIO

1. Install [VS Code](https://code.visualstudio.com/)
2. Install [PlatformIO](https://platformio.org/platformio-ide)
3. Install [**Teensy_Slow_PWM** library](https://platformio.org/lib/show/12888/Teensy_Slow_PWM) by using [Library Manager](https://platformio.org/lib/show/12888/Teensy_Slow_PWM/installation). Search for **Teensy_Slow_PWM** in [Platform.io Author's Libraries](https://platformio.org/lib/search?query=author:%22Khoi%20Hoang%22)
4. Use included [platformio.ini](platformio/platformio.ini) file from examples to ensure that all dependent libraries will installed automatically. Please visit documentation for the other options and examples at [Project Configuration File](https://docs.platformio.org/page/projectconf.html)


---
---

### Packages' Patches

#### 1. For Teensy boards
 
 **To be able to compile and run on Teensy boards**, you have to copy the file [Teensy boards.txt](Packages_Patches/hardware/teensy/avr/boards.txt) into Teensy hardware directory (./arduino-1.8.16/hardware/teensy/avr/boards.txt). 

Supposing the Arduino version is 1.8.16. These files must be copied into the directory:

- `./arduino-1.8.16/hardware/teensy/avr/boards.txt`
- `./arduino-1.8.16/hardware/teensy/avr/cores/teensy/Stream.h`
- `./arduino-1.8.16/hardware/teensy/avr/cores/teensy3/Stream.h`
- `./arduino-1.8.16/hardware/teensy/avr/cores/teensy4/Stream.h`

Whenever a new version is installed, remember to copy this file into the new version directory. For example, new version is x.yy.zz
This file must be copied into the directory:

- `./arduino-x.yy.zz/hardware/teensy/avr/boards.txt`
- `./arduino-x.yy.zz/hardware/teensy/avr/cores/teensy/Stream.h`
- `./arduino-x.yy.zz/hardware/teensy/avr/cores/teensy3/Stream.h`
- `./arduino-x.yy.zz/hardware/teensy/avr/cores/teensy4/Stream.h`

---
---

### HOWTO Fix `Multiple Definitions` Linker Error

The current library implementation, using **xyz-Impl.h instead of standard xyz.cpp**, possibly creates certain `Multiple Definitions` Linker error in certain use cases. Although it's simple to just modify several lines of code, either in the library or in the application, the library is adding 2 more source directories

1. **scr_h** for new h-only files
2. **src_cpp** for standard h/cpp files

besides the standard **src** directory.

To use the **old standard cpp** way, locate this library' directory, then just 

1. **Delete the all the files in src directory.**
2. **Copy all the files in src_cpp directory into src.**
3. Close then reopen the application code in Arduino IDE, etc. to recompile from scratch.

To re-use the **new h-only** way, just 

1. **Delete the all the files in src directory.**
2. **Copy the files in src_h directory into src.**
3. Close then reopen the application code in Arduino IDE, etc. to recompile from scratch.


---
---

## Usage


Before using any Timer, you have to make sure the Timer has not been used by any other purpose.


#### 1. Init Hardware Timer

```
// You can select Teensy Hardware Timer from TEENSY_TIMER_1 or TEENSY_TIMER_3

// Init Teensy timer TEENSY_TIMER_1
TeensyTimer ITimer(TEENSY_TIMER_1);

// Init Teensy_SLOW_PWM, each can service 16 different ISR-based PWM channels
Teensy_SLOW_PWM ISR_PWM;
```

#### 2. Set PWM Frequency, dutycycle, attach irqCallbackStartFunc and irqCallbackStopFunc functions

```
void irqCallbackStartFunc()
{

}

void irqCallbackStopFunc()
{

}

void setup()
{
  ....
  
  // You can use this with PWM_Freq in Hz
  ISR_PWM.setPWM(PWM_Pin, PWM_Freq, PWM_DutyCycle, irqCallbackStartFunc, irqCallbackStopFunc);
                   
  ....                 
}  
```

---
---

### Examples: 

 1. [ISR_8_PWMs_Array](examples/ISR_8_PWMs_Array)
 2. [ISR_8_PWMs_Array_Complex](examples/ISR_8_PWMs_Array_Complex)
 3. [ISR_8_PWMs_Array_Simple](examples/ISR_8_PWMs_Array_Simple)
 4. [ISR_Changing_PWM](examples/ISR_Changing_PWM)
 5. [ISR_Modify_PWM](examples/ISR_Modify_PWM)

 
---
---

### Example [ISR_8_PWMs_Array_Complex](examples/ISR_8_PWMs_Array_Complex)

```
#if !( defined(CORE_TEENSY) || defined(TEENSYDUINO) )
  #error This code is designed to run on Teensy platform! Please check your Tools->Board setting.
#endif

// These define's must be placed at the beginning before #include "ESP32_PWM.h"
// _PWM_LOGLEVEL_ from 0 to 4
// Don't define _PWM_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define _PWM_LOGLEVEL_      3

#define USING_MICROS_RESOLUTION       true  //false 

#include "Teensy_Slow_PWM.h"

#include <SimpleTimer.h>              // https://github.com/jfturcot/SimpleTimer

#define LED_OFF             HIGH
#define LED_ON              LOW

#ifndef LED_BUILTIN
  #define LED_BUILTIN       13
#endif

#ifndef LED_BLUE
  #define LED_BLUE          10
#endif

#ifndef LED_RED
  #define LED_RED           11
#endif

#define USING_HW_TIMER_INTERVAL_MS        false   //true

#if defined(__IMXRT1062__)
  // For Teensy 4.0 and 4.1
  // Don't change these numbers to make higher Timer freq. System can hang
  #define HW_TIMER_INTERVAL_MS        0.0333f
  #define HW_TIMER_INTERVAL_FREQ      30000L
#elif defined(__MK66FX1M0__)
  // For Teensy 3.6
  // Don't change these numbers to make higher Timer freq. System can hang
  #define HW_TIMER_INTERVAL_MS        0.05f
  #define HW_TIMER_INTERVAL_FREQ      20000L
#else
  // Don't change these numbers to make higher Timer freq. System can hang
  #define HW_TIMER_INTERVAL_MS        0.1f
  #define HW_TIMER_INTERVAL_FREQ      10000L
#endif

volatile uint32_t startMicros = 0;

// You can select Teensy Hardware Timer  from TEENSY_TIMER_1 or TEENSY_TIMER_3

// Init Teensy timer TEENSY_TIMER_1
TeensyTimer ITimer(TEENSY_TIMER_1);

// Init Teensy_SLOW_PWM, each can service 16 different ISR-based PWM channels
Teensy_SLOW_PWM ISR_PWM;

//////////////////////////////////////////////////////

void TimerHandler()
{
  ISR_PWM.run();
}

/////////////////////////////////////////////////

#define PIN_D0      0
#define PIN_D1      1
#define PIN_D2      2
#define PIN_D3      3
#define PIN_D4      4
#define PIN_D5      5
#define PIN_D6      6

// You can assign pins here. Be careful to select good pin to use or crash, e.g pin 6-11
uint32_t PWM_Pin[] =
{
  LED_BUILTIN, PIN_D0, PIN_D1,  PIN_D2,  PIN_D3,  PIN_D4,  PIN_D5,  PIN_D6
};

#define NUMBER_ISR_PWMS         ( sizeof(PWM_Pin) / sizeof(uint32_t) )

typedef void (*irqCallback)  ();

//////////////////////////////////////////////////////

#define USE_COMPLEX_STRUCT      true

//////////////////////////////////////////////////////

#if USE_COMPLEX_STRUCT

typedef struct
{
  uint32_t      PWM_Pin;
  irqCallback   irqCallbackStartFunc;
  irqCallback   irqCallbackStopFunc;

  uint32_t      PWM_Freq;

  uint32_t      PWM_DutyCycle;

  uint32_t      deltaMicrosStart;
  uint32_t      previousMicrosStart;

  uint32_t      deltaMicrosStop;
  uint32_t      previousMicrosStop;

} ISR_PWM_Data;

// In nRF52, avoid doing something fancy in ISR, for example Serial.print()
// The pure simple Serial.prints here are just for demonstration and testing. Must be eliminate in working environment
// Or you can get this run-time error / crash

void doingSomethingStart(int index);

void doingSomethingStop(int index);

#else   // #if USE_COMPLEX_STRUCT

volatile unsigned long deltaMicrosStart    [NUMBER_ISR_PWMS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
volatile unsigned long previousMicrosStart [NUMBER_ISR_PWMS] = { 0, 0, 0, 0, 0, 0, 0, 0 };

volatile unsigned long deltaMicrosStop     [NUMBER_ISR_PWMS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
volatile unsigned long previousMicrosStop  [NUMBER_ISR_PWMS] = { 0, 0, 0, 0, 0, 0, 0, 0 };


// You can assign any interval for any timer here, in Microseconds
uint32_t PWM_Period[NUMBER_ISR_PWMS] =
{
  1000L,   500L,   333L,   250L,   200L,   166L,   142L,   125L
};

// You can assign any interval for any timer here, in Hz
double PWM_Freq[NUMBER_ISR_PWMS] =
{
  1.0f,  2.0f,  3.0f,  4.0f,  5.0f,  6.0f,  7.0f,  8.0f,
};

// You can assign any interval for any timer here, in Microseconds
uint32_t PWM_DutyCycle[NUMBER_ISR_PWMS] =
{
  5, 10, 20, 25, 30, 35, 40, 45
};

void doingSomethingStart(int index)
{
  unsigned long currentMicros  = micros();

  deltaMicrosStart[index]    = currentMicros - previousMicrosStart[index];
  previousMicrosStart[index] = currentMicros;
}

void doingSomethingStop(int index)
{
  unsigned long currentMicros  = micros();

  // Count from start to stop PWM pulse
  deltaMicrosStop[index]    = currentMicros - previousMicrosStart[index];
  previousMicrosStop[index] = currentMicros;
}

#endif    // #if USE_COMPLEX_STRUCT

////////////////////////////////////
// Shared
////////////////////////////////////

void doingSomethingStart0()
{
  doingSomethingStart(0);
}

void doingSomethingStart1()
{
  doingSomethingStart(1);
}

void doingSomethingStart2()
{
  doingSomethingStart(2);
}

void doingSomethingStart3()
{
  doingSomethingStart(3);
}

void doingSomethingStart4()
{
  doingSomethingStart(4);
}

void doingSomethingStart5()
{
  doingSomethingStart(5);
}

void doingSomethingStart6()
{
  doingSomethingStart(6);
}

void doingSomethingStart7()
{
  doingSomethingStart(7);
}


//////////////////////////////////////////////////////

void doingSomethingStop0()
{
  doingSomethingStop(0);
}

void doingSomethingStop1()
{
  doingSomethingStop(1);
}

void doingSomethingStop2()
{
  doingSomethingStop(2);
}

void doingSomethingStop3()
{
  doingSomethingStop(3);
}

void doingSomethingStop4()
{
  doingSomethingStop(4);
}

void doingSomethingStop5()
{
  doingSomethingStop(5);
}

void doingSomethingStop6()
{
  doingSomethingStop(6);
}

void doingSomethingStop7()
{
  doingSomethingStop(7);
}


//////////////////////////////////////////////////////

#if USE_COMPLEX_STRUCT

ISR_PWM_Data curISR_PWM_Data[NUMBER_ISR_PWMS] =
{
  // pin, irqCallbackStartFunc, irqCallbackStopFunc, PWM_Freq, PWM_DutyCycle, deltaMicrosStart, previousMicrosStart, deltaMicrosStop, previousMicrosStop
  { LED_BUILTIN,  doingSomethingStart0,    doingSomethingStop0,    1,   5, 0, 0, 0, 0 },
  { PIN_D0,       doingSomethingStart1,    doingSomethingStop1,    2,  10, 0, 0, 0, 0 },
  { PIN_D1,       doingSomethingStart2,    doingSomethingStop2,    3,  20, 0, 0, 0, 0 },
  { PIN_D2,       doingSomethingStart3,    doingSomethingStop3,    4,  25, 0, 0, 0, 0 },
  { PIN_D3,       doingSomethingStart4,    doingSomethingStop4,    5,  30, 0, 0, 0, 0 },
  { PIN_D4,       doingSomethingStart5,    doingSomethingStop5,    6,  35, 0, 0, 0, 0 },
  { PIN_D5,       doingSomethingStart6,    doingSomethingStop6,    7,  40, 0, 0, 0, 0 },
  { PIN_D6,       doingSomethingStart7,    doingSomethingStop7,    8,  45, 0, 0, 0, 0 },
};


void doingSomethingStart(int index)
{
  unsigned long currentMicros  = micros();

  curISR_PWM_Data[index].deltaMicrosStart    = currentMicros - curISR_PWM_Data[index].previousMicrosStart;
  curISR_PWM_Data[index].previousMicrosStart = currentMicros;
}

void doingSomethingStop(int index)
{
  unsigned long currentMicros  = micros();

  //curISR_PWM_Data[index].deltaMicrosStop     = currentMicros - curISR_PWM_Data[index].previousMicrosStop;
  // Count from start to stop PWM pulse
  curISR_PWM_Data[index].deltaMicrosStop     = currentMicros - curISR_PWM_Data[index].previousMicrosStart;
  curISR_PWM_Data[index].previousMicrosStop  = currentMicros;
}

#else   // #if USE_COMPLEX_STRUCT

irqCallback irqCallbackStartFunc[NUMBER_ISR_PWMS] =
{
  doingSomethingStart0,  doingSomethingStart1,  doingSomethingStart2,  doingSomethingStart3,
  doingSomethingStart4,  doingSomethingStart5,  doingSomethingStart6,  doingSomethingStart7
};

irqCallback irqCallbackStopFunc[NUMBER_ISR_PWMS] =
{
  doingSomethingStop0,  doingSomethingStop1,  doingSomethingStop2,  doingSomethingStop3,
  doingSomethingStop4,  doingSomethingStop5,  doingSomethingStop6,  doingSomethingStop7
};

#endif    // #if USE_COMPLEX_STRUCT

//////////////////////////////////////////////////////

#define SIMPLE_TIMER_MS        2000L

// Init SimpleTimer
SimpleTimer simpleTimer;

// Here is software Timer, you can do somewhat fancy stuffs without many issues.
// But always avoid
// 1. Long delay() it just doing nothing and pain-without-gain wasting CPU power.Plan and design your code / strategy ahead
// 2. Very long "do", "while", "for" loops without predetermined exit time.
void simpleTimerDoingSomething2s()
{
  static unsigned long previousMicrosStart = startMicros;

  unsigned long currMicros = micros();

  Serial.print(F("SimpleTimer (us): ")); Serial.print(SIMPLE_TIMER_MS);
  Serial.print(F(", us : ")); Serial.print(currMicros);
  Serial.print(F(", Dus : ")); Serial.println(currMicros - previousMicrosStart);

  for (uint16_t i = 0; i < NUMBER_ISR_PWMS; i++)
  {
#if USE_COMPLEX_STRUCT
    Serial.print(F("PWM Channel : ")); Serial.print(i);
    Serial.print(F(", prog Period (ms): "));

    Serial.print(1000.f / curISR_PWM_Data[i].PWM_Freq);

    Serial.print(F(", actual : ")); Serial.print((uint32_t) curISR_PWM_Data[i].deltaMicrosStart);

    Serial.print(F(", prog DutyCycle : "));

    Serial.print(curISR_PWM_Data[i].PWM_DutyCycle);

    Serial.print(F(", actual : ")); Serial.println((float) curISR_PWM_Data[i].deltaMicrosStop * 100.0f / curISR_PWM_Data[i].deltaMicrosStart);
    //Serial.print(F(", actual deltaMicrosStop : ")); Serial.println(curISR_PWM_Data[i].deltaMicrosStop);
    //Serial.print(F(", actual deltaMicrosStart : ")); Serial.println(curISR_PWM_Data[i].deltaMicrosStart);

#else

    Serial.print(F("PWM Channel : ")); Serial.print(i);

    Serial.print(1000 / PWM_Freq[i]);

    Serial.print(F(", prog. Period (us): ")); Serial.print(PWM_Period[i]);
    Serial.print(F(", actual : ")); Serial.print(deltaMicrosStart[i]);

    Serial.print(F(", prog DutyCycle : "));

    Serial.print(PWM_DutyCycle[i]);

    Serial.print(F(", actual : ")); Serial.println( (float) deltaMicrosStop[i] * 100.0f / deltaMicrosStart[i]);
    //Serial.print(F(", actual deltaMicrosStop : ")); Serial.println(deltaMicrosStop[i]);
    //Serial.print(F(", actual deltaMicrosStart : ")); Serial.println(deltaMicrosStart[i]);
#endif
  }

  previousMicrosStart = currMicros;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  delay(2000);

  Serial.print(F("\nStarting ISR_8_PWMs_Array_Complex on ")); Serial.println(BOARD_NAME);
  Serial.println(TEENSY_SLOW_PWM_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

  // Timer0 is used for micros(), micros(), delay(), etc and can't be used
  // Select Timer 1-2 for UNO, 1-5 for MEGA, 1,3,4 for 16u4/32u4
  // Timer 2 is 8-bit timer, only for higher frequency
  // Timer 4 of 16u4 and 32u4 is 8/10-bit timer, only for higher frequency

#if USING_HW_TIMER_INTERVAL_MS

  // Interval in microsecs
  if (ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS * 1000, TimerHandler))
  {
    startMicros = micros();
    Serial.print(F("Starting ITimer OK, micros() = ")); Serial.println(startMicros);
  }
  else
    Serial.println(F("Can't set ITimer correctly. Select another freq. or interval"));
    
#else

  if (ITimer.attachInterrupt(HW_TIMER_INTERVAL_FREQ, TimerHandler))
  {
    Serial.print(F("Starting  ITimer OK, micros() = ")); Serial.println(micros());
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));
    
#endif

  startMicros = micros();

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_PWM

  for (uint16_t i = 0; i < NUMBER_ISR_PWMS; i++)
  {
#if USE_COMPLEX_STRUCT
    curISR_PWM_Data[i].previousMicrosStart = startMicros;
    //ISR_PWM.setInterval(curISR_PWM_Data[i].PWM_Period, curISR_PWM_Data[i].irqCallbackStartFunc);

    //void setPWM(uint32_t pin, uint32_t frequency, uint32_t dutycycle
    // , timer_callback_p StartCallback = nullptr, timer_callback_p StopCallback = nullptr)

    // You can use this with PWM_Freq in Hz
    ISR_PWM.setPWM(curISR_PWM_Data[i].PWM_Pin, curISR_PWM_Data[i].PWM_Freq, curISR_PWM_Data[i].PWM_DutyCycle,
                   curISR_PWM_Data[i].irqCallbackStartFunc, curISR_PWM_Data[i].irqCallbackStopFunc);

#else
    previousMicrosStart[i] = micros();

    // You can use this with PWM_Freq in Hz
    ISR_PWM.setPWM(PWM_Pin[i], PWM_Freq[i], PWM_DutyCycle[i], irqCallbackStartFunc[i], irqCallbackStopFunc[i]);

#endif
  }

  // You need this timer for non-critical tasks. Avoid abusing ISR if not absolutely necessary.
  simpleTimer.setInterval(SIMPLE_TIMER_MS, simpleTimerDoingSomething2s);
}

#define BLOCKING_TIME_MS      10000L

void loop()
{
  // This unadvised blocking task is used to demonstrate the blocking effects onto the execution and accuracy to Software timer
  // You see the time elapse of ISR_PWM still accurate, whereas very unaccurate for Software Timer
  // The time elapse for 2000ms software timer now becomes 3000ms (BLOCKING_TIME_MS)
  // While that of ISR_PWM is still prefect.
  delay(BLOCKING_TIME_MS);

  // You need this Software timer for non-critical tasks. Avoid abusing ISR if not absolutely necessary
  // You don't need to and never call ISR_PWM.run() here in the loop(). It's already handled by ISR timer.
  simpleTimer.run();
}
```
---
---

### Debug Terminal Output Samples

### 1. ISR_8_PWMs_Array_Complex on Teensy 4.1

The following is the sample terminal output when running example [ISR_8_PWMs_Array_Complex](examples/ISR_8_PWMs_Array_Complex) on **Teensy 4.1** to demonstrate how to use multiple PWM channels with complex callback functions, the accuracy of ISR Hardware PWM-channels, **especially when system is very busy**.  The ISR PWM-channels is **running exactly according to corresponding programmed periods and duty-cycles**


```
Starting ISR_8_PWMs_Array_Complex on Teensy 4.1
Teensy_Slow_PWM v1.1.0
CPU Frequency = 600 MHz
[PWM] TEENSY_TIMER_1: , F_BUS_ACTUAL (MHz) = 150
[PWM] Request interval = 20 , actual interval (us) = 20
[PWM] Prescale = 0 , _timerCount = 1500
Starting  ITimer OK, micros() = 2968006
Channel : 0	Period : 1000000		OnTime : 50000	Start_Time : 2968007
Channel : 1	Period : 500000		OnTime : 50000	Start_Time : 2968007
Channel : 2	Period : 333333		OnTime : 66666	Start_Time : 2968007
Channel : 3	Period : 250000		OnTime : 62500	Start_Time : 2968007
Channel : 4	Period : 200000		OnTime : 60000	Start_Time : 2968007
Channel : 5	Period : 166666		OnTime : 58333	Start_Time : 2968007
Channel : 6	Period : 142857		OnTime : 57142	Start_Time : 2968007
Channel : 7	Period : 125000		OnTime : 56250	Start_Time : 2968007
SimpleTimer (us): 2000, us : 12968028, Dus : 10000021
PWM Channel : 0, prog Period (ms): 1000.00, actual : 1000014, prog DutyCycle : 5, actual : 5.00
PWM Channel : 1, prog Period (ms): 500.00, actual : 500006, prog DutyCycle : 10, actual : 10.00
PWM Channel : 2, prog Period (ms): 333.33, actual : 333351, prog DutyCycle : 20, actual : 20.00
PWM Channel : 3, prog Period (ms): 250.00, actual : 250003, prog DutyCycle : 25, actual : 25.00
PWM Channel : 4, prog Period (ms): 200.00, actual : 200006, prog DutyCycle : 30, actual : 30.00
PWM Channel : 5, prog Period (ms): 166.67, actual : 166676, prog DutyCycle : 35, actual : 34.99
PWM Channel : 6, prog Period (ms): 142.86, actual : 142868, prog DutyCycle : 40, actual : 39.99
PWM Channel : 7, prog Period (ms): 125.00, actual : 125002, prog DutyCycle : 45, actual : 44.99
SimpleTimer (us): 2000, us : 22968058, Dus : 10000030
PWM Channel : 0, prog Period (ms): 1000.00, actual : 1000013, prog DutyCycle : 5, actual : 5.00
PWM Channel : 1, prog Period (ms): 500.00, actual : 500006, prog DutyCycle : 10, actual : 10.00
PWM Channel : 2, prog Period (ms): 333.33, actual : 333351, prog DutyCycle : 20, actual : 20.00
PWM Channel : 3, prog Period (ms): 250.00, actual : 250003, prog DutyCycle : 25, actual : 24.99
PWM Channel : 4, prog Period (ms): 200.00, actual : 200007, prog DutyCycle : 30, actual : 30.00
PWM Channel : 5, prog Period (ms): 166.67, actual : 166675, prog DutyCycle : 35, actual : 34.99
PWM Channel : 6, prog Period (ms): 142.86, actual : 142867, prog DutyCycle : 40, actual : 39.99
PWM Channel : 7, prog Period (ms): 125.00, actual : 125002, prog DutyCycle : 45, actual : 44.99
SimpleTimer (us): 2000, us : 32968087, Dus : 10000029
PWM Channel : 0, prog Period (ms): 1000.00, actual : 1000013, prog DutyCycle : 5, actual : 5.00
PWM Channel : 1, prog Period (ms): 500.00, actual : 500007, prog DutyCycle : 10, actual : 10.00
PWM Channel : 2, prog Period (ms): 333.33, actual : 333351, prog DutyCycle : 20, actual : 20.00
PWM Channel : 3, prog Period (ms): 250.00, actual : 250003, prog DutyCycle : 25, actual : 24.99
PWM Channel : 4, prog Period (ms): 200.00, actual : 200007, prog DutyCycle : 30, actual : 30.00
PWM Channel : 5, prog Period (ms): 166.67, actual : 166676, prog DutyCycle : 35, actual : 34.99
PWM Channel : 6, prog Period (ms): 142.86, actual : 142868, prog DutyCycle : 40, actual : 39.99
PWM Channel : 7, prog Period (ms): 125.00, actual : 125001, prog DutyCycle : 45, actual : 44.99
```

---

### 2. ISR_8_PWMs_Array on Teensy 4.1

The following is the sample terminal output when running example [**ISR_8_PWMs_Array**](examples/ISR_8_PWMs_Array) on **on Teensy 4.1** to demonstrate how to use multiple PWM channels with simple callback functions.

```
Starting ISR_8_PWMs_Array on Teensy 4.1
Teensy_Slow_PWM v1.1.0
CPU Frequency = 600 MHz
[PWM] TEENSY_TIMER_1: , F_BUS_ACTUAL (MHz) = 150
[PWM] Request interval = 33 , actual interval (us) = 33
[PWM] Prescale = 0 , _timerCount = 2475
Starting ITimer OK, micros() = 2922006
Channel : 0	Period : 1000000		OnTime : 50000	Start_Time : 2922007
Channel : 1	Period : 500000		OnTime : 50000	Start_Time : 2922007
Channel : 2	Period : 333333		OnTime : 66666	Start_Time : 2922007
Channel : 3	Period : 250000		OnTime : 62500	Start_Time : 2922007
Channel : 4	Period : 200000		OnTime : 60000	Start_Time : 2922007
Channel : 5	Period : 166666		OnTime : 58333	Start_Time : 2922007
Channel : 6	Period : 142857		OnTime : 57142	Start_Time : 2922007
Channel : 7	Period : 125000		OnTime : 56250	Start_Time : 2922007
```

---

### 3. ISR_8_PWMs_Array_Simple on Teensy 4.1

The following is the sample terminal output when running example [**ISR_8_PWMs_Array_Simple**](examples/ISR_8_PWMs_Array_Simple) on **on Teensy 4.1** to demonstrate how to use multiple PWM channels.

```
Starting ISR_8_PWMs_Array_Simple on Teensy 4.1
Teensy_Slow_PWM v1.1.0
CPU Frequency = 600 MHz
[PWM] TEENSY_TIMER_1: , F_BUS_ACTUAL (MHz) = 150
[PWM] Request interval = 33 , actual interval (us) = 33
[PWM] Prescale = 0 , _timerCount = 2475
Starting ITimer OK, micros() = 3906006
Channel : 0	Period : 1000000		OnTime : 50000	Start_Time : 3906007
Channel : 1	Period : 500000		OnTime : 50000	Start_Time : 3906007
Channel : 2	Period : 333333		OnTime : 66666	Start_Time : 3906007
Channel : 3	Period : 250000		OnTime : 62500	Start_Time : 3906007
Channel : 4	Period : 200000		OnTime : 60000	Start_Time : 3906007
Channel : 5	Period : 166666		OnTime : 58333	Start_Time : 3906007
Channel : 6	Period : 142857		OnTime : 57142	Start_Time : 3906007
Channel : 7	Period : 125000		OnTime : 56250	Start_Time : 3906007
```

---

### 4. ISR_Modify_PWM on Teensy 4.1

The following is the sample terminal output when running example [ISR_Modify_PWM](examples/ISR_Modify_PWM) on **Teensy 4.1** to demonstrate how to modify PWM settings on-the-fly without deleting the PWM channel

```
Starting ISR_Modify_PWM on Teensy 4.1
Teensy_Slow_PWM v1.1.0
CPU Frequency = 600 MHz
[PWM] TEENSY_TIMER_1: , F_BUS_ACTUAL (MHz) = 150
[PWM] Request interval = 33 , actual interval (us) = 33
[PWM] Prescale = 0 , _timerCount = 2475
Starting ITimer OK, micros() = 2826009
Using PWM Freq = 1.00, PWM DutyCycle = 10
Channel : 0	Period : 1000000		OnTime : 100000	Start_Time : 2826012
Channel : 0	Period : 500000		OnTime : 450000	Start_Time : 12827000
Channel : 0	Period : 1000000		OnTime : 100000	Start_Time : 22828000
Channel : 0	Period : 500000		OnTime : 450000	Start_Time : 32829000
```

---

### 5. ISR_Changing_PWM on Teensy 4.1

The following is the sample terminal output when running example [ISR_Changing_PWM](examples/ISR_Changing_PWM) on **Teensy 4.1** to demonstrate how to modify PWM settings on-the-fly by deleting the PWM channel and reinit the PWM channel

```
Starting ISR_Changing_PWM on Teensy 4.1
Teensy_Slow_PWM v1.1.0
CPU Frequency = 600 MHz
[PWM] TEENSY_TIMER_1: , F_BUS_ACTUAL (MHz) = 150
[PWM] Request interval = 33 , actual interval (us) = 33
[PWM] Prescale = 0 , _timerCount = 2475
Starting ITimer OK, micros() = 2892009
Using PWM Freq = 1.00, PWM DutyCycle = 50
Channel : 0	Period : 1000000		OnTime : 500000	Start_Time : 2892013
Using PWM Freq = 2.00, PWM DutyCycle = 90
Channel : 0	Period : 500000		OnTime : 450000	Start_Time : 12892018
Using PWM Freq = 1.00, PWM DutyCycle = 50
Channel : 0	Period : 1000000		OnTime : 500000	Start_Time : 22892023
Using PWM Freq = 2.00, PWM DutyCycle = 90
Channel : 0	Period : 500000		OnTime : 450000	Start_Time : 32892029
```


---
---

### Debug

Debug is enabled by default on Serial.

You can also change the debugging level `_PWM_LOGLEVEL_` from 0 to 4

```cpp
// Don't define _PWM_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define _PWM_LOGLEVEL_     0
```

---

### Troubleshooting

If you get compilation errors, more often than not, you may need to install a newer version of the core for Arduino boards.

Sometimes, the library will only work if you update the board core to the latest version because I am using newly added functions.


---
---

### Issues

Submit issues to: [Teensy_Slow_PWM issues](https://github.com/khoih-prog/Teensy_Slow_PWM/issues)

---

## TO DO

1. Search for bug and improvement.
2. Similar features for remaining Arduino boards

---

## DONE

1. Basic hardware multi-channel PWM for **Teensy boards, such as Teensy 2.x, Teensy LC, Teensy 3.x, Teensy 4.x, Teensy MicroMod, etc.**, etc. using [Teensyduno core](https://www.pjrc.com/teensy/td_download.html)
2. Add Table of Contents
3. Add functions to modify PWM settings on-the-fly

---
---

### Contributions and Thanks

Many thanks for everyone for bug reporting, new feature suggesting, testing and contributing to the development of this library.


---

## Contributing

If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

---

### License

- The library is licensed under [MIT](https://github.com/khoih-prog/Teensy_Slow_PWM/blob/main/LICENSE)

---

## Copyright

Copyright 2021- Khoi Hoang


