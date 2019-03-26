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

  main.c, main.h
  - Demonstration on how ISOS works using simple Console program
  - Various simple examples on how to make tasks using ISOS
  - Simulate multiple tasks with different priorities run in the OS
  - Simulate clashes of resource tasks usage and how they are handled by the OS
  - Demonstrate how the tasks and resources sharing are handled well by the OS
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> //used for C to have time_t structure
#include <math.h> //used to generate random number
#include "main.h"
#include "isos_debug_basic.h"
#include "isos_utilities.h"

#define RESOURCE_3_RX_BUFFER_SIZE 256
#define RESOURCE_4_TX_BUFFER_SIZE 128
#define RESOURCE_5_TX_BUFFER_SIZE 64
#define RESOURCE_5_RX_BUFFER_SIZE 128
#define RESOURCE_6_TX_BUFFER_SIZE 512
#define RESOURCE_6_RX_BUFFER_SIZE 256
unsigned char Resource3RxBuffer[RESOURCE_3_RX_BUFFER_SIZE];
unsigned char Resource4TxBuffer[RESOURCE_4_TX_BUFFER_SIZE];
unsigned char Resource5TxBuffer[RESOURCE_5_TX_BUFFER_SIZE];
unsigned char Resource5RxBuffer[RESOURCE_5_RX_BUFFER_SIZE];
unsigned char Resource6TxBuffer[RESOURCE_6_TX_BUFFER_SIZE];
unsigned char Resource6RxBuffer[RESOURCE_6_RX_BUFFER_SIZE];

int main() {
  time_t t;
  srand((unsigned)time(&t)); //random seed

  IsosClock mainClock;
  char val = '\0';
  Isos_Init(); //the first to be called before registering any task
  registerTasks();

  while (1){ //While (1) to be used internally in the Isos_Run() function for actual implementation, see TODO note on Isos_Run() function
    mainClock = Isos_GetClock();
    if (mainClock.Ms > 0 && mainClock.Ms % 1000 == 0){
      printf("Press any character key but [x+Enter] to continue...\n");
      scanf(" %c", &val);
      if (val == 'x')
        break;
    }
    Isos_Run();
    Isos_Tick(); //to simulate the ticking from the interrupt, to be called in the interrupt per ms in the actual implementation
  }

  return 0;
}

