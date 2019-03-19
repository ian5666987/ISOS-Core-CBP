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

#include <stdio.h>
#include <string.h>
#include "isos.h"
#include "isos_quicksort.h"

#define BASIC_DEBUG 1

#if BASIC_DEBUG
#include "isos_debug_basic.h"
#endif // BASIC_DEBUG

//All the variable belows shouldn't be used anywhere else, therefore statics, thus cannot be "extern"-ed
//The main clock used for the whole project
static IsosClock IsosMainClock;
static IsosClock LastSchedulerRun;
static IsosClock LastSchedulerFinished; //probably good for debugging
static IsosClock SchedulerPeriod;
static IsosTask IsosTaskList[MAX_TASK_SIZE];
static IsosDueTask IsosDueTaskList[MAX_TASK_SIZE]; //an array, as required by the QuickSort
static unsigned char IsosResourceTaskList[RESOURCE_SIZE]; //to store the mapping of the resource type to the task Id
static short IsosDueTaskSize = 0;
static short IsosTaskSize = 0;
static char IsosRequestSorting = 0; //a flag to request sorting in the scheduler
static IsosResourceTaskType LastClaimedResourceTask = IsosResourceTaskType_Unspecified; //Used as a flag if there is any resource task that has just been claimed
static IsosResourceTaskType LastReleasedResourceTask = IsosResourceTaskType_Unspecified; //Used as a flag if there is any resource task that has just been released

//Since it is so common to have these variables, we my as well initialize them outside of functions
// to save some initialization of variables across different functions
static IsosTask* genericTask;
static IsosTaskInfo* genericTaskInfo;
static IsosTaskActionInfo* genericTaskActionInfo;

void Isos_Init(){ //just to be safe, zeroes everything out
  memset(IsosTaskList, 0, sizeof(IsosTaskList));
  memset(IsosDueTaskList, 0, sizeof(IsosDueTaskList));
  memset(IsosResourceTaskList, 0, sizeof(IsosResourceTaskList));
  memset(&IsosMainClock, 0, sizeof(IsosMainClock));
  memset(&LastSchedulerRun, 0, sizeof(LastSchedulerRun));
  memset(&LastSchedulerFinished, 0, sizeof(LastSchedulerFinished));
  SchedulerPeriod.Day = CLOCK_PERIOD_DAY;
  SchedulerPeriod.Ms = CLOCK_PERIOD_MS;
  IsosDueTaskSize = 0;
  IsosTaskSize = 0;
  IsosRequestSorting = 0;
}

void Isos_initClockToNow(IsosTaskInfo* taskInfo){
  IsosClock clock;
  clock = Isos_GetClock();
  taskInfo->LastDueReported = clock;
  taskInfo->LastExecuted = clock;
  taskInfo->LastFinished = clock;
  taskInfo->SuspensionDue = clock;
}

void Isos_queueOnDueHandled(IsosTaskInfo* taskInfo, IsosClock* clock){
  taskInfo->ForcedDue = 0; //whatever happen, reset the force run now flag here
  taskInfo->IsDueReported = 1; //report that this task has been queued
  taskInfo->LastDueReported = *clock; //record this due reported time
  IsosRequestSorting = 1; //raise flag to force the sorting next time the scheduler is run (due to the newly claimed resource task)
}

void Isos_queueOnDue(IsosTaskInfo* taskInfo, IsosClock clock){
  IsosDueTaskList[IsosDueTaskSize].TaskId = taskInfo->Id;
  IsosDueTaskList[IsosDueTaskSize].Priority = taskInfo->Priority;
  IsosDueTaskSize++;
  Isos_queueOnDueHandled(taskInfo, &clock);
}

void Isos_removeDueTaskByIndex(short dueTaskIndex){
  if (dueTaskIndex >= IsosDueTaskSize)
    return; //cannot exceed the boundary
  if (dueTaskIndex < IsosDueTaskSize - 1) //NOT the last element
    // example: size = 10, i = 6, then 6-9 (4 items) is replaced by 7-9 (3 items)
    memcpy(&IsosDueTaskList[dueTaskIndex], &IsosDueTaskList[dueTaskIndex + 1], sizeof(IsosDueTask) * (IsosDueTaskSize - (dueTaskIndex + 1)));
  IsosDueTaskSize--; //Reduce the size. If this is the last element no memcpy needed
}

