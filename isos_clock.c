/*Created by Ian K (Mar-2019)
  Inspire Satellite Operating System (ISOS) is an (1) easy-to-understand, (2) easy-to-use,
  (3) small-sized, (4) fairly-comprehensive Operating System (OS) software used in Inspire Satellite-4 (IS-4)
  flight software as a replacement for the earlier non-OS flight software developed fore Inspire Satellite-1 (IS-1).
  (1) Easy-to-understand:
      -> Uses simple, straight forward, yet efficient C programming style
      -> Does not use stack pointers manipulation at all, so it is fully compatible with any code optimizers
      -> Very clear naming of the files, variables, and functions - leaving little rooms for guessing what they stand for
      -> No convoluted/fanciful-looking macros, code, etc, just pure, simple C
      -> No deeply-nested/recursive code for everything (quick_sort algorithm excepted), making it very easy to read, even for beginner C coder
      -> Uses (i) simple switch case and (ii) concept of subtask, instead of stack pointers manipulation to control segmentation of the tasks in a task function
  (2) Easy-to-use:
      -> One line code to initialize
      -> One line code to register each of the various type of tasks
      -> One line code to run the OS
         Note: See note (TODO) in the Isos_Run() function to implement
      -> One file to configure the OS settings (Isos_task.h), with only six (6) macros to set
      -> Demonstrations/examples provided
  (3) Small-sized
      -> The entire OS code is less than 1000 lines
      -> Four (4) small C + header files only (+1 quick_sort and +1 debugging file-pairs)
  (4) Fairly-comprehensive
      -> Sufficient for most of small-to-medium micro-controller OS development project requirements
      -> Capable of handling various type of tasks with different time-cycle/non-cycle requirements
      -> Capable of handling multiple level of priorities of the tasks
      -> Capable of managing use of shared resources among the tasks using simple claiming-releasing mechanism
      -> Capable of segmenting single-function task into various task segments (called subtask)
      -> Capable of handling task waiting and simple inter-task signals using char[] flags

  isos_clock.c, isos_clock.h
  - Describe the task clock used in ISOS
  - Provide arithmetic operation functions of the task clock
*/

#include <stdio.h>
#include <string.h>
#include "isos_clock.h"

IsosClock IsosClock_Create(short day, long ms){
  IsosClock clock;
  clock.Day = day;
  clock.Ms = ms;
  return clock;
}

//if both are positive, then it is ok
//if both are negative, then it is also ok
//if day is positive and ms is negative, it is ok
//if day is negative and ms is positive, then try to make the ms negative instead
//assumes the data is at most 1 day difference, no overflow
void IsosClock_Adjust(IsosClock *clock){
  while (clock->Ms >= MS_PER_DAY){ //too big for ms
    clock->Ms -= MS_PER_DAY; //minus the ms per day
    clock->Day++; //adds the day
  }
  //adjust only once
  if (clock->Day > 0 && clock->Ms < 0){
    clock->Day--;
    clock->Ms += MS_PER_DAY;
  } else if (clock->Day < 0 && clock->Ms > 0){
    clock->Day++;
    clock->Ms -= MS_PER_DAY;
  }
}

//Ideally, add clock is always positive
IsosClock IsosClock_Add(const IsosClock* clock, const IsosClock* addClock){
  IsosClock resultClock;
  resultClock = IsosClock_Create(clock->Day + addClock->Day, clock->Ms + addClock->Ms);
  IsosClock_Adjust(&resultClock);
  return resultClock;
}

IsosClock IsosClock_Minus(const IsosClock *clock, const IsosClock *minusClock){
  IsosClock resultClock;
  resultClock = IsosClock_Create(clock->Day - minusClock->Day, clock->Ms - minusClock->Ms);
  IsosClock_Adjust(&resultClock);
  return resultClock;
}

//The input is an adjusted clock
int IsosClock_GetDirection(const IsosClock *adjustedClock){
  if(adjustedClock->Day == 0 && adjustedClock->Ms == 0)
    return 0; //this is neutral clock direction
  if(adjustedClock->Day > 0)
    return 1; //always positive
  else if(adjustedClock->Day == 0)
    return adjustedClock->Ms > 0 ? 1 : -1; //the case for 0 would have been taken cared of
  return -1; //otherwise it is always negative result
}
