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

#include "isos_buffer.h"
#include <string.h>

void IsosBuffer_Init(IsosBuffer* isosBuffer, unsigned char* buffer, short bufferSize){
  isosBuffer->Buffer = buffer;
  isosBuffer->BufferSize = bufferSize; //buffer size info should be provided
  IsosBuffer_ResetState(isosBuffer);
}

void IsosBuffer_ResetState(IsosBuffer* isosBuffer){
  memset(isosBuffer->Buffer, 0, isosBuffer->BufferSize); //set everything to zero
  isosBuffer->DataSize = 0;
  isosBuffer->PutIndex = 0;
  isosBuffer->GetIndex = 0;
}

void IsosBuffer_Flush(IsosBuffer* isosBuffer){
  isosBuffer->DataSize = 0; //set the data size to zero
  isosBuffer->GetIndex = isosBuffer->PutIndex; //get index equal to put index
}

//Expected to be used by ISR to put Rx data one-by-one in the buffer for later-retrieval
char IsosBuffer_Put(IsosBuffer* isosBuffer, unsigned char item){
  if (isosBuffer->DataSize >= isosBuffer->BufferSize) //buffer is full! nothing can be put anymore
    return 0;
  isosBuffer->Buffer[isosBuffer->PutIndex] = item;
  isosBuffer->DataSize++; //increases the data size of the buffer
  isosBuffer->PutIndex++; //increases the put index, so that next time item will be placed in the next index
  isosBuffer->PutIndex = isosBuffer->PutIndex % isosBuffer->BufferSize; //so that the index will never be placed outside the buffer memory
  return 1; //successful
}

//Expected to be used by ISR / task to peek data from the Rx buffer initiated by external party
char IsosBuffer_Peek(IsosBuffer* isosBuffer, unsigned char* item){
  if (!isosBuffer->DataSize) //nothing in the buffer
    return 0;
  *item = isosBuffer->Buffer[isosBuffer->GetIndex];
  return 1; //successful
}

//Expected to be used by ISR to put Tx data one-by-one from the buffer for transmission
char IsosBuffer_Get(IsosBuffer* isosBuffer, unsigned char* item){
  if (!isosBuffer->DataSize) //nothing in the buffer
    return 0;
  *item = isosBuffer->Buffer[isosBuffer->GetIndex];
  isosBuffer->DataSize--; //reduces the data size of the buffer
  isosBuffer->GetIndex++; //increases the get index, so that next time item will be obtained from the next index
  isosBuffer->GetIndex = isosBuffer->GetIndex % isosBuffer->BufferSize; //so that the index will never be placed outside the buffer memory
  return 1; //successful
}

//Expected to be used by resource task to put all Tx data in the buffer before starting the transmission
char IsosBuffer_Puts(IsosBuffer* isosBuffer, unsigned char* items, short itemSize){
  char willOverflow;
  short copySize, maxCopySize;
  if (isosBuffer->DataSize + itemSize > isosBuffer->BufferSize) //will overflow if continued!
    return 0; //unsuccessful
  maxCopySize = isosBuffer->BufferSize - isosBuffer->PutIndex;
  willOverflow = itemSize > maxCopySize;
  copySize = willOverflow ? maxCopySize : itemSize;
  memcpy(&isosBuffer->Buffer[isosBuffer->PutIndex], items, copySize); //copy to segment from this current index to the last index
  if (willOverflow) //if it will overflow, copy for the second time
    memcpy(isosBuffer->Buffer, &items[copySize], itemSize - copySize); //copy to segment from 0-index to the current index-1
  isosBuffer->DataSize += itemSize; //increase both the DataSize and PutIndex by itemSize
  isosBuffer->PutIndex += itemSize;
  isosBuffer->PutIndex = isosBuffer->PutIndex % isosBuffer->BufferSize; //so that the index will never be placed outside the buffer memory
  return 1;
}

//Expected to be used by ISR / task to peek all some data from Rx buffer
//If minItemSize is non-positive, then all data will be retrieved
//If minItemSize is positive, then only get all data if the DataSize in the buffer >= minItemSize
short IsosBuffer_Peeks(IsosBuffer* isosBuffer, unsigned char* items, short minItemSize){
  char willOverflow;
  short itemSize, copySize, maxCopySize;
  itemSize = isosBuffer->DataSize; //assume itemSize to be DataSize unless proven otherwise
  if (minItemSize > 0){ //positive expected minimum item size
    if (isosBuffer->DataSize + minItemSize > isosBuffer->BufferSize) //will overflow if continued!
      return 0; //unsuccessful
    if (isosBuffer->DataSize < minItemSize) //positive minItemSize but DataSize is smaller than that
      return 0; //unsuccessful
    itemSize = minItemSize; //updates itemSize to be retrieved to minItemSize as specified only on successful case
  }
  if (itemSize <= 0) //nothing to return
    return 0;
  maxCopySize = isosBuffer->BufferSize - isosBuffer->GetIndex;
  willOverflow = itemSize > maxCopySize;
  copySize = willOverflow ? maxCopySize : itemSize;
  memcpy(items, &isosBuffer->Buffer[isosBuffer->GetIndex], copySize); //copy from segment from this current index to the last index
  if (willOverflow)
    memcpy(&items[copySize], isosBuffer->Buffer, itemSize - copySize); //copy from segment from 0-index to the current index-1
  return itemSize; //returns the itemSize
}

//Expected to be used by resource task to retrieve all Rx data from the buffer for retrieval
//If minItemSize is non-positive, then all data will be retrieved
//If minItemSize is positive, then only get all data if the DataSize in the buffer >= minItemSize
short IsosBuffer_Gets(IsosBuffer* isosBuffer, unsigned char* items, short minItemSize){
  short itemSize;
  itemSize = IsosBuffer_Peeks(isosBuffer, items, minItemSize);
  if (!itemSize) //if no item size is detected, then immediately returns
    return 0;
  isosBuffer->DataSize -= itemSize; //reduce the data size as many as the itemSize retrieved
  isosBuffer->GetIndex += itemSize; //adds the get index pointer as many as the itemSize retrieved
  isosBuffer->GetIndex = isosBuffer->GetIndex % isosBuffer->BufferSize; //so that the index will never be placed outside the buffer memory
  return itemSize; //returns the itemSize
}

//Very special function to determine if a buffer contains expected data size
//See IsosBuffer.ExpectedDataSize description in isos_buffer.h
char IsosBuffer_HasExpectedDataSize(IsosBuffer* isosBuffer){
  if (isosBuffer->ExpectedDataSize < 0) //negative value means expecting any positive data size
    return isosBuffer->DataSize > 0;
  if (isosBuffer->ExpectedDataSize == 0) //zero value means does not expect any data size from this buffer
    return 1; //always return true
  //positive value can only be true if the data size in this buffer is AT LEAST as many as expected
  return isosBuffer->DataSize >= isosBuffer->ExpectedDataSize;
}
