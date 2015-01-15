
//
// helpers
//
// Tue Jan 13 06:21:52 JST 2015
//


#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>

#include "flutil.h"
#include "flutim.h"


int hlp_wait_for_file(char filetype, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *p = flu_svprintf(path, ap);
  int maxsec = va_arg(ap, int);
  va_end(ap);

  char t = 0;

  for (size_t i = 0; i < maxsec * 50; ++i) // approx...
  {
    flu_msleep(20);

    t = flu_fstat(p);

    if (t != 0) break;
  }

  free(p);

  return t == filetype;
}