void Isos_dequeueFromDue(unsigned char taskId){
  short i;
  if (IsosDueTaskSize == 0) //cannot dequeue anything
    return;
  if (IsosDueTaskSize == 1){ //immediately emptifies the current list
    memset(IsosDueTaskList, 0, sizeof(IsosDueTaskList));
    IsosDueTaskSize = 0; //return the due task size back to zero
    return;
  }
  //In all likelihood, the reverse order search would be faster
  for (i = IsosDueTaskSize - 1; i >= 0; --i) //going through the list
    if (IsosDueTaskList[i].TaskId == taskId){
      Isos_removeDueTaskByIndex(i);
      break;
    }
}

void Isos_insertTaskOnDue(short currentRunningTaskIndex, IsosTaskInfo* taskInfo, IsosClock clock){
  short prevIndex, tailTaskNo;
  if (currentRunningTaskIndex < 0) //does not make sense
    return;
  if (IsosDueTaskSize <= 0){
    Isos_queueOnDue(taskInfo, clock); //just queue normally - this should be an impossible case though
    return;
  }
  prevIndex = IsosDueTaskSize - 1;
  if (currentRunningTaskIndex == prevIndex){ //simply shift the current one to the next position
    IsosDueTaskList[IsosDueTaskSize].TaskId = IsosDueTaskList[prevIndex].TaskId;
    IsosDueTaskList[IsosDueTaskSize].Priority = IsosDueTaskList[prevIndex].Priority;
    IsosDueTaskList[prevIndex].TaskId = taskInfo->Id;
    IsosDueTaskList[prevIndex].Priority = taskInfo->Priority;
  } else { //All the memcpy is used for all non trivial cases
    tailTaskNo = IsosDueTaskSize - currentRunningTaskIndex; //no of task after the currently running one
    memcpy(&IsosDueTaskList[currentRunningTaskIndex + 1], &IsosDueTaskList[currentRunningTaskIndex], sizeof(IsosDueTask) * tailTaskNo);
    IsosDueTaskList[currentRunningTaskIndex].TaskId = taskInfo->Id;
    IsosDueTaskList[currentRunningTaskIndex].Priority = taskInfo->Priority;
  }
  IsosDueTaskSize++;
  Isos_queueOnDueHandled(taskInfo, &clock);
}

void Isos_prepareToDueTask(IsosTaskInfo* taskInfo, unsigned char priority, char withReset){
  if (taskInfo->ActionInfo.State == IsosTaskState_Suspended) //if the task has been suspended before, then the state will need to be changed to running first
    taskInfo->ActionInfo.State = IsosTaskState_Running; //otherwise, don't change the state, just ask to be re-run will do
  taskInfo->Priority = priority; //change the priority of the task first
  taskInfo->ActionInfo.Enabled = 1; //whatever happen, enable it
  if (withReset) {
    if (taskInfo->IsDueReported) //takes away the task from the due list before reseting the state
      Isos_dequeueFromDue(taskInfo->Id);
    IsosTask_ResetState(taskInfo); //reset the state if required
  }
  if (taskInfo->IsDueReported) //already in the task list to run, just continue, no need to re-run
    IsosRequestSorting = 1; //no need to report to run the task again, just need to do re-sorting in case priority changes
}

void Isos_prepareGenericTaskPointersById(unsigned char taskId){
  genericTask = &IsosTaskList[taskId];
  genericTaskInfo = &genericTask->Info;
  genericTaskActionInfo = &genericTaskInfo->ActionInfo;
}

void Isos_commonPrepareDueNonCyclicalTask(IsosTaskInfo* taskInfo, unsigned char priority, char withReset, IsosClock clock){
  Isos_prepareToDueTask(taskInfo, priority, withReset);
  if (taskInfo->IsDueReported) //already in the task list to run, just continue, no need to re-run
    return;
  taskInfo->TimeInfo.ExecutionDue.Day = clock.Day;
  taskInfo->TimeInfo.ExecutionDue.Ms = clock.Ms;
}