void registerTasks(){
  Isos_RegisterNonCyclicalTask(1, 0, 500, 0, 0, 40, NonCyclicalTask1); //suppose this is antenna deployment
  Isos_RegisterNonCyclicalTask(1, 0, 800, 0, 0, 45, NonCyclicalTask2); //suppose this is solar panel deployment
  Isos_RegisterNonCyclicalTask(1, 0, 370, 0, 0, 5, NonCyclicalTask3); //purposely made to simulate interesting clash on 370ms time stamp
  Isos_RegisterLooselyRepeatedTask(1, 0, 100, 0, 0, 0, LooselyRepeatedTask1);
  Isos_RegisterLooselyRepeatedTask(1, 0, 150, 0, 0, 1, LooselyRepeatedTask2);
  Isos_RegisterLooselyRepeatedTask(1, 0, 400, 0, 0, 2, LooselyRepeatedTask3); //Added to test task waiting case
  Isos_RegisterLooselyRepeatedTask(1, 0, 180, 0, 0, 3, LooselyRepeatedTask4); //Added to test resource task with Rx buffer only
  Isos_RegisterLooselyRepeatedTask(1, 0, 220, 0, 0, 4, LooselyRepeatedTask5); //Added to test resource task with Tx buffer only
  Isos_RegisterRepeatedTask(1, 0, 200, 0, 0, 6, RepeatedTask1);
  Isos_RegisterRepeatedTask(1, 0, 300, 0, 0, 7, RepeatedTask2);
  Isos_RegisterRepeatedTask(1, 0, 120, 0, 0, 8, RepeatedTask3); //Added to test resource task with Tx & Rx buffers waiting by size case
  Isos_RegisterRepeatedTask(1, 0, 160, 0, 0, 9, RepeatedTask4); //Added to test resource task with Tx & Rx buffers waiting by time case
  Isos_RegisterRepeatedTask(1, 0, 200, 0, 0, 10, RepeatedTask5); //Added to test competing task for the same resource with a stuck-task
  Isos_RegisterPeriodicTask(1, 0, 200, 0, 0, 11, PeriodicTask1);
  Isos_RegisterPeriodicTask(1, 0, 250, 0, 0, 12, PeriodicTask2);
  Isos_RegisterPeriodicTask(1, 0, 300, 0, 0, 13, PeriodicTask3);
  Isos_RegisterPeriodicTask(1, 0, 350, 0, 0, 14, PeriodicTask4);
  Isos_RegisterPeriodicTask(1, 0, 190, 0, 30, 15, PeriodicTask5); //test OS response for a stuck-task claiming a resource
  Isos_RegisterPeriodicTask(1, 0, 280, 0, 0, 16, PeriodicTask6); //task which encounter occasional resource task's timeout

  //Better put all resource task priorities higher than all other tasks
  Isos_RegisterResourceTask(IsosResourceTaskType_Type1, 0, 0, MAX_PRIORITY-5, ResourceTask1);
  Isos_RegisterResourceTask(IsosResourceTaskType_Type2, 0, 0, MAX_PRIORITY-4, ResourceTask2);
  Isos_RegisterResourceTaskWithBuffer(IsosResourceTaskType_Type3, 0, 0, MAX_PRIORITY-3, ResourceTask3, //Added for resource task with Rx buffer only case
                                      0, Resource3RxBuffer, RESOURCE_3_RX_BUFFER_SIZE);
  Isos_RegisterResourceTaskWithBuffer(IsosResourceTaskType_Type4, 0, 0, MAX_PRIORITY-2, ResourceTask4, //Added for resource task with Tx buffer only case
                                      1, Resource4TxBuffer, RESOURCE_4_TX_BUFFER_SIZE);
  Isos_RegisterResourceTaskWithBuffers(IsosResourceTaskType_Type5, 0, 0, MAX_PRIORITY-1, ResourceTask5, //Added for resource task with Tx & Rx buffers waited by size case
                                       Resource5TxBuffer, RESOURCE_5_TX_BUFFER_SIZE, Resource5RxBuffer, RESOURCE_5_RX_BUFFER_SIZE);
  Isos_RegisterResourceTaskWithBuffers(IsosResourceTaskType_Type6, 0, 0, MAX_PRIORITY, ResourceTask6, //Added for resource task with Tx & Rx buffers waited by time case
                                       Resource6TxBuffer, RESOURCE_6_TX_BUFFER_SIZE, Resource6RxBuffer, RESOURCE_6_RX_BUFFER_SIZE);
  Isos_RegisterResourceTask(IsosResourceTaskType_Type7, 0, 0, MAX_PRIORITY-6, ResourceTask7);
  Isos_RegisterResourceTask(IsosResourceTaskType_Type8, 0, 30, MAX_PRIORITY-7, ResourceTask8);
}

void simulateCommonTask(IsosTaskActionInfo* taskActionInfo, int endSubtaskNo, IsosTaskState endState){
  if (taskActionInfo->Subtask == endSubtaskNo)
    taskActionInfo->State = endState;
  else
    taskActionInfo->Subtask++;
}

void simulateCommonTaskWithSuspension(unsigned char taskId, IsosTaskActionInfo* taskActionInfo, int endSubtaskNo, IsosTaskState endState,
                                      int waitingSubtaskNo, short waitingDay, long waitingMs){
  if (taskActionInfo->Subtask == endSubtaskNo)
    taskActionInfo->State = endState;
  else if(taskActionInfo->Subtask == waitingSubtaskNo){
    Isos_Wait(taskId, waitingDay, waitingMs);
    taskActionInfo->Subtask++;
  } else
    taskActionInfo->Subtask++;
}

