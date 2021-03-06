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

  isos.c, isos.h
  - The main files of the ISOS
  - Provide all internal variables used in the ISOS (main clock, scheduler period, task/resource task/due lists, etc)
  - Provide all major OS related functions (task registration, scheduler, task execution, OS running, clock ticking, etc)
  - Provide supporting functions to use in the task implementation (get main clock time, get task flags, claim/release resources, etc)
*/

#ifndef ISOS_H
#define ISOS_H

#include "isos_task.h"
#include "isos_buffer.h"

//To tell the OS about the resource task type being stored, used for mapping it to the task id
typedef enum IsosResourceTaskTypeEnum {
  IsosResourceTaskType_Unspecified = -1,
  IsosResourceTaskType_Type1 = 0,
  IsosResourceTaskType_Type2,
  IsosResourceTaskType_Type3,
  IsosResourceTaskType_Type4,
  IsosResourceTaskType_Type5,
  IsosResourceTaskType_Type6,
  IsosResourceTaskType_Type7,
  IsosResourceTaskType_Type8
} IsosResourceTaskType;

typedef struct IsosTaskStruct {
  IsosTaskInfo Info;
  void (*Action)(unsigned char, IsosTaskActionInfo*);
} IsosTask;

typedef struct IsosDueTaskStruct {
  short TaskId; //the task index of the due task
  unsigned char Priority; //the task priority of the due task
} IsosDueTask;

//Initialization
void Isos_Init();

//Utility functions
IsosClock Isos_GetClock();
unsigned char Isos_GetTaskFlags(unsigned char taskId, unsigned char flagNo);
IsosTask* Isos_GetTask(unsigned char taskId); //intended to be called by "super user" outside
short Isos_GetTaskSize();
void Isos_SetTaskTimeout(unsigned char taskId, short timeoutDay, long timeoutMs);

//Task registration
char Isos_RegisterNonCyclicalTask(char enabled, short executionDueDay, long executionDueMs, short timeoutDay, long timeoutMs,
                              unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*));
char Isos_RegisterResourceTaskWithBuffers(IsosResourceTaskType resourceType, short timeoutDay, long timeoutMs,
                                          unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*),
                                          unsigned char* txBuffer, short txBufferSize, unsigned char* rxBuffer, short rxBufferSize);
char Isos_RegisterResourceTaskWithBuffer(IsosResourceTaskType resourceType, short timeoutDay, long timeoutMs,
                                         unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*),
                                         char isTxBuffer, unsigned char* buffer, short bufferSize);
char Isos_RegisterResourceTask(IsosResourceTaskType resourceType, short timeoutDay, long timeoutMs,
                               unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*));
char Isos_RegisterLooselyRepeatedTask(char enabled, short periodDay, long periodMs, short timeoutDay, long timeoutMs,
                                      unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*));
char Isos_RegisterRepeatedTask(char enabled, short periodDay, long periodMs, short timeoutDay, long timeoutMs,
                               unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*));
char Isos_RegisterPeriodicTask(char enabled, short periodDay, long periodMs, short timeoutDay, long timeoutMs,
                               unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*));

//Tasks scheduling and execution functions
void Isos_ScheduleNonCyclicalTask(IsosTaskInfo* taskInfo, unsigned char priority, char withReset, short executionDueDay, long executionDueMs);
void Isos_DueNonCyclicalOrResourceTaskNow(IsosTaskInfo* taskInfo, unsigned char priority, char withReset);
void Isos_DueTaskNow(IsosTaskInfo* taskInfo, unsigned char priority, char withReset);
void Isos_Run();
void Isos_Wait(unsigned char taskId, short waitingDay, long waitingMs);
void Isos_WaitFromSuspensionTime(unsigned char taskId);
void Isos_Tick();

//Resource tasks related functions
char Isos_ClaimResourceTask(unsigned char claimerTaskId, IsosResourceTaskType type);
char Isos_PrepareResourceTaskTx(IsosResourceTaskType type, unsigned char* txData, short txDataSize);
char Isos_PrepareResourceTaskTxWithSizeReturn(IsosResourceTaskType type, unsigned char* txData, short txDataSize, short expectedRxDataSize);
char Isos_PrepareResourceTaskTxWithTimeReturn(IsosResourceTaskType type, unsigned char* txData, short txDataSize, short waitRxDay, long waitRxMs);
IsosTaskState Isos_GetResourceTaskState(IsosResourceTaskType type);
char Isos_PeekResourceTaskRx(IsosResourceTaskType type, unsigned char* rxDataBuffer, short rxDataSize);
char Isos_GetResourceTaskRx(IsosResourceTaskType type, unsigned char* rxDataBuffer, short rxDataSize);
void Isos_ReleaseResourceTask(IsosResourceTaskType type);
void Isos_FlushResourceTaskTx(IsosResourceTaskType type);
void Isos_FlushResourceTaskRx(IsosResourceTaskType type);
short Isos_GetResourceTaskTxDataSize(IsosResourceTaskType type);
short Isos_GetResourceTaskRxDataSize(IsosResourceTaskType type);
char Isos_ResourceTaskHasExpectedDataSize(IsosResourceTaskType type, char isTx);
IsosBuffer* Isos_GetResourceTaskBuffer(char* result, IsosResourceTaskType type, char isTx); //To be used by ISR to get the needed buffer
char Isos_GetResourceTaskBufferFlags(IsosResourceTaskType type); //0: no buffer, 1:Tx, 2:Rx, 3:TxRx

#endif