void Isos_scheduler(){
  //the main loop to check every task
  IsosTaskInfo* taskInfo;
  IsosClock mainClock;
  short i = 0;
  mainClock = Isos_GetClock(); //freezes the clock when checking the due
  for (i = 0; i < IsosTaskSize; ++i){
    taskInfo = &IsosTaskList[i].Info;
    if (taskInfo->IsDueReported || !taskInfo->ActionInfo.Enabled) //no need to process reported or disabled task further
      continue;
    if (!taskInfo->ForcedDue && //if the task is not being forced run and in the suspended state, do not run, any other state is valid to re-run the task
      (taskInfo->ActionInfo.State == IsosTaskState_Suspended)) //naturally, task should not be in suspended state on the time it is to be due
      continue; //only if the due is not reported then we need to report, otherwise, just leave it
    if (taskInfo->ForcedDue || IsosTask_IsDue(&mainClock, taskInfo)) //internally is due does not check anything except the time
      Isos_queueOnDue(taskInfo, mainClock); //queue the tasks
  }

  //only if the task has changes or request sort flag is raise then we *may* need to rearrange the due tasks
  if (IsosRequestSorting){
    IsosRequestSorting = 0; //reset the request sorting flag
    if (IsosDueTaskSize > 1) //only if due task size > 1 then this actually really needs re-sorting
      Isos_QuickSortAsc(IsosDueTaskList, 0, IsosDueTaskSize-1); //use ASC so that it is easier to remove the last task
  }
}

void Isos_execute(IsosTask* task){
  IsosClock clock;
  Isos_prepareGenericTaskPointersById(task->Info.Id);
  //Disabled task, suspended, or not reported task cannot be run,
  if (!genericTaskActionInfo->Enabled || !genericTaskInfo->IsDueReported)
    return;

  if (genericTaskActionInfo->State == IsosTaskState_Suspended){ //if the task is suspended, check if the due is already coming
    clock = Isos_GetClock();
    clock = IsosClock_Minus(&clock, &genericTaskInfo->SuspensionDue);
    if (IsosClock_GetDirection(&clock) < 0){ //not due yet
      #if BASIC_DEBUG
      IsosDebugBasic_PrintTaskInfo(genericTaskInfo);
      #endif // BASIC_DEBUG
      return;
    } else {
      genericTaskActionInfo->State = IsosTaskState_Running; //change the task's state back to running
      #if BASIC_DEBUG
      IsosDebugBasic_PrintEndWaitingNote(genericTaskInfo);
      #endif // BASIC_DEBUG
    }
  }

  //The previous state can be initialized, failed, or successful, it does not matter! Run the task as long as it is not running but dued
  if (genericTaskActionInfo->State != IsosTaskState_Running){ //first time, task to be re-run: Initial, Failed or Success
    genericTaskActionInfo->State = IsosTaskState_Running; //as long as it is executed, force the state to be running
    genericTaskInfo->LastExecuted = Isos_GetClock(); //task to be executed for the first time
  }
  #if BASIC_DEBUG
  IsosDebugBasic_PrintTaskInfo(genericTaskInfo);
  #endif // BASIC_DEBUG
  task->Action(genericTaskInfo->Id, genericTaskActionInfo); //something will happen inside, the task state will change here
  if (genericTaskActionInfo->State == IsosTaskState_Failed || genericTaskActionInfo->State == IsosTaskState_Success){ //means the task has been executed
    #if BASIC_DEBUG
    IsosDebugBasic_PrintTaskInfo(genericTaskInfo);
    #endif // BASIC_DEBUG
    genericTaskActionInfo->Subtask = 0; //Run is completed, reset the subtask
    genericTaskInfo->IsDueReported = 0; //now the flag is set down so that we can know that this can be reported again
    genericTaskInfo->ForcedDue = 0; //whatever happen, reset the force due now flag here
    genericTaskInfo->LastFinished = Isos_GetClock(); //update the last time task is finished executed
    if (genericTaskInfo->Type == IsosTaskType_Resource || genericTaskInfo->Type == IsosTaskType_RunOnce)
      genericTaskActionInfo->Enabled = 0; //resource task and runOnce tasks are always disabled after completed to prevent them to be re-run
    Isos_dequeueFromDue(genericTaskInfo->Id);
    //The task result is to be treated by other tasks who wait for the run task results
  }
}