void simulateCommonTaskWithErrorRate(IsosTaskActionInfo* taskActionInfo, int endSubtaskNo, int errorMultiplierValue){
  static int runningValue = 0;
  if (taskActionInfo->Subtask == endSubtaskNo) {
    runningValue++;
    taskActionInfo->State = runningValue % errorMultiplierValue == 0 ? //error every multiply of errorMultiplierValue
      IsosTaskState_Failed : IsosTaskState_Success;
  } else
    taskActionInfo->Subtask++;
}

void simulateCommonTaskWithResourceUsage(unsigned char taskId, IsosTaskActionInfo* taskActionInfo, IsosResourceTaskType type){
  char result = 0, isResource = 0;
  IsosTaskState taskState = IsosTaskState_Undefined;
  //static int timeoutTrials; //may not be necessary
  switch(taskActionInfo->Subtask){
  case 0:
    result = Isos_ClaimResourceTask(taskId, type);
    if (result)
      taskActionInfo->Subtask++;
    break;
  case 1:
    taskState = Isos_GetResourceTaskState(type);
    if (taskState == IsosTaskState_Success){
      Isos_ReleaseResourceTask(type);
      taskActionInfo->Subtask = 2; //go to substate 2
      IsosDebugBasic_PrintSubtaskNote(1, 2, isResource);
    } else if (taskState == IsosTaskState_Failed) {
      Isos_ReleaseResourceTask(type);
      taskActionInfo->Subtask = 3; //go to substate 3
      IsosDebugBasic_PrintSubtaskNote(-1, 3, isResource);
    } else if (taskState == IsosTaskState_Timeout){
      Isos_ReleaseResourceTask(type);
      taskActionInfo->Subtask = 4; //go to substate 4
      IsosDebugBasic_PrintSubtaskNote(-2, 4, isResource);
    } else {
      IsosDebugBasic_PrintSubtaskNote(0, 1, isResource);
    }
    break;
  case 2: //successful case, do something
    taskActionInfo->State = IsosTaskState_Success;
    break;
  case 3: //failed case, do something
    taskActionInfo->State = IsosTaskState_Failed;
    break;
  case 4: //timeout case, do something
    taskActionInfo->State = IsosTaskState_Timeout; //internal timeout
    break;
  }
}

void simulateCommonTaskWithMultiResourcesUsage(unsigned char taskId, IsosTaskActionInfo* taskActionInfo, IsosResourceTaskType type1, IsosResourceTaskType type2){
  char result = 0, isResource = 0;
  IsosTaskState taskState = IsosTaskState_Undefined;
  //static int timeoutTrials; //may not be necessary
  switch(taskActionInfo->Subtask){
  case 0:
    result = Isos_ClaimResourceTask(taskId, type1);
    if (result)
      taskActionInfo->Subtask++;
    break;
  case 1:
    taskState = Isos_GetResourceTaskState(type1);
    if (taskState == IsosTaskState_Success){
      Isos_ReleaseResourceTask(type1);
      taskActionInfo->Subtask++; //go to next substate
      IsosDebugBasic_PrintSubtaskNote(1, 2, isResource);
    } else if (taskState == IsosTaskState_Failed) {
      Isos_ReleaseResourceTask(type1);
      taskActionInfo->Subtask = 5; //go to substate 5
      IsosDebugBasic_PrintSubtaskNote(-1, 5, isResource);
    } else if (taskState == IsosTaskState_Timeout){
      Isos_ReleaseResourceTask(type1);
      taskActionInfo->Subtask = 6; //go to substate 6
      IsosDebugBasic_PrintSubtaskNote(-2, 6, isResource);
    } else {
      IsosDebugBasic_PrintSubtaskNote(0, 1, isResource);
    }
    break;
  case 2: //successful case first stage, do something
    result = Isos_ClaimResourceTask(taskId, type2);
    if (result)
      taskActionInfo->Subtask++;
    break;
  case 3:
    taskState = Isos_GetResourceTaskState(type2);
    if (taskState == IsosTaskState_Success){
      Isos_ReleaseResourceTask(type2);
      taskActionInfo->Subtask++; //go to next substate
      IsosDebugBasic_PrintSubtaskNote(1, 4, isResource);
    } else if (taskState == IsosTaskState_Failed) {
      Isos_ReleaseResourceTask(type2);
      taskActionInfo->Subtask = 5; //go to substate 5
      IsosDebugBasic_PrintSubtaskNote(-1, 5, isResource);
    } else if (taskState == IsosTaskState_Timeout){
      Isos_ReleaseResourceTask(type2);
      taskActionInfo->Subtask = 6; //go to substate 6
      IsosDebugBasic_PrintSubtaskNote(-2, 6, isResource);
    } else {
      IsosDebugBasic_PrintSubtaskNote(0, 3, isResource);
    }
    break;
  case 4: //successful case, do something
    taskActionInfo->State = IsosTaskState_Success;
    break;
  case 5: //failed case, do something
    taskActionInfo->State = IsosTaskState_Failed;
    break;
  case 6: //timeout case, do something
    taskActionInfo->State = IsosTaskState_Timeout; //internal timeout
    break;
  }
}

