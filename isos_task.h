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

  isos_task.c, isos_task.h
  - Describe the task information structures and functions used in ISOS
  - Describe different task-state and task-types used in ISOS
  - isos_task.h is the only file used to configure ISOS settings (with only six (6) macros to set)
*/

#ifndef ISOS_TASK_H
#define ISOS_TASK_H

#include "isos_clock.h"

//DO NOT configure all these "MIN_" macros
#define MIN_TASK_FLAGS_SIZE 3 //task flags size should NOT be less than 3
#define MIN_TASK_SIZE 2 //DO NOT CHANGE!!! MIN_TASK_SIZE CANNOT BE less than 2
#define MIN_PRIORITY 0 //default priority

//Configure only the macros below
#define TASK_FLAGS_SIZE 4
#define MAX_TASK_SIZE 48 //assume maximum of 48 tasks for now, put this between 2 to 127 (can actually be more, but not tested yet...)
#define MAX_PRIORITY 100 //like immediately needs to be run
#define CLOCK_PERIOD_DAY 0 //0-32,767 depending on the clock speed, this may be adjusted
#define CLOCK_PERIOD_MS 10 //0-86,399,999 depending on the clock speed, this may be adjusted
#define RESOURCE_SIZE 8 //should be identical number with enum IsosResourceTaskType

enum IsosTaskType {
    //Non-cyclical
    IsosTaskType_RunOnce,  //used for task needs to be run only once
    IsosTaskType_Resource, //used to mark resource task type, otherwise actually very similar to run once

    //Cyclical
    IsosTaskType_LooselyRepeated, //when delayed, start over the period calculation for the next execution from the last time the task is finished
    IsosTaskType_Repeated, //when delayed, start over the period calculation for the next execution from the last time the task is executed
    IsosTaskType_Periodic, //when delayed, start over the period calculation for the next execution from the last time the task is due
};

enum IsosTaskState {
    IsosTaskState_Undefined = -1, //not used most of the time
    IsosTaskState_Initial, //is initialized, the very first time
    IsosTaskState_Running, //running - does not mean executed
    IsosTaskState_Suspended, //suspended for whatever reason
    IsosTaskState_Failed, //finished and failed
    IsosTaskState_Success, //finished and successful
};

//The shared info means it is shared with the task action which runs it
struct IsosTaskActionInfo {
    enum IsosTaskState State; //The state of the task - so that it can be changed to success, failed, or suspended
    char Enabled; //flag to specify if a task is enabled or not - so that the task action can disabled its own task if necessary
    unsigned char Subtask; //just a number indicating its current subtask - so that the task action can change its own subtask
    unsigned char Flags[TASK_FLAGS_SIZE]; //just simple, additional semaphore flags to indicate task result, if there is any - so that the task action can share its results
    //for resource task, Flags = Next Claimer Flag | Next Claimer Id | Next Claimer Priority | Reserved
};

struct IsosTaskInfo {
    //Declaring the task Id outside is useless, since it will be determined by the ISOS on registration...
    unsigned char Id; //The Id of the task, to be used for arrangement, basically the same as index of the task in the register
    unsigned char Priority; //the higher the more priority
    struct IsosTaskActionInfo ActionInfo; //the action info of the task
    enum IsosTaskType Type; //The type of the task
    struct IsosClock LastDueReported; //The last time the task is reported to be on due (supposed to be executed)
    struct IsosClock LastExecuted; //The last time the task is executed (started to be executed)
    struct IsosClock LastFinished; //The last time the task is finished
    struct IsosClock Period; //The period of the task, not applied for "Once" task type
    struct IsosClock ExecutionDue; //The time for the task to be run - only applicable for RunOnce tasks
    struct IsosClock SuspensionDue; //The time for the task to be started to be considered in the scheduler again
    char IsDueReported; //flag to indicate if the due has been reported
    char ForcedDue; //special flag to forcefully run the task immediately
};

char IsosTask_IsDue(struct IsosClock* mainClock, struct IsosTaskInfo *taskInfo);
void IsosTask_ClearActionFlags(struct IsosTaskActionInfo* taskActionInfo);
void IsosTask_ResetState(struct IsosTaskInfo *taskInfo);

#endif
