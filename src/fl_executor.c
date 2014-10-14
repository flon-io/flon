
//
// Copyright (c) 2013-2014, John Mettraux, jmettraux+flon@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Made in Japan.
//

#define _POSIX_C_SOURCE 200809L

#include <string.h>

#include "djan.h"
#include "gajeta.h"
#include "fl_executor.h"


//typedef fabr_tree *fabr_p_func(
//  const char *, size_t, size_t, fabr_parser *, int flags);
typedef int flon_exe_func(fdja_value *);

// INVOKE
//
static int exe_invoke(fdja_value *exe)
{
  return 1;
}

typedef struct {
  char *name;
  flon_exe_func *function;
} name_function;

static name_function *name_functions[] = {
  &(name_function){ "invoke", exe_invoke },
  NULL
};

static int flon_exec(fdja_value *exe)
{
  fdja_value *x = fdja_lookup(exe, "execute");
  char *name = fdja_lookup_string(x, "0", NULL);
  flon_exe_func *func = NULL;

  fgaj_d("name: '%s'", name);

  for (size_t i = 0; name_functions[i] != NULL; ++i)
  {
    name_function *nf = name_functions[i];
    if (strcmp(nf->name, name) == 0) { func = nf->function; break; }
  }

  if (func == NULL)
  {
    fgaj_e("don't know how to execute \"%s\"", name); return 1;
  }

  // TODO: create node...

  return func(exe);
}

int flon_execute(const char *path)
{
  fdja_value *exe = fdja_parse_obj_f(path);

  // TODO: hydrate ?

  if (exe == NULL)
  {
    fgaj_r("couldn't read exe msg at %s", path); return 1;
  }

  //printf(">>>\n%s\n<<<\n", fdja_to_json(inv));

  if (fdja_lookup(exe, "execute") == NULL)
  {
    fgaj_e("no 'execute' key in the message"); return 1;
  }

  //printf(">>>\n%s\n<<<\n", fdja_to_json(exe));

  return flon_exec(exe);
}