void simulateGettingRxData(IsosResourceTaskType type, int rxSize){
  char result;
  IsosBuffer* buffer;
  short usedRxSize;
  unsigned char rxSimulatedDataBuffer[RX_DATA_BUFFER];
  buffer = Isos_GetResourceTaskBuffer(&result, type, 0); //by right result is always 1 for this simulation
  if (buffer->DataSize <= 0){
    usedRxSize = rxSize < 0 ? buffer->ExpectedDataSize : rxSize;
    for (int i = 0; i < usedRxSize; ++i)
      rxSimulatedDataBuffer[i] = rand();
    IsosBuffer_Puts(buffer, rxSimulatedDataBuffer, usedRxSize);
  }
}

void NonCyclicalTask1(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type1);
}

void NonCyclicalTask2(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithMultiResourcesUsage(taskId, taskActionInfo, IsosResourceTaskType_Type1, IsosResourceTaskType_Type2);
}

void NonCyclicalTask3(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type2);
}

void LooselyRepeatedTask1(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type2);
}

void LooselyRepeatedTask2(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type1);
}

void LooselyRepeatedTask3(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithSuspension(taskId, taskActionInfo, 3, IsosTaskState_Success, 1, 0, 50);
}

//Task which calls resource task with Rx only buffer
void LooselyRepeatedTask4(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  char result = 0, isResource = 0;
  IsosTaskState taskState = IsosTaskState_Undefined;
  IsosResourceTaskType type = IsosResourceTaskType_Type3;
  unsigned char rxDataBuffer[RX_DATA_BUFFER];
  switch(taskActionInfo->Subtask){
  case 0:
    //May directly use "Peek" here, do not need to do claim etc at all, unless the task is used by others
    result = Isos_ClaimResourceTask(taskId, type);
    if (result)
      taskActionInfo->Subtask++;
    break;
  case 1:
    taskState = Isos_GetResourceTaskState(type);
    if (taskState == IsosTaskState_Success){
      Isos_GetResourceTaskRx(type, rxDataBuffer, RX_DATA_BUFFER); //Do not peek here, but get it
      Isos_ReleaseResourceTask(type);
      taskActionInfo->Subtask = 2; //go to substate 2
      IsosDebugBasic_PrintSubtaskNote(1, 2, isResource);
    } else if (taskState == IsosTaskState_Failed) {
      Isos_ReleaseResourceTask(type);
      taskActionInfo->Subtask = 3; //go to substate 3
      IsosDebugBasic_PrintSubtaskNote(-1, 3, isResource);
    } else {
      IsosDebugBasic_PrintSubtaskNote(0, 1, isResource);
    }
    break;
  case 2: //successful case, do something
    taskActionInfo->State = IsosTaskState_Success;
    break;
  case 3: //failed case, do something else
    taskActionInfo->State = IsosTaskState_Failed;
    break;
  }
}