//Find the due task index within the limit of the searched elements (not going through the whole list)
short Isos_findDueTaskIndex(unsigned char taskId, unsigned char inclusiveSearchLimit){
  short i;
  for (i = IsosDueTaskSize - 1; i >= inclusiveSearchLimit; --i) //searching from the end most to the search limit index
    if (IsosDueTaskList[i].TaskId == taskId)
      return i;
  return -1; //cannot be found
}

char Isos_registerTask(IsosTaskType type, IsosResourceTaskType resourceType, char enabled,
                       short timeInfoDay, long timeInfoMs, unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*)){
  IsosClock timeInfo;
  IsosTask task;
  if (IsosTaskSize >= MAX_TASK_SIZE)
    return 0; //cannot register a task anymore
  IsosTask_ResetState(&task.Info);
  Isos_initClockToNow(&task.Info);
  task.Info.Type = type;
  task.Info.ActionInfo.Enabled = enabled;
  timeInfo = IsosClock_Create(timeInfoDay, timeInfoMs);
  task.Info.TimeInfo.Any = timeInfo; //"Any" will adjust internally
  task.Info.Priority = priority;
  task.Info.Id = IsosTaskSize; //the Id follows whatever is the current task set size
  task.Action = taskAction;
  if (type == IsosTaskType_Resource && resourceType >= 0 && resourceType < RESOURCE_SIZE)
    IsosResourceTaskList[resourceType] = task.Info.Id; //resource type Id must be specially mapped to the resource task list
  IsosTaskList[IsosTaskSize] = task;
  IsosTaskSize++;
  return 1; //successful
}

char Isos_RegisterRunOnceTask(char enabled, short executionDueDay, long executionDueMs, unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*)){
  return Isos_registerTask(IsosTaskType_RunOnce, IsosResourceTaskType_Unspecified, enabled, executionDueDay, executionDueMs, priority, taskAction);
}

char Isos_RegisterResourceTask(IsosResourceTaskType resourceType, unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*)){
  //resource task always started disabled, only to be enabled when needed to be run
  return Isos_registerTask(IsosTaskType_Resource, resourceType, 0, 0, 0, priority, taskAction);
}

char Isos_RegisterLooselyRepeatedTask(char enabled, short periodDay, long periodMs, unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*)){
  return Isos_registerTask(IsosTaskType_LooselyRepeated, IsosResourceTaskType_Unspecified, enabled, periodDay, periodMs, priority, taskAction);
}

char Isos_RegisterRepeatedTask(char enabled, short periodDay, long periodMs, unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*)){
  return Isos_registerTask(IsosTaskType_Repeated, IsosResourceTaskType_Unspecified, enabled, periodDay, periodMs, priority, taskAction);
}

char Isos_RegisterPeriodicTask(char enabled, short periodDay, long periodMs, unsigned char priority, void (*taskAction)(unsigned char, IsosTaskActionInfo*)){
  return Isos_registerTask(IsosTaskType_Periodic, IsosResourceTaskType_Unspecified, enabled, periodDay, periodMs, priority, taskAction);
}

//Function to schedule a RunOnce task to be run sometime in the future with specified priority
void Isos_ScheduleRunOnceTask(IsosTaskInfo* taskInfo, unsigned char priority, char withReset, short executionDueDay, long executionDueMs){
  IsosClock clock;
  if (taskInfo->Type != IsosTaskType_RunOnce)
    return; //rejects to run cyclical task type
  clock.Day = executionDueDay;
  clock.Ms = executionDueMs;
  Isos_commonPrepareDueNonCyclicalTask(taskInfo, priority, withReset, clock);
}

