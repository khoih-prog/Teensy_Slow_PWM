# Teensy_Slow_PWM Library

[![arduino-library-badge](https://www.ardu-badge.com/badge/Teensy_Slow_PWM.svg?)](https://www.ardu-badge.com/Teensy_Slow_PWM)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/Teensy_Slow_PWM.svg)](https://github.com/khoih-prog/Teensy_Slow_PWM/releases)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/khoih-prog/Teensy_Slow_PWM/blob/main/LICENSE)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/Teensy_Slow_PWM.svg)](http://github.com/khoih-prog/Teensy_Slow_PWM/issues)

<a href="https://www.buymeacoffee.com/khoihprog6" title="Donate to my libraries using BuyMeACoffee"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Donate to my libraries using BuyMeACoffee" style="height: 50px !important;width: 181px !important;" ></a>
<a href="https://www.buymeacoffee.com/khoihprog6" title="Donate to my libraries using BuyMeACoffee"><img src="https://img.shields.io/badge/buy%20me%20a%20coffee-donate-orange.svg?logo=buy-me-a-coffee&logoColor=FFDD00" style="height: 20px !important;width: 200px !important;" ></a>


---
---

## Table of Contents

* [Important Change from v1.2.0](#Important-Change-from-v120)
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
  * [ 6. multiFileProject](examples/multiFileProject). **New**
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

### Important Change from v1.2.0

Please have a look at [HOWTO Fix `Multiple Definitions` Linker Error](#howto-fix-multiple-definitions-linker-error)

As more complex calculation and check **inside ISR** are introduced from v1.2.0, there is possibly some crash depending on use-case.

You can modify to use larger `HW_TIMER_INTERVAL_US`, (from current 10 / 20 / 100uS), according to your board and use-case if crash happens.


```
#if defined(__IMXRT1062__)
  // For Teensy 4.0 and 4.1
  // Don't change these numbers to make higher Timer freq. System can hang
  #define HW_TIMER_INTERVAL_MS        0.01f
  #define HW_TIMER_INTERVAL_FREQ      100000L
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
```

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

 1. [`Arduino IDE 1.8.19+` for Arduino](https://github.com/arduino/Arduino). [![GitHub release](https://img.shields.io/github/release/arduino/Arduino.svg)](https://github.com/arduino/Arduino/releases/latest)
 2. [`Teensy Core 1.56+`](https://www.pjrc.com/teensy/td_download.html) for Teensy.
 
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
3. Install [**Teensy_Slow_PWM** library](https://registry.platformio.org/libraries/khoih-prog/Teensy_Slow_PWM) by using [Library Manager](https://registry.platformio.org/libraries/khoih-prog/Teensy_Slow_PWM/installation). Search for **Teensy_Slow_PWM** in [Platform.io Author's Libraries](https://platformio.org/lib/search?query=author:%22Khoi%20Hoang%22)
4. Use included [platformio.ini](platformio/platformio.ini) file from examples to ensure that all dependent libraries will installed automatically. Please visit documentation for the other options and examples at [Project Configuration File](https://docs.platformio.org/page/projectconf.html)


---
---

### Packages' Patches

#### 1. For Teensy boards
 
 **To be able to compile and run on Teensy boards**, you have to copy the file [Teensy boards.txt](Packages_Patches/hardware/teensy/avr/boards.txt) into Teensy hardware directory (./arduino-1.8.19/hardware/teensy/avr/boards.txt). 

Supposing the Arduino version is 1.8.19. These files must be copied into the directory:

- `./arduino-1.8.19/hardware/teensy/avr/boards.txt`
- `./arduino-1.8.19/hardware/teensy/avr/cores/teensy/Stream.h`
- `./arduino-1.8.19/hardware/teensy/avr/cores/teensy3/Stream.h`
- `./arduino-1.8.19/hardware/teensy/avr/cores/teensy4/Stream.h`

Whenever a new version is installed, remember to copy this file into the new version directory. For example, new version is x.yy.zz
This file must be copied into the directory:

- `./arduino-x.yy.zz/hardware/teensy/avr/boards.txt`
- `./arduino-x.yy.zz/hardware/teensy/avr/cores/teensy/Stream.h`
- `./arduino-x.yy.zz/hardware/teensy/avr/cores/teensy3/Stream.h`
- `./arduino-x.yy.zz/hardware/teensy/avr/cores/teensy4/Stream.h`

---
---

### HOWTO Fix `Multiple Definitions` Linker Error

The current library implementation, using `xyz-Impl.h` instead of standard `xyz.cpp`, possibly creates certain `Multiple Definitions` Linker error in certain use cases.

You can include this `.hpp` file

```
// Can be included as many times as necessary, without `Multiple Definitions` Linker Error
#include "Teensy_Slow_PWM.hpp"    //https://github.com/khoih-prog/Teensy_Slow_PWM
```

in many files. But be sure to use the following `.h` file **in just 1 `.h`, `.cpp` or `.ino` file**, which must **not be included in any other file**, to avoid `Multiple Definitions` Linker Error

```
// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "Teensy_Slow_PWM.h"      //https://github.com/khoih-prog/Teensy_Slow_PWM
```

Check the new [**multiFileProject** example](examples/multiFileProject) for a `HOWTO` demo.

Have a look at the discussion in [Different behaviour using the src_cpp or src_h lib #80](https://github.com/khoih-prog/ESPAsync_WiFiManager/discussions/80)



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
 6. [**multiFileProject**](examples/multiFileProject) **New**

 
---
---

### Example [ISR_8_PWMs_Array_Complex](examples/ISR_8_PWMs_Array_Complex)

https://github.com/khoih-prog/Teensy_Slow_PWM/blob/cdd23d2c5f0594d8453779c754877a98a35d2dcb/examples/ISR_8_PWMs_Array_Complex/ISR_8_PWMs_Array_Complex.ino#L16-L469


---
---

### Debug Terminal Output Samples

### 1. ISR_8_PWMs_Array_Complex on Teensy 4.1

The following is the sample terminal output when running example [ISR_8_PWMs_Array_Complex](examples/ISR_8_PWMs_Array_Complex) on **Teensy 4.1** to demonstrate how to use multiple PWM channels with complex callback functions, the accuracy of ISR Hardware PWM-channels, **especially when system is very busy**.  The ISR PWM-channels is **running exactly according to corresponding programmed periods and duty-cycles**


```
Starting ISR_8_PWMs_Array_Complex on Teensy 4.1
Teensy_Slow_PWM v1.2.1
CPU Frequency = 600 MHz
[PWM] TEENSY_TIMER_1: , F_BUS_ACTUAL (MHz) = 150
[PWM] Request interval = 10 , actual interval (us) = 10
[PWM] Prescale = 0 , _timerCount = 750
Starting  ITimer OK, micros() = 3017009
Channel : 0	    Period : 1000000		OnTime : 50000	Start_Time : 3017012
Channel : 1	    Period : 500000		OnTime : 50000	Start_Time : 3017017
Channel : 2	    Period : 333333		OnTime : 66666	Start_Time : 3017022
Channel : 3	    Period : 250000		OnTime : 75000	Start_Time : 3017027
Channel : 4	    Period : 200000		OnTime : 80000	Start_Time : 3017032
Channel : 5	    Period : 166666		OnTime : 74999	Start_Time : 3017037
Channel : 6	    Period : 142857		OnTime : 71428	Start_Time : 3017043
Channel : 7	    Period : 125000		OnTime : 68750	Start_Time : 3017048
SimpleTimer (us): 2000, us : 13017053, Dus : 10000043
PWM Channel : 0, prog Period (ms): 1000.00, actual : 1000006, prog DutyCycle : 5.00, actual : 5.00
PWM Channel : 1, prog Period (ms): 500.00, actual : 500003, prog DutyCycle : 10.00, actual : 10.00
PWM Channel : 2, prog Period (ms): 333.33, actual : 333342, prog DutyCycle : 20.00, actual : 20.00
PWM Channel : 3, prog Period (ms): 250.00, actual : 250006, prog DutyCycle : 30.00, actual : 30.00
PWM Channel : 4, prog Period (ms): 200.00, actual : 200004, prog DutyCycle : 40.00, actual : 40.00
PWM Channel : 5, prog Period (ms): 166.67, actual : 166671, prog DutyCycle : 45.00, actual : 44.99
PWM Channel : 6, prog Period (ms): 142.86, actual : 142865, prog DutyCycle : 50.00, actual : 50.00
PWM Channel : 7, prog Period (ms): 125.00, actual : 125003, prog DutyCycle : 55.00, actual : 55.00
SimpleTimer (us): 2000, us : 23017098, Dus : 10000045
PWM Channel : 0, prog Period (ms): 1000.00, actual : 1000006, prog DutyCycle : 5.00, actual : 5.00
PWM Channel : 1, prog Period (ms): 500.00, actual : 500003, prog DutyCycle : 10.00, actual : 10.00
PWM Channel : 2, prog Period (ms): 333.33, actual : 333342, prog DutyCycle : 20.00, actual : 20.00
PWM Channel : 3, prog Period (ms): 250.00, actual : 250007, prog DutyCycle : 30.00, actual : 30.00
PWM Channel : 4, prog Period (ms): 200.00, actual : 200003, prog DutyCycle : 40.00, actual : 40.00
PWM Channel : 5, prog Period (ms): 166.67, actual : 166671, prog DutyCycle : 45.00, actual : 44.99
PWM Channel : 6, prog Period (ms): 142.86, actual : 142866, prog DutyCycle : 50.00, actual : 50.00
PWM Channel : 7, prog Period (ms): 125.00, actual : 125003, prog DutyCycle : 55.00, actual : 55.00
SimpleTimer (us): 2000, us : 33017142, Dus : 10000044
PWM Channel : 0, prog Period (ms): 1000.00, actual : 1000006, prog DutyCycle : 5.00, actual : 5.00
PWM Channel : 1, prog Period (ms): 500.00, actual : 500003, prog DutyCycle : 10.00, actual : 10.00
PWM Channel : 2, prog Period (ms): 333.33, actual : 333342, prog DutyCycle : 20.00, actual : 20.00
PWM Channel : 3, prog Period (ms): 250.00, actual : 250006, prog DutyCycle : 30.00, actual : 30.00
PWM Channel : 4, prog Period (ms): 200.00, actual : 200003, prog DutyCycle : 40.00, actual : 40.00
PWM Channel : 5, prog Period (ms): 166.67, actual : 166671, prog DutyCycle : 45.00, actual : 44.99
PWM Channel : 6, prog Period (ms): 142.86, actual : 142865, prog DutyCycle : 50.00, actual : 50.00
PWM Channel : 7, prog Period (ms): 125.00, actual : 125004, prog DutyCycle : 55.00, actual : 54.99
SimpleTimer (us): 2000, us : 43017187, Dus : 10000045
PWM Channel : 0, prog Period (ms): 1000.00, actual : 1000006, prog DutyCycle : 5.00, actual : 5.00
PWM Channel : 1, prog Period (ms): 500.00, actual : 500003, prog DutyCycle : 10.00, actual : 10.00
PWM Channel : 2, prog Period (ms): 333.33, actual : 333342, prog DutyCycle : 20.00, actual : 20.00
PWM Channel : 3, prog Period (ms): 250.00, actual : 250007, prog DutyCycle : 30.00, actual : 30.00
PWM Channel : 4, prog Period (ms): 200.00, actual : 200004, prog DutyCycle : 40.00, actual : 40.00
PWM Channel : 5, prog Period (ms): 166.67, actual : 166671, prog DutyCycle : 45.00, actual : 44.99
PWM Channel : 6, prog Period (ms): 142.86, actual : 142865, prog DutyCycle : 50.00, actual : 50.00
PWM Channel : 7, prog Period (ms): 125.00, actual : 125003, prog DutyCycle : 55.00, actual : 55.00
```

---

### 2. ISR_8_PWMs_Array on Teensy 4.1

The following is the sample terminal output when running example [**ISR_8_PWMs_Array**](examples/ISR_8_PWMs_Array) on **on Teensy 4.1** to demonstrate how to use multiple PWM channels with simple callback functions.

```
Starting ISR_8_PWMs_Array on Teensy 4.1
Teensy_Slow_PWM v1.2.1
CPU Frequency = 600 MHz
[PWM] TEENSY_TIMER_1: , F_BUS_ACTUAL (MHz) = 150
[PWM] Request interval = 10 , actual interval (us) = 10
[PWM] Prescale = 0 , _timerCount = 750
Starting ITimer OK, micros() = 2938009
Channel : 0	    Period : 1000000		OnTime : 50000	Start_Time : 2938012
Channel : 1	    Period : 500000		OnTime : 50000	Start_Time : 2938017
Channel : 2	    Period : 333333		OnTime : 66666	Start_Time : 2938022
Channel : 3	    Period : 250000		OnTime : 75000	Start_Time : 2938027
Channel : 4	    Period : 200000		OnTime : 80000	Start_Time : 2938032
Channel : 5	    Period : 166666		OnTime : 74999	Start_Time : 2938037
Channel : 6	    Period : 142857		OnTime : 71428	Start_Time : 2938042
Channel : 7	    Period : 125000		OnTime : 68750	Start_Time : 2938047
```

---

### 3. ISR_8_PWMs_Array_Simple on Teensy 4.1

The following is the sample terminal output when running example [**ISR_8_PWMs_Array_Simple**](examples/ISR_8_PWMs_Array_Simple) on **on Teensy 4.1** to demonstrate how to use multiple PWM channels.

```
Starting ISR_8_PWMs_Array_Simple on Teensy 4.1
Teensy_Slow_PWM v1.2.1
CPU Frequency = 600 MHz
[PWM] TEENSY_TIMER_1: , F_BUS_ACTUAL (MHz) = 150
[PWM] Request interval = 10 , actual interval (us) = 10
[PWM] Prescale = 0 , _timerCount = 750
Starting ITimer OK, micros() = 3220009
Channel : 0	    Period : 1000000		OnTime : 50000	Start_Time : 3220012
Channel : 1	    Period : 500000		OnTime : 50000	Start_Time : 3220017
Channel : 2	    Period : 333333		OnTime : 66666	Start_Time : 3220022
Channel : 3	    Period : 250000		OnTime : 75000	Start_Time : 3220027
Channel : 4	    Period : 200000		OnTime : 80000	Start_Time : 3220032
Channel : 5	    Period : 166666		OnTime : 74999	Start_Time : 3220037
Channel : 6	    Period : 142857		OnTime : 71428	Start_Time : 3220043
Channel : 7	    Period : 125000		OnTime : 68750	Start_Time : 3220048
```

---

### 4. ISR_Modify_PWM on Teensy 4.1

The following is the sample terminal output when running example [ISR_Modify_PWM](examples/ISR_Modify_PWM) on **Teensy 4.1** to demonstrate how to modify PWM settings on-the-fly without deleting the PWM channel

```
Starting ISR_Modify_PWM on Teensy 4.1
Teensy_Slow_PWM v1.2.1
CPU Frequency = 600 MHz
[PWM] TEENSY_TIMER_1: , F_BUS_ACTUAL (MHz) = 150
[PWM] Request interval = 33 , actual interval (us) = 33
[PWM] Prescale = 0 , _timerCount = 2475
Starting ITimer OK, micros() = 2531009
Using PWM Freq = 200.00, PWM DutyCycle = 1.00
Channel : 0	    Period : 5000		OnTime : 50	Start_Time : 2531014
Channel : 0	New Period : 10000		OnTime : 555	Start_Time : 12529921
Channel : 0	New Period : 5000		OnTime : 50	Start_Time : 22525957
Channel : 0	New Period : 10000		OnTime : 555	Start_Time : 32529848
Channel : 0	New Period : 5000		OnTime : 50	Start_Time : 42525884
Channel : 0	New Period : 10000		OnTime : 555	Start_Time : 52534793
Channel : 0	New Period : 5000		OnTime : 50	Start_Time : 62530829
Channel : 0	New Period : 10000		OnTime : 555	Start_Time : 72534720
Channel : 0	New Period : 5000		OnTime : 50	Start_Time : 82530756
Channel : 0	New Period : 10000		OnTime : 555	Start_Time : 92539665
Channel : 0	New Period : 5000		OnTime : 50	Start_Time : 102535701
Channel : 0	New Period : 10000		OnTime : 555	Start_Time : 112539593
Channel : 0	New Period : 5000		OnTime : 50	Start_Time : 122535629
```

---

### 5. ISR_Changing_PWM on Teensy 4.1

The following is the sample terminal output when running example [ISR_Changing_PWM](examples/ISR_Changing_PWM) on **Teensy 4.1** to demonstrate how to modify PWM settings on-the-fly by deleting the PWM channel and reinit the PWM channel

```
Starting ISR_Changing_PWM on Teensy 4.1
Teensy_Slow_PWM v1.2.1
CPU Frequency = 600 MHz
[PWM] TEENSY_TIMER_1: , F_BUS_ACTUAL (MHz) = 150
[PWM] Request interval = 33 , actual interval (us) = 33
[PWM] Prescale = 0 , _timerCount = 2475
Starting ITimer OK, micros() = 3617009
Using PWM Freq = 1.00, PWM DutyCycle = 50.00
Channel : 0	    Period : 1000000		OnTime : 500000	Start_Time : 3617014
Using PWM Freq = 2.00, PWM DutyCycle = 90.00
Channel : 0	    Period : 500000		OnTime : 450000	Start_Time : 13617022
Using PWM Freq = 1.00, PWM DutyCycle = 50.00
Channel : 0	    Period : 1000000		OnTime : 500000	Start_Time : 23617030
Using PWM Freq = 2.00, PWM DutyCycle = 90.00
Channel : 0	    Period : 500000		OnTime : 450000	Start_Time : 33617038
Using PWM Freq = 1.00, PWM DutyCycle = 50.00
Channel : 0	    Period : 1000000		OnTime : 500000	Start_Time : 43617046
Using PWM Freq = 2.00, PWM DutyCycle = 90.00
Channel : 0	    Period : 500000		OnTime : 450000	Start_Time : 53617054
Using PWM Freq = 1.00, PWM DutyCycle = 50.00
Channel : 0	    Period : 1000000		OnTime : 500000	Start_Time : 63617062
Using PWM Freq = 2.00, PWM DutyCycle = 90.00
Channel : 0	    Period : 500000		OnTime : 450000	Start_Time : 73617070
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
4. Fix `multiple-definitions` linker error
5. Optimize library code by using `reference-passing` instead of `value-passing`
6. Improve accuracy by using `float`, instead of `uint32_t` for `dutycycle`
7. DutyCycle to be optionally updated at the end current PWM period instead of immediately.
8. Display informational warning only when `_PWM_LOGLEVEL_` > 3

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


