Inspire Satellite Operating System (ISOS) is an (1) easy-to-understand, (2) easy-to-use, (3) small-sized, (4) fairly-comprehensive Operating System (OS) software used in Inspire Satellite-4 (IS-4) flight software as a replacement for the earlier non-OS flight software developed fore Inspire Satellite-1 (IS-1)

1) Easy-to-understand:
   - Uses simple, straight forward, yet efficient C programming style
   - Does not use stack pointers manipulation at all, so it is fully compatible with any code optimizers
   - Very clear naming of the files, variables, and functions - leaving little rooms for guessing what they stand for
   - Configurable task behaviors (based on task cyclical/non-cyclical types) and structures
   - No convoluted/fanciful-looking macros, code, etc, just pure, simple C
   - No deeply-nested/recursive code for everything (quick_sort algorithm excepted), making it very easy to read, even for beginner C coder
   - Uses (i) simple switch case and (ii) concept of subtask, instead of stack pointers manipulation to control segmentation of the tasks in a task function
2) Easy-to-use:
   - One line code to initialize
   - One line code to register each of the various type of tasks
   - One line code to run the OS
     Note: See note (TODO) in the Isos_Run() function to implement
   - One file to configure the OS settings (Isos_task.h), with only six (6) macros to set
   - Demonstrations/examples provided
3) Small-sized
   - The entire OS code is less than 1000 lines
   - Four (4) small C + header files only (+1 quick_sort and +1 debugging file-pairs)
4) Fairly-comprehensive
   - Sufficient for most of small-to-medium micro-controller OS development project requirements
   - Capable of handling various type of tasks with different time-cycle/non-cycle requirements
   - Capable of handling multiple level of priorities of the tasks
   - Capable of managing use of shared resources among the tasks using simple claiming-releasing mechanism
   - Capable of segmenting single-function task into various task segments (called subtask)
   - Capable of handling task waiting and simple inter-task signals using char[] flags
   - Capable of handling (resource) tasks with Tx and/or Rx buffers
   - Capable of handling task timeouts (that is, killing "dead" tasks)

Created by: Ian K (c) 2019
Coded using: CodeBlocks 17.12 IDE (http://www.codeblocks.org/)