//Function to hasten the due of a non-cyclical (RunOnce or Resource) task to be run immediately with specified priority
void Isos_DueNonCyclicalTaskNow(IsosTaskInfo* taskInfo, unsigned char priority, char withReset){
  if (taskInfo->Type != IsosTaskType_RunOnce && taskInfo->Type != IsosTaskType_Resource)
    return; //rejects to run cyclical task type
  Isos_commonPrepareDueNonCyclicalTask(taskInfo, priority, withReset, Isos_GetClock());
}

//To force to due any task right now with specified priority, regardless of the due, using special flag
//Use this only for special case - direct intervention for the execution
void Isos_DueTaskNow(IsosTaskInfo* taskInfo, unsigned char priority, char withReset){
  Isos_prepareToDueTask(taskInfo, priority, withReset);
  if (taskInfo->IsDueReported) //already in the task list to run, just continue, no need to re-run
    return;
  taskInfo->ForcedDue = 1; //by passing isDue checking so that it will be due to run though the due has not come yet
}

IsosClock Isos_GetClock(){ return IsosMainClock; } //create a copy of main clock for use

void Isos_handleLastReleasedResource(short* currentDueIndex){
  //Because there are many variables initialized here, static could probably help to save some initialization time
  static IsosTask* nextClaimerTask;
  static unsigned char nextClaimerId;
  static short nextClaimerDueTaskIndex;
  if (LastReleasedResourceTask == IsosResourceTaskType_Unspecified)
    return;
  // a resource task has just been released
  Isos_prepareGenericTaskPointersById(IsosResourceTaskList[LastReleasedResourceTask]);
  LastReleasedResourceTask = IsosResourceTaskType_Unspecified;
  if (!genericTaskActionInfo->Flags[0]) //no flag raised, then do not need to continue
    return;
  //Otherwise, there is a next claimer for this task
  nextClaimerId = genericTaskActionInfo->Flags[1];
  IsosTask_ClearActionFlags(genericTaskActionInfo); //clear the flags for the next cycle
  //NOTE: Since the current due task could have been removed, the next claimer due index can as well be the same as current index
  nextClaimerDueTaskIndex = Isos_findDueTaskIndex(nextClaimerId, *currentDueIndex); //find the next claimer on the list of already executed due tasks this turn
  if (nextClaimerDueTaskIndex < *currentDueIndex) //means task with higher priority not found
    return; //just return immediately
  //Otherwise, the next claimer is due before (that is, having higher index than) the current task, but is "overlooked" by this cycle!
  //NOTE: nextClaimerDueTaskIndex == i is a very special case, means the due is in the current index due to the previous due is done
  if (nextClaimerDueTaskIndex > *currentDueIndex){ //means it is not directly next to the just executed task
    nextClaimerTask = &IsosTaskList[nextClaimerId]; //1. find the next claimer task
    Isos_removeDueTaskByIndex(nextClaimerDueTaskIndex); //2. removes that task from the original position
    Isos_insertTaskOnDue(*currentDueIndex, &nextClaimerTask->Info, nextClaimerTask->Info.LastDueReported); //3. insert that task to the current index position, DO NOT change the due reported time
  }
  ++(*currentDueIndex); //simply trick the scheduler to run the next claimer immediately
  IsosRequestSorting = 1; //Isos_insertTaskOnDue should have done this, but there is no guarantee that the if condition where the function Isos_insertTaskOnDue is called above is always true
}

void Isos_handleLastClaimedResource(short* currentDueIndex){
  if (LastClaimedResourceTask == IsosResourceTaskType_Unspecified) //No resource claim is made
    return; //returns immediately
  //Otherwise, a resource task has just been claimed!
  Isos_prepareGenericTaskPointersById(IsosResourceTaskList[LastClaimedResourceTask]);
  Isos_insertTaskOnDue(*currentDueIndex, genericTaskInfo, Isos_GetClock()); //insert it to the due
  LastClaimedResourceTask = IsosResourceTaskType_Unspecified; //claim is received
  ++(*currentDueIndex); //trick the loop so that it will run the claimed resource task immediately
}

