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

#include <stdio.h>
#include <string.h>
#include "isos_task.h"

IsosClock IsosTask_getCycleTaskNextDue(const IsosTaskInfo *taskInfo){
  //Use internally, IsosTaskType_RunOnce and IsosTaskType_Resource cases need not be handled
  switch(taskInfo->Type){
  case IsosTaskType_LooselyRepeated:
    return IsosClock_Add(&taskInfo->LastFinished, &taskInfo->TimeInfo.Period);
  case IsosTaskType_Repeated:
    return IsosClock_Add(&taskInfo->LastExecuted, &taskInfo->TimeInfo.Period);
  default:
    return IsosClock_Add(&taskInfo->LastDueReported, &taskInfo->TimeInfo.Period); //default case, assume periodic task
  }
}

IsosClock IsosTask_getCycleTaskDiffToNextDue(const IsosClock* mainClock, const IsosTaskInfo *taskInfo){
  IsosClock clock = IsosTask_getCycleTaskNextDue(taskInfo);
  return IsosClock_Minus(mainClock, &clock);
}

//Function to check if a task is due, task which is already due should be checked here in the first place
char IsosTask_IsDue(const IsosClock* mainClock, const IsosTaskInfo *taskInfo){
  IsosClock clock; //if run once or resource task, check if it is due compared to the main clock, periodic or repeated, then just
  clock = taskInfo->Type == IsosTaskType_RunOnce || taskInfo->Type == IsosTaskType_Resource ?
    IsosClock_Minus(mainClock, &taskInfo->TimeInfo.ExecutionDue) :
    IsosTask_getCycleTaskDiffToNextDue(mainClock, taskInfo);
  return IsosClock_GetDirection(&clock) >= 0;
}

void IsosTask_ClearActionFlags(IsosTaskActionInfo* taskActionInfo){
  memset(taskActionInfo->Flags, 0, sizeof(taskActionInfo->Flags));
}

void IsosTask_ResetState(IsosTaskInfo *taskInfo){
  IsosTask_ClearActionFlags(&taskInfo->ActionInfo);
  taskInfo->ActionInfo.Subtask = 0;
  taskInfo->ActionInfo.State = IsosTaskState_Initial;
  taskInfo->IsDueReported = 0;
  taskInfo->ForcedDue = 0;
}

char IsosTask_IsTimeout(const IsosClock* mainClock, const IsosTaskInfo *taskInfo){
  IsosClock clock;
  if (!taskInfo->Timeout.Day && !taskInfo->Timeout.Ms)
    return 0; //uninitialized timeout values means there is no timeout for this task
  clock = IsosClock_Minus(mainClock, &taskInfo->LastExecuted);
  clock = IsosClock_Minus(&taskInfo->Timeout, &clock);
  return IsosClock_GetDirection(&clock) <= 0; //means the elapsed time since last executed is greater than the allowed time for timeout
}
