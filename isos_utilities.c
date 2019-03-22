#include "isos_utilities.h"
#include <stdarg.h>
#include <stddef.h>

void IsosUtility_FillBuffer(short len, unsigned char* buffer, ...){
  va_list valist;
  short i;
  va_start(valist, buffer);
  for(i = 0; i < len; ++i)
    buffer[i] = va_arg(valist, int);
  va_end(valist);
}