void Isos_Run(){
  //TODO wrap this entire function in while(1) loop when code not used for demonstration
  //Because there are many variables initialized here, static could probably help to save some initialization time
  static IsosClock measuredClock, clock;
  static short initialDueTaskSize, i;
  static IsosDueTask* dueTask;
  measuredClock = Isos_GetClock(); //the very first measured clock right now
  clock = IsosClock_Minus(&measuredClock, &LastSchedulerRun); //the difference between the clock now with the last time the scheduler runs
  clock = IsosClock_Minus(&clock, &SchedulerPeriod); //check if the difference computed above surpasses the scheduler period
  if (IsosClock_GetDirection(&clock) < 0) //the period for the scheduler to run has not come yet
    return;
  Isos_scheduler();
  LastSchedulerRun = measuredClock;
  initialDueTaskSize = IsosDueTaskSize; //assign the value to local variable first because it is going to change in the loop

  #if BASIC_DEBUG
  IsosDebugBasic_PrintDueTasks(IsosDueTaskList, initialDueTaskSize);
  #endif // BASIC_DEBUG
  for (i = initialDueTaskSize - 1; i >= 0; --i){
    dueTask = &IsosDueTaskList[i];
    Isos_execute(&IsosTaskList[dueTask->TaskId]);
    Isos_handleLastReleasedResource(&i);
    Isos_handleLastClaimedResource(&i);
  }
  LastSchedulerFinished = Isos_GetClock(); //maybe required for debugging
  #if BASIC_DEBUG
  IsosDebugBasic_PrintDueTasksEnding(initialDueTaskSize);
  #endif // BASIC_DEBUG
}

unsigned char Isos_GetTaskFlags(unsigned char taskId, unsigned char flagNo){
  if (taskId < 0 || taskId >= IsosTaskSize || flagNo < 0 || flagNo >= TASK_FLAGS_SIZE)
    return 0; //no raised flag should be return for invalid input case
  Isos_prepareGenericTaskPointersById(taskId);
  return genericTaskActionInfo->Flags[flagNo];
}

//Reminder: for resource task, Flags = Next Claimer Flag | Next Claimer Id | Next Claimer Priority | Reserved
void Isos_putNextClaimerFlags(unsigned char* resourceTaskInfoFlags, unsigned char nextClaimerId, unsigned char nextClaimerPriority){
  resourceTaskInfoFlags[0] = 1;
  resourceTaskInfoFlags[1] = nextClaimerId;
  resourceTaskInfoFlags[2] = nextClaimerPriority;
}

//1. parse the current flag to show if there is currently a next claimer, if there is not, just queue this new claimer
//2. if there is, get the Id and priority of the competing claimer
//3. check the priority of the current next claimer
//4. If the new next claimer has higher priority than the current next claimer, replace the flag, otherwise, let it be
void Isos_solveCompetingNextClaims(unsigned char* resourceTaskInfoFlags, unsigned char challengerId, unsigned char challengerPriority){
  unsigned char hasCompetitor, currentPriority;
  hasCompetitor = resourceTaskInfoFlags[0];
  if (!hasCompetitor){ //no competitor, just put this as next claimer
    Isos_putNextClaimerFlags(resourceTaskInfoFlags, challengerId, challengerPriority);
    return;
  }
  currentPriority = resourceTaskInfoFlags[2];
  if (challengerPriority > currentPriority) //only if the challenger is having HIGHER priority (not equal or less) than the current priority the next claimer can be changed
    Isos_putNextClaimerFlags(resourceTaskInfoFlags, challengerId, challengerPriority);
}