//Task which calls resource task with Tx only buffer
void LooselyRepeatedTask5(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  char result = 0, isResource = 0;
  IsosTaskState taskState = IsosTaskState_Undefined;
  IsosResourceTaskType type = IsosResourceTaskType_Type4;
  unsigned char txDataBuffer[TX_DATA_BUFFER];
  int i;
  switch(taskActionInfo->Subtask){
  case 0:
    result = Isos_ClaimResourceTask(taskId, type);
    if (result){
      for (i = 0; i < TX_DATA_BUFFER; ++i) //simulate data to be sent
        txDataBuffer[i] = rand();
      Isos_PrepareResourceTaskTx(type, txDataBuffer, TX_DATA_BUFFER);
      taskActionInfo->Subtask++;
    }
    break;
  case 1:
    taskState = Isos_GetResourceTaskState(type);
    if (taskState == IsosTaskState_Success){ //means everything in the buffer has been sent out
      Isos_ReleaseResourceTask(type);
      taskActionInfo->Subtask = 2; //go to substate 2
      IsosDebugBasic_PrintSubtaskNote(1, 2, isResource);
    } else if (taskState == IsosTaskState_Failed) {
      Isos_ReleaseResourceTask(type);
      taskActionInfo->Subtask = 3; //go to substate 3
      IsosDebugBasic_PrintSubtaskNote(-1, 3, isResource);
    } else {
      IsosDebugBasic_PrintSubtaskNote(0, 1, isResource);
    }
    break;
  case 2: //successful case, do something
    taskActionInfo->State = IsosTaskState_Success;
    break;
  case 3: //failed case, do something else
    taskActionInfo->State = IsosTaskState_Failed;
    break;
  }
}

void RepeatedTask1(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTask(taskActionInfo, 5, IsosTaskState_Success);
}

void RepeatedTask2(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTask(taskActionInfo, 4, IsosTaskState_Failed);
}

void RepeatedTask3(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  char result = 0, isResource = 0;
  IsosTaskState taskState = IsosTaskState_Undefined;
  IsosResourceTaskType type = IsosResourceTaskType_Type5;
  unsigned char txDataBuffer[TX_DATA_BUFFER];
  unsigned char rxDataBuffer[RX_DATA_BUFFER];
  int i;
  //static int timeoutTrials; //may not be necessary
  switch(taskActionInfo->Subtask){
  case 0:
    result = Isos_ClaimResourceTask(taskId, type);
    if (result){
      for (i = 0; i < TX_DATA_BUFFER; ++i) //simulate data to be sent
        txDataBuffer[i] = rand();
      Isos_PrepareResourceTaskTxWithSizeReturn(type, txDataBuffer, TX_DATA_BUFFER, RX_DATA_BUFFER);
      taskActionInfo->Subtask++;
    }
    break;
  case 1: //after pushing the Tx, this task waits for the response
    taskState = Isos_GetResourceTaskState(type);
    if (taskState == IsosTaskState_Success){
      Isos_GetResourceTaskRx(type, rxDataBuffer, RX_DATA_BUFFER); //Do not peek here, but get it
      Isos_ReleaseResourceTask(type);
      taskActionInfo->Subtask = 2; //go to substate 2
      IsosDebugBasic_PrintSubtaskNote(1, 2, isResource);
    } else if (taskState == IsosTaskState_Failed) {
      Isos_ReleaseResourceTask(type);
      taskActionInfo->Subtask = 3; //go to substate 3
      IsosDebugBasic_PrintSubtaskNote(-1, 3, isResource);
    } else {
      IsosDebugBasic_PrintSubtaskNote(0, 1, isResource);
    }
    break;
  case 2: //successful case, do something
    taskActionInfo->State = IsosTaskState_Success;
    break;
  case 3: //failed case, do something else
    taskActionInfo->State = IsosTaskState_Failed;
    break;
  }
}

