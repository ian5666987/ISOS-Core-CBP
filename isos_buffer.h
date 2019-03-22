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

  isos_buffer.c, isos_buffer.h
  - Describe the buffer structures and functions used in ISOS
  - Buffers in ISOS are circular
*/

#ifndef ISOS_BUFFER_H
#define ISOS_BUFFER_H

//The IsosBuffer is circular
typedef struct IsosBufferStruct {
  unsigned char* Buffer; //pointer to buffer
  short BufferSize; //the size of the buffer
  short PutIndex; //current put index of the buffer
  short GetIndex; //current get index of the buffer
  short DataSize; //the current size of data buffered. As long as > 0, means there is (are) data
  short ExpectedDataSize; //ExpectedDataSize: very special parameter
                          // negative value means expecting any data size on this buffer
                          // zero value means expecting no data size at all on this buffer
                          // positive value means expecting AT LEAST specified (expected) data size found in this buffer
} IsosBuffer;

void IsosBuffer_Init(IsosBuffer* isosBuffer, unsigned char* buffer, short bufferSize);
void IsosBuffer_ResetState(IsosBuffer* isosBuffer); //if something goes wrong, use this to reset the buffer's state
void IsosBuffer_Flush(IsosBuffer* isosBuffer); //to clear the buffer up to this state, clearing any remaining data, if there is any
char IsosBuffer_Put(IsosBuffer* isosBuffer, unsigned char item); //to put data to the buffer, if unsuccessful, returns 0
char IsosBuffer_Peek(IsosBuffer* isosBuffer, unsigned char* item); //to peek data from the buffer, DO NOT removes it from the buffer when successful, if unsuccessful, returns 0
char IsosBuffer_Get(IsosBuffer* isosBuffer, unsigned char* item); //to get data from the buffer, removes it from the buffer when successful, if unsuccessful, returns 0
char IsosBuffer_Puts(IsosBuffer* isosBuffer, unsigned char* items, short itemSize); //to put data (plural) to the buffer, if unsuccessful, returns 0
//to peek data (plural) from the buffer, DO NOT removes it from the buffer when successful. If unsuccessful, returns 0. If successful, returns the itemSize retrieved.
short IsosBuffer_Peeks(IsosBuffer* isosBuffer, unsigned char* items, short minItemSize);
//to get data (plural) from the buffer, use non-positive minItemSize to indicate "retrieve all available data". If unsuccessful, returns 0. If successful, returns the itemSize retrieved.
short IsosBuffer_Gets(IsosBuffer* isosBuffer, unsigned char* items, short minItemSize);
char IsosBuffer_HasExpectedDataSize(IsosBuffer* isosBuffer); //very special function which makes use of IsosBuffer.ExpectedDataSize info

#endif // ISOS_BUFFER_H