//Function to claim a resource task
//The correct way to use a resource: after successful claim, immediately prepares all the necessary things for the task to run
// (i.e. the Tx data) because the resource task is going to be immediately run!
//WARNING: as a best practice, DO NOT claim/release more than one resource task per subtask
char Isos_ClaimResourceTask(unsigned char claimerTaskId, IsosResourceTaskType type){
  IsosTask *claimerTask;
  if (type < 0 || type >= RESOURCE_SIZE) {//non-existing resource type
    #if BASIC_DEBUG
    IsosDebugBasic_PrintResourceClaiming(type, 0, -1);
    #endif // BASIC_DEBUG
    return 0;
  }
  Isos_prepareGenericTaskPointersById(IsosResourceTaskList[type]);
  if (genericTaskActionInfo->Enabled || genericTaskActionInfo->Subtask || genericTaskActionInfo->State != IsosTaskState_Initial){ //cannot claim enabled, started, or running resource task
    #if BASIC_DEBUG
    IsosDebugBasic_PrintResourceClaiming(type, 0, IsosResourceTaskList[type]);
    #endif // BASIC_DEBUG
    claimerTask = &IsosTaskList[claimerTaskId];
    Isos_solveCompetingNextClaims(genericTaskActionInfo->Flags, claimerTask->Info.Id, claimerTask->Info.Priority);
    return 0;
  }
  genericTaskActionInfo->Enabled = 1; //enable the resource task for use
  genericTaskInfo->TimeInfo.ExecutionDue = Isos_GetClock(); //execute immediately
  LastClaimedResourceTask = type;
  #if BASIC_DEBUG
  IsosDebugBasic_PrintResourceClaiming(type, 1, IsosResourceTaskList[type]);
  #endif // BASIC_DEBUG
  return 1;
}

IsosTaskState Isos_GetResourceTaskState(IsosResourceTaskType type){
  IsosTaskState taskState;
  if (type < 0 || type >= RESOURCE_SIZE){ //non-existing resource type
    #if BASIC_DEBUG
    IsosDebugBasic_PrintResourceChecking(type, IsosTaskState_Undefined, -1);
    #endif // BASIC_DEBUG
    return IsosTaskState_Undefined; //the only time the IsosTaskState_Undefined is used
  }
  taskState = IsosTaskList[IsosResourceTaskList[type]].Info.ActionInfo.State;
  #if BASIC_DEBUG
  IsosDebugBasic_PrintResourceChecking(type, taskState, IsosResourceTaskList[type]);
  #endif // BASIC_DEBUG
  return taskState;
}

//Function to release a resource task, always successful
//WARNING: as a best practice, DO NOT claim/release more than one resource task per subtask
void Isos_ReleaseResourceTask(IsosResourceTaskType type){
  if (type < 0 || type >= RESOURCE_SIZE) //non-existing resource type
    return;
  Isos_prepareGenericTaskPointersById(IsosResourceTaskList[type]);
  genericTaskActionInfo->Enabled = 0; //disabled the task on release. By right, it should have been disabled on failed or success in Isos_execute(). Consider this an extra reset if the task stuck, never really finishes its job)
  genericTaskActionInfo->Subtask = 0; //reset the subtask back to 0
  genericTaskActionInfo->State = IsosTaskState_Initial; //reset the resource task's state back to initial
  LastReleasedResourceTask = type;
  #if BASIC_DEBUG
  IsosDebugBasic_PrintResourceReleasing(type, IsosResourceTaskList[type]);
  #endif // BASIC_DEBUG
}

void Isos_Wait(unsigned char taskId, short waitingDay, long waitingMs){
  IsosClock clock, addClock;
  if (taskId < 0 || taskId >= IsosTaskSize)
    return; //such task does not exist
  clock = Isos_GetClock();
  addClock = IsosClock_Create(waitingDay, waitingMs);
  Isos_prepareGenericTaskPointersById(taskId);
  genericTaskActionInfo->State = IsosTaskState_Suspended; //put the task state to Suspended
  genericTaskInfo->SuspensionDue = IsosClock_Add(&clock, &addClock); //set the suspended time
  #if BASIC_DEBUG
  IsosDebugBasic_PrintWaitingNote(genericTaskInfo);
  #endif // BASIC_DEBUG
}

//Call this every 1 ms
void Isos_Tick(){
  IsosMainClock.Ms++;
  if (IsosMainClock.Ms >= MS_PER_DAY){
    IsosMainClock.Ms = 0;
    IsosMainClock.Day++;
  }
}

//May not really be the best way to expose tasks to the outsider, but this function is assumed to be called only by "super-user"
IsosTask* Isos_GetTask(unsigned char taskId){
  if (taskId < 0 || taskId >= IsosTaskSize)
    return (void *)0; //null pointer
  return &IsosTaskList[taskId];
}

short Isos_GetTaskSize(){ return IsosTaskSize; }
