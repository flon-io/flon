
//
// helpers for l/ specs
//
// Tue Nov 18 16:40:44 JST 2014
//

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <unistd.h>

#include "flutil.h"
#include "l_helpers.h"


void llog(char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *fo = flu_sprintf("[1;30m.:n:. %s[0;0m\n", format);
  vprintf(fo, ap);
  free(fo);
  va_end(ap);
}