void RepeatedTask4(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  char result = 0, isResource = 0;
  IsosTaskState taskState = IsosTaskState_Undefined;
  IsosResourceTaskType type = IsosResourceTaskType_Type6;
  unsigned char txDataBuffer[TX_DATA_BUFFER];
  unsigned char rxDataBuffer[RX_DATA_BUFFER];
  int i;
  //static int timeoutTrials; //may not be necessary
  switch(taskActionInfo->Subtask){
  case 0:
    result = Isos_ClaimResourceTask(taskId, type);
    if (result){
      for (i = 0; i < TX_DATA_BUFFER; ++i) //simulate data to be sent
        txDataBuffer[i] = rand();
      Isos_PrepareResourceTaskTxWithTimeReturn(type, txDataBuffer, TX_DATA_BUFFER, 0, 30);
      taskActionInfo->Subtask++;
    }
    break;
  case 1: //after pushing the Tx, this task waits for the response
    taskState = Isos_GetResourceTaskState(type);
    if (taskState == IsosTaskState_Success){
      Isos_GetResourceTaskRx(type, rxDataBuffer, RX_DATA_BUFFER); //Do not peek here, but get it
      Isos_ReleaseResourceTask(type);
      taskActionInfo->Subtask = 2; //go to substate 2
      IsosDebugBasic_PrintSubtaskNote(1, 2, isResource);
    } else if (taskState == IsosTaskState_Failed) {
      Isos_ReleaseResourceTask(type);
      taskActionInfo->Subtask = 3; //go to substate 3
      IsosDebugBasic_PrintSubtaskNote(-1, 3, isResource);
    } else {
      IsosDebugBasic_PrintSubtaskNote(0, 1, isResource);
    }
    break;
  case 2: //successful case, do something
    taskActionInfo->State = IsosTaskState_Success;
    break;
  case 3: //failed case, do something else
    taskActionInfo->State = IsosTaskState_Failed;
    break;
  }
}

void RepeatedTask5(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type7);
}

void PeriodicTask1(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type1);
}

void PeriodicTask2(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type2);
}

void PeriodicTask3(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithMultiResourcesUsage(taskId, taskActionInfo, IsosResourceTaskType_Type1, IsosResourceTaskType_Type2);
}

void PeriodicTask4(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithMultiResourcesUsage(taskId, taskActionInfo, IsosResourceTaskType_Type2, IsosResourceTaskType_Type1);
}

//A stuck task
void PeriodicTask5(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  char result = 0;
  IsosResourceTaskType type = IsosResourceTaskType_Type7;
  switch(taskActionInfo->Subtask){
  case 0:
    result = Isos_ClaimResourceTask(taskId, type);
    if (result)
      taskActionInfo->Subtask++;
    break;
  case 1:
    IsosDebugBasic_PrintStuckTask(taskId);
    break;
  }
}

void PeriodicTask6(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type8);
}

void ResourceTask1(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTask(taskActionInfo, 3, IsosTaskState_Success);
}

//Resource task with failure rate
void ResourceTask2(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTaskWithErrorRate(taskActionInfo, 3, 3);
}

//Resource with Rx only buffer (may not be needed)
void ResourceTask3(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  static int runningNo = 0; //to simulate occasional retrieval of data from the Rx buffer
  int i;
  char result = 0, hasRx = 0;
  unsigned char rxSimulatedDataBuffer[RX_DATA_BUFFER];
  unsigned char rxDataBuffer[RX_DATA_BUFFER];
  IsosResourceTaskType type = IsosResourceTaskType_Type3;
  IsosBuffer* buffer;
  if (0 == taskActionInfo->Subtask)
    ++runningNo;
  hasRx = 0 == runningNo % RX_RETRIEVAL_NO; //occasionally, this task will have Rx
  if (hasRx) { //simulate putting the Rx
    buffer = Isos_GetResourceTaskBuffer(&result, type, 0); //by right result is always 1 for this simulation
    for (i = 0; i < RX_DATA_BUFFER; ++i)
      rxSimulatedDataBuffer[i] = rand();
    IsosBuffer_Puts(buffer, rxSimulatedDataBuffer, RX_DATA_BUFFER);
  }
  switch(taskActionInfo->Subtask){
  case 0:
    result = Isos_PeekResourceTaskRx(type, rxDataBuffer, RX_DATA_BUFFER); //Peek whatever is in the buffer
    taskActionInfo->State = result ? IsosTaskState_Success : IsosTaskState_Failed; //if has sufficient data then successful, otherwise, failed
    break;
  }
}

