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

  isos_debug_basic.c, isos_debug_basic.h
  - Provide basic debugging (printing functions) for demonstration of ISOS
*/

#ifndef ISOS_DEBUG_BASIC_H
#define ISOS_DEBUG_BASIC_H

#include "isos.h"

/* Bit-fields have certain restrictions. You cannot take the address of a bit-field. Bitfields
cannot be arrayed. They cannot be declared as static. You cannot know, from
machine to machine, whether the fields will run from right to left or from left to right;
this implies that any code using bit-fields may have some machine dependencies */

//Example on how to create bit-field in C
//-> unsigned [name]: [bit-field size]
typedef struct IsosFlagsExampleStruct {
  unsigned Flag00: 1;
  unsigned Flag01: 1;
  unsigned Flag02: 1;
  unsigned Flag03: 1;
  unsigned Flag04: 1;
  unsigned Flag05: 1;
  unsigned Flag06: 1;
  unsigned Flag07: 1;
  unsigned Flag0815: 8;
  unsigned Flag1623: 8;
  unsigned Flag2431: 8;
} IsosFlagsExample;

char* IsosDebugBasic_TaskTypeToString(IsosTaskType type);
char* IsosDebugBasic_ResourceTypeToString(IsosResourceTaskType type);
char* IsosDebugBasic_TaskStateToString(IsosTaskState state);
void IsosDebugBasic_PrintFrontBlank();
void IsosDebugBasic_PrintResourceClaiming(IsosResourceTaskType type, char result, unsigned char id);
void IsosDebugBasic_PrintResourceChecking(IsosResourceTaskType type, IsosTaskState state, unsigned char id);
void IsosDebugBasic_PrintResourceReleasing(IsosResourceTaskType type, unsigned char id);
void IsosDebugBasic_GetPrintClock(const IsosClock* clock, char* results);
void IsosDebugBasic_PrintClock(const IsosClock* clock);
void IsosDebugBasic_PrintTaskInfo(const IsosTaskInfo* taskInfo);
void IsosDebugBasic_PrintDueTasks(const IsosDueTask* dueTask, short dueSize);
void IsosDebugBasic_PrintDueTasksEnding(short dueSize);
void IsosDebugBasic_PrintSubtaskNote(char subtaskCase, short subtaskDirectionNo);
void IsosDebugBasic_PrintWaitingNote(const IsosTaskInfo* taskInfo);
void IsosDebugBasic_PrintEndWaitingNote(const IsosTaskInfo* taskInfo);

#endif
