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

#include <string.h>
#include <stdio.h>
#include "isos_debug_basic.h"

#define PRINT_RESOURCE_EVENT 1
#define PRINT_SUBTASK_EVENT 1
#define PRINT_DUE_TASK_HEADER 1

char* IsosDebugBasic_TaskTypeToString(IsosTaskType type){
  switch(type){
    case IsosTaskType_RunOnce: return "NC-RO";
    case IsosTaskType_Resource: return "NC-RS";
    case IsosTaskType_LooselyRepeated: return "CY-LR";
    case IsosTaskType_Repeated: return "CY-RE";
    case IsosTaskType_Periodic: return "CY-PR";
  }
  return "UNKNOWN";
}

char* IsosDebugBasic_ResourceTypeToString(IsosResourceTaskType type){
  switch(type){
    case IsosResourceTaskType_Type1: return "Type 1";
    case IsosResourceTaskType_Type2: return "Type 2";
    case IsosResourceTaskType_Type3: return "Type 3";
    case IsosResourceTaskType_Type4: return "Type 4";
    case IsosResourceTaskType_Type5: return "Type 5";
    case IsosResourceTaskType_Type6: return "Type 6";
    case IsosResourceTaskType_Type7: return "Type 7";
    case IsosResourceTaskType_Type8: return "Type 8";
    case IsosResourceTaskType_Unspecified:
    default: return "Unspecified";
  }
}

char* IsosDebugBasic_TaskStateToString(IsosTaskState state){
  switch(state){
    case IsosTaskState_Failed: return "Failed";
    case IsosTaskState_Initial: return "Started";
    case IsosTaskState_Running: return "Running";
    case IsosTaskState_Success: return "Completed Successfully";
    case IsosTaskState_Suspended: return "Suspended";
    case IsosTaskState_Undefined:
    default: return "in Unknown state";
  }
}

void IsosDebugBasic_PrintFrontBlank(){ printf("              "); }

void IsosDebugBasic_PrintResourceClaiming(IsosResourceTaskType type, char result, unsigned char id){
  if (PRINT_RESOURCE_EVENT){
    IsosDebugBasic_PrintFrontBlank();
    printf("Claiming resource [%s] [Task Id: %02d]: ", IsosDebugBasic_ResourceTypeToString(type), id);
    printf(result ? "Successful\n" : "Failed\n");
  }
}

void IsosDebugBasic_PrintResourceChecking(IsosResourceTaskType type, IsosTaskState state, unsigned char id){
  if (PRINT_RESOURCE_EVENT){
    IsosDebugBasic_PrintFrontBlank();
    printf("Checking resource [%s] [Task Id: %02d]: %s\n", IsosDebugBasic_ResourceTypeToString(type), id, IsosDebugBasic_TaskStateToString(state));
  }
}

void IsosDebugBasic_PrintResourceReleasing(IsosResourceTaskType type, unsigned char id){
  if (PRINT_RESOURCE_EVENT){
    IsosDebugBasic_PrintFrontBlank();
    printf("Releasing resource [%s] [Task Id: %02d]...\n", IsosDebugBasic_ResourceTypeToString(type), id);
  }
}

//Ensuring the value of clock to be unchanged inside the function
void IsosDebugBasic_GetPrintClock(const IsosClock* clock, char* results){
  short dayPart = clock->Day;
  long mMsPart = clock->Ms / 1000000;
  long kMsPart = (clock->Ms / 1000) % 1000;
  long msPart = clock->Ms % 1000;
  results[0] = 0x30 + dayPart / 100;
  results[1] = 0x30 + ((dayPart / 10) % 10);
  results[2] = 0x30 + dayPart % 10;
  results[3] = '-';
  results[4] = 0x30 + ((mMsPart / 10) % 10);
  results[5] = 0x30 + mMsPart % 10;
  results[6] = 0x30 + kMsPart / 100;
  results[7] = 0x30 + ((kMsPart / 10) % 10);
  results[8] = 0x30 + kMsPart % 10;
  results[9] = 0x30 + msPart / 100;
  results[10] = 0x30 + ((msPart / 10) % 10);
  results[11] = 0x30 + msPart % 10;
  results[12] = '\0'; //always this one must be the last character
}

void IsosDebugBasic_PrintClock(const IsosClock* clock){
  char results[13];
  IsosDebugBasic_GetPrintClock(clock, results);
  printf("%s", results);
}

void IsosDebugBasic_PrintTaskInfo(const IsosTaskInfo* taskInfo){
  char mainClockResults[13], clockResults[13];
  IsosClock mainClock = Isos_GetClock();
  IsosDebugBasic_GetPrintClock(&mainClock, mainClockResults);
  IsosDebugBasic_GetPrintClock(&taskInfo->TimeInfo.Any, clockResults);
  printf("%s: Task %02d-S%02d [%s P%03d] T:%s is %s\n", mainClockResults, //printing like this is a lot easier to see the format
         taskInfo->Id, taskInfo->ActionInfo.Subtask, IsosDebugBasic_TaskTypeToString(taskInfo->Type), taskInfo->Priority,
         clockResults, IsosDebugBasic_TaskStateToString(taskInfo->ActionInfo.State));
}

void IsosDebugBasic_PrintDueTasks(const IsosDueTask* dueTask, short dueSize){
  short i;
  IsosClock mainClock = Isos_GetClock();
  char mainClockResults[13];
  if (dueSize <= 0)
    return;
  if (PRINT_DUE_TASK_HEADER){
    IsosDebugBasic_GetPrintClock(&mainClock, mainClockResults);
    printf("%s: Due Task(s): [", mainClockResults);
    for (i = dueSize - 1; i >= 0; --i){
      if (i < dueSize - 1)
        printf(", ");
      printf("%02d-P%03d", dueTask[i].TaskId, dueTask[i].Priority);
    }
    printf("]\n");
    printf("------------------------------------------------------------------------------------------------\n");
  }
}

void IsosDebugBasic_PrintDueTasksEnding(short dueSize){
  if (PRINT_DUE_TASK_HEADER)
    if (dueSize > 0)
      printf("\n");
}

void IsosDebugBasic_PrintSubtaskNote(char subtaskCase, short subtaskDirectionNo){
  if (PRINT_SUBTASK_EVENT){
    IsosDebugBasic_PrintFrontBlank();
    if (subtaskCase > 0){
      printf("Successful case, move to Subtask %d\n", subtaskDirectionNo);
    } else if (subtaskCase == 0){
      printf("Waiting resource to finish, stay in Subtask %d\n", subtaskDirectionNo);
    } else {
      printf("Failed case, move to Subtask %d\n", subtaskDirectionNo);
    }
  }
}

void IsosDebugBasic_PrintWaitingNote(const IsosTaskInfo* taskInfo){
  if (PRINT_SUBTASK_EVENT){
    IsosDebugBasic_PrintFrontBlank();
    printf("Task [%d] is [Suspended] until T:", taskInfo->Id);
    IsosDebugBasic_PrintClock(&taskInfo->SuspensionDue);
    printf("\n");
  }
}

void IsosDebugBasic_PrintEndWaitingNote(const IsosTaskInfo* taskInfo){
  if (PRINT_SUBTASK_EVENT)
    printf("[Note]      : Task [%d] suspension time is over\n", taskInfo->Id);
}