//Resource task Tx only buffer
void ResourceTask4(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  static int runningNo = 0; //to simulate if the sending has been completed
  char result = 0, txSent = 0, isResource = 1;
  short dataSize = 0;
  unsigned char txDataBuffer[TX_DATA_BUFFER];
  IsosResourceTaskType type = IsosResourceTaskType_Type4;
  IsosBuffer* buffer;
  ++runningNo;
  txSent = 3 == runningNo % TX_TRANSMITTED_NO;
  buffer = Isos_GetResourceTaskBuffer(&result, type, 1); //by right result is always 1 for this simulation
  if (txSent) //simulate complete transmission
    dataSize = IsosBuffer_Gets(buffer, txDataBuffer, -1); //removes the data from the Tx buffer
  switch(taskActionInfo->Subtask){
  case 0:
    taskActionInfo->Subtask++; //assuming the trigger of the sending occurs here
    IsosDebugBasic_PrintResourceTaskBufferData(type, buffer, 4);
    break;
  case 1:
    dataSize = Isos_GetResourceTaskTxDataSize(type); //assuming the data checking occurs here
    if (dataSize > 0){ //not completed, just wait here
      IsosDebugBasic_PrintSubtaskNote(0, 1, isResource);
      //probably want to put some sort of timeout here
    } else { //completed transmission
      taskActionInfo->State = IsosTaskState_Success;
      runningNo = 0;
      IsosDebugBasic_PrintSubtaskNote(1, -1, isResource);
    }
    break;
  }
}

//Resource task with Tx & Rx buffers waited by size
void ResourceTask5(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  static int runningNo = 0; //to simulate if the sending has been completed
  char result = 0, txSent = 0, hasRx = 0, isResource = 1;
  short dataSize = 0;
  unsigned char txDataBuffer[TX_DATA_BUFFER];
  IsosResourceTaskType type = IsosResourceTaskType_Type5;
  IsosBuffer* buffer;
  ++runningNo;
  txSent = 3 == runningNo % TX_TRANSMITTED_NO;
  hasRx = 0 == runningNo % RX_RETRIEVAL_NO; //occasionally, this task will have Rx
  if (hasRx && taskActionInfo->Subtask >= 2) //simulate putting the Rx
    simulateGettingRxData(type, -1);
  buffer = Isos_GetResourceTaskBuffer(&result, type, 1); //by right result is always 1 for this simulation
  if (txSent) //simulate complete transmission
    dataSize = IsosBuffer_Gets(buffer, txDataBuffer, -1); //removes the data from the Tx buffer
  switch(taskActionInfo->Subtask){
  case 0:
    taskActionInfo->Subtask++; //assuming the trigger of the sending occurs here
    IsosDebugBasic_PrintResourceTaskBufferData(type, buffer, 4);
    break;
  case 1:
    dataSize = Isos_GetResourceTaskTxDataSize(type); //assuming the data checking occurs here
    if (dataSize > 0){ //not completed, just wait here
      IsosDebugBasic_PrintSubtaskNote(0, 1, isResource);
      //probably want to put some sort of timeout here
    } else { //completed transmission
      runningNo = 0;
      IsosDebugBasic_PrintSubtaskNote(1, 2, isResource);
      taskActionInfo->Subtask++;
    }
    break;
  case 2:
    result = Isos_ResourceTaskHasExpectedDataSize(type, 0); //Check if the buffer has the expected data size
    if (result) {
      runningNo = 0;
      taskActionInfo->State = IsosTaskState_Success; //if has sufficient data then successful, otherwise, failed
      IsosDebugBasic_PrintSubtaskNote(1, -1, isResource);
    } else {
      IsosDebugBasic_PrintSubtaskNote(0, 2, isResource);
    }
    break;
  }
}

