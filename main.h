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

#include "isos.h"

//For simulation
//Resource task with Rx buffer only
#define RX_RETRIEVAL_NO 2 //to simulate on which cycle exactly the simulated RX would have been available for reading
#define MIN_RX_TO_CHECK 5
#define RX_DATA_BUFFER 10
#define TX_TRANSMITTED_NO 4 //to simulate on which cycle exactly the simulated TX would have been sent
#define TX_DATA_BUFFER 15

void registerTasks();

void RunOnceTask1(unsigned char taskId, IsosTaskActionInfo* taskActionInfo);
void RunOnceTask2(unsigned char taskId, IsosTaskActionInfo* taskActionInfo);
void RunOnceTask3(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //test lower priority resource task claimer at T:370ms
void LooselyRepeatedTask1(unsigned char taskId, IsosTaskActionInfo* taskActionInfo);
void LooselyRepeatedTask2(unsigned char taskId, IsosTaskActionInfo* taskActionInfo);
void LooselyRepeatedTask3(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //test task with suspension
void LooselyRepeatedTask4(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //test resource task with Rx only buffer
void LooselyRepeatedTask5(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //test resource task with Tx only buffer
void RepeatedTask1(unsigned char taskId, IsosTaskActionInfo* taskActionInfo);
void RepeatedTask2(unsigned char taskId, IsosTaskActionInfo* taskActionInfo);
void RepeatedTask3(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //test resource task with Tx & Rx buffers, called by size
void RepeatedTask4(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //test resource task with Tx & Rx buffers, called by time
void RepeatedTask5(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //test competing task for the same resource with a stuck-task
void PeriodicTask1(unsigned char taskId, IsosTaskActionInfo* taskActionInfo);
void PeriodicTask2(unsigned char taskId, IsosTaskActionInfo* taskActionInfo);
void PeriodicTask3(unsigned char taskId, IsosTaskActionInfo* taskActionInfo);
void PeriodicTask4(unsigned char taskId, IsosTaskActionInfo* taskActionInfo);
void PeriodicTask5(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //test OS response for a stuck-task claiming a resource
void PeriodicTask6(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //task which encounter occasional resource task's timeout
void ResourceTask1(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //normal resource task
void ResourceTask2(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //resource task with failure rate
void ResourceTask3(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //resource task with Tx only buffer
void ResourceTask4(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //resource task with Rx only buffer
void ResourceTask5(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //resource task with Tx & Rx buffers, called by size
void ResourceTask6(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //resource task with Tx & Rx buffers, called by time
void ResourceTask7(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //normal resource task for a stuck task
void ResourceTask8(unsigned char taskId, IsosTaskActionInfo* taskActionInfo); //resource task with occasional timeout
