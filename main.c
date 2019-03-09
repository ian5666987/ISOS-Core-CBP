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
#include "main.h"
#include "isos_debug_basic.h"

int main()
{
    struct IsosClock mainClock;
    char val;
    Isos_Init(); //the first to be called before registering any task
    registerTasks();

    while (1){ //While (1) to be used internally in the Isos_Run() function for actual implementation, see TODO note on Isos_Run() function
        mainClock = Isos_GetClock();
        if (mainClock.Ms > 0 && mainClock.Ms % 2000 == 0){
            printf("Press any key but [x+Enter] to continue...\n");
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
    Isos_RegisterRunOnceTask(1, 0, 1200, MAX_PRIORITY-2, RunOnceTask1); //suppose this is antenna deployment
    Isos_RegisterRunOnceTask(1, 0, 1800, MAX_PRIORITY-3, RunOnceTask2); //suppose this is solar panel deployment
    Isos_RegisterRunOnceTask(1, 0, 370, 3, RunOnceTask3); //purposely made to simulate interesting clash on 370ms time stamp
    Isos_RegisterLooselyRepeatedTask(1, 0, 100, 0, LooselyRepeatedTask1);
    Isos_RegisterLooselyRepeatedTask(1, 0, 150, 1, LooselyRepeatedTask2);
    Isos_RegisterLooselyRepeatedTask(1, 0, 400, 2, LooselyRepeatedTask3); //Added to test task waiting case
    Isos_RegisterRepeatedTask(1, 0, 200, 4, RepeatedTask1);
    Isos_RegisterRepeatedTask(1, 0, 300, 5, RepeatedTask2);
    Isos_RegisterPeriodicTask(1, 0, 200, 6, PeriodicTask1);
    Isos_RegisterPeriodicTask(1, 0, 250, 7, PeriodicTask2);
    Isos_RegisterPeriodicTask(1, 0, 300, 8, PeriodicTask3);
    Isos_RegisterPeriodicTask(1, 0, 350, 9, PeriodicTask4);

    //Better put all resource task priorities higher than all other tasks
    Isos_RegisterResourceTask(IsosResourceTaskType_Type1, MAX_PRIORITY-1, ResourceTask1);
    Isos_RegisterResourceTask(IsosResourceTaskType_Type2, MAX_PRIORITY, ResourceTask2);
}

void simulateCommonTask(struct IsosTaskActionInfo* taskActionInfo, int endSubtaskNo, enum IsosTaskState endState){
    if (taskActionInfo->Subtask == endSubtaskNo)
        taskActionInfo->State = endState;
    else
        taskActionInfo->Subtask++;
}

void simulateCommonTaskWithSuspension(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo, int endSubtaskNo, enum IsosTaskState endState,
                                      int waitingSubtaskNo, short waitingDay, long waitingMs){
    if (taskActionInfo->Subtask == endSubtaskNo)
        taskActionInfo->State = endState;
    else if(taskActionInfo->Subtask == waitingSubtaskNo){
        Isos_Wait(taskId, waitingDay, waitingMs);
        taskActionInfo->Subtask++;
    } else
        taskActionInfo->Subtask++;
}

void simulateCommonTaskWithErrorRate(struct IsosTaskActionInfo* taskActionInfo, int endSubtaskNo, int errorMultiplierValue){
    static int runningValue = 0;
    if (taskActionInfo->Subtask == endSubtaskNo) {
        runningValue++;
        taskActionInfo->State = runningValue % errorMultiplierValue == 0 ? //error every multiply of errorMultiplierValue
            IsosTaskState_Failed : IsosTaskState_Success;
    } else
        taskActionInfo->Subtask++;
}

void simulateCommonTaskWithResourceUsage(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo, enum IsosResourceTaskType type){
    char result = 0;
    enum IsosTaskState taskState = IsosTaskState_Undefined;
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
            Isos_ReleaseResourceTask(type, taskActionInfo);
            taskActionInfo->Subtask = 2; //go to substate 2
            IsosDebugBasic_PrintSubtaskNote(1, 2);
        } else if (taskState == IsosTaskState_Failed) {
            Isos_ReleaseResourceTask(type, taskActionInfo);
            taskActionInfo->Subtask = 3; //go to substate 3
            IsosDebugBasic_PrintSubtaskNote(-1, 3);
        } else {
            IsosDebugBasic_PrintSubtaskNote(0, 1);
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

void simulateCommonTaskWithMultiResourcesUsage(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo, enum IsosResourceTaskType type1, enum IsosResourceTaskType type2){
    char result = 0;
    enum IsosTaskState taskState = IsosTaskState_Undefined;
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
            Isos_ReleaseResourceTask(type1, taskActionInfo);
            taskActionInfo->Subtask++; //go to next substate
            IsosDebugBasic_PrintSubtaskNote(1, 2);
        } else if (taskState == IsosTaskState_Failed) {
            Isos_ReleaseResourceTask(type1, taskActionInfo);
            taskActionInfo->Subtask = 5; //go to substate 5
            IsosDebugBasic_PrintSubtaskNote(-1, 5);
        } else {
            IsosDebugBasic_PrintSubtaskNote(0, 1);
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
            Isos_ReleaseResourceTask(type2, taskActionInfo);
            taskActionInfo->Subtask++; //go to next substate
            IsosDebugBasic_PrintSubtaskNote(1, 4);
        } else if (taskState == IsosTaskState_Failed) {
            Isos_ReleaseResourceTask(type2, taskActionInfo);
            taskActionInfo->Subtask = 5; //go to substate 5
            IsosDebugBasic_PrintSubtaskNote(-1, 5);
        } else {
            IsosDebugBasic_PrintSubtaskNote(0, 3);
        }
        break;
    case 4: //successful case, do something
        taskActionInfo->State = IsosTaskState_Success;
        break;
    case 5: //failed case, do something else
        taskActionInfo->State = IsosTaskState_Failed;
        break;
    }
}

void RunOnceTask1(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type1);
}

void RunOnceTask2(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTaskWithMultiResourcesUsage(taskId, taskActionInfo, IsosResourceTaskType_Type1, IsosResourceTaskType_Type2);
}

void RunOnceTask3(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type2);
}

void LooselyRepeatedTask1(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type2);
}

void LooselyRepeatedTask2(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type1);
}

void LooselyRepeatedTask3(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTaskWithSuspension(taskId, taskActionInfo, 3, IsosTaskState_Success, 1, 0, 50);
}

void RepeatedTask1(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTask(taskActionInfo, 5, IsosTaskState_Success);
}

void RepeatedTask2(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTask(taskActionInfo, 4, IsosTaskState_Failed);
}

void PeriodicTask1(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type1);
}

void PeriodicTask2(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTaskWithResourceUsage(taskId, taskActionInfo, IsosResourceTaskType_Type2);
}

void PeriodicTask3(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTaskWithMultiResourcesUsage(taskId, taskActionInfo, IsosResourceTaskType_Type1, IsosResourceTaskType_Type2);
}

void PeriodicTask4(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTaskWithMultiResourcesUsage(taskId, taskActionInfo, IsosResourceTaskType_Type2, IsosResourceTaskType_Type1);
}

void ResourceTask1(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTask(taskActionInfo, 3, IsosTaskState_Success);
}

void ResourceTask2(unsigned char taskId, struct IsosTaskActionInfo* taskActionInfo){
    simulateCommonTaskWithErrorRate(taskActionInfo, 3, 3);
}