//Resource task with Tx & Rx buffers waited by time
void ResourceTask6(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  static int runningNo = 0, shouldSuccess = 0; //to simulate if the sending has been completed. failed first, then success
  char result = 0, txSent = 0, isResource = 1;
  short dataSize = 0;
  unsigned char txDataBuffer[TX_DATA_BUFFER];
  unsigned char rxDataBuffer[RX_DATA_BUFFER];
  IsosResourceTaskType type = IsosResourceTaskType_Type6;
  IsosBuffer* buffer;
  ++runningNo;
  txSent = 3 == runningNo % TX_TRANSMITTED_NO;
  if (shouldSuccess % 2 == 1 && taskActionInfo->Subtask >= 2)
    simulateGettingRxData(type, RX_DATA_BUFFER);
  buffer = Isos_GetResourceTaskBuffer(&result, type, 1); //by right result is always 1 for this simulation
  if (txSent) //simulate complete transmission
    dataSize = IsosBuffer_Gets(buffer, txDataBuffer, -1); //removes the data from the Tx buffer
  switch(taskActionInfo->Subtask){
  case 0:
    taskActionInfo->Subtask++; //assuming the trigger of the sending occurs here
    IsosDebugBasic_PrintResourceTaskBufferData(type, buffer, 4);
    break;
  case 1:
    dataSize = Isos_GetResourceTaskTxDataSize(type); //assuming the data checking occurs here
    if (dataSize > 0){ //not completed, just wait here
      IsosDebugBasic_PrintSubtaskNote(0, 1, isResource);
      //probably want to put some sort of timeout here
    } else { //completed transmission
      runningNo = 0;
      Isos_WaitFromSuspensionTime(taskId); //sets this task to wait based on the previously given suspension time
      IsosDebugBasic_PrintSubtaskNote(1, 2, isResource);
      taskActionInfo->Subtask++;
    }
    break;
  case 2: //whatever happens, this is done! because the time is over
    result = Isos_PeekResourceTaskRx(type, rxDataBuffer, -1);
    if (result) { //has data
      taskActionInfo->State = IsosTaskState_Success; //has data
      IsosDebugBasic_PrintSubtaskNote(1, -1, isResource);
    } else { //no data retrieved
      shouldSuccess++;
      taskActionInfo->State = IsosTaskState_Failed; //no data
      IsosDebugBasic_PrintSubtaskNote(-1, -1, isResource);
    }
    runningNo = 0;
    break;
  }
}

void ResourceTask7(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  simulateCommonTask(taskActionInfo, 4, IsosTaskState_Success);
}

void ResourceTask8(unsigned char taskId, IsosTaskActionInfo* taskActionInfo){
  static int runningNo = 0;
  char timeToStuck = runningNo % 3 == 2;
  if (0 == taskActionInfo->Subtask)
    ++runningNo;
  switch(taskActionInfo->Subtask){
  case 0: taskActionInfo->Subtask++; break;
  case 1:
    if (timeToStuck){
      IsosDebugBasic_PrintStuckTask(taskId);
    } else
      taskActionInfo->Subtask++;
    break;
  case 2: taskActionInfo->State = IsosTaskState_Success; break;
  }
}

//  IsosBuffer txBuffer, rxBuffer;
//  unsigned char txBufferData[200];
//  unsigned char rxBufferData[200];
//  unsigned char data[25];
//  IsosBuffer_Init(&txBuffer, txBufferData, 200);
//  IsosBuffer_Init(&rxBuffer, rxBufferData, 200);
//  IsosUtility_FillBuffer(25, data, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25);
//  IsosBuffer_Puts(&txBuffer, data, 25);
//  IsosDebugBasic_PrintBufferData(&txBuffer);
//  return 0;
//

