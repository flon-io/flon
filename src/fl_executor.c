
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


// holding the current execution as a global variable

static fdja_value *execution = NULL;

//
// execute and receive functions

typedef int flon_exe_func(fdja_value *);

// INVOKE

static int exe_invoke(fdja_value *exe)
{
  return 1; // failure
}

static int rcv_invoke(fdja_value *rcv)
{
  return 1; // failure
}

typedef struct {
  char *name;
  flon_exe_func *exe;
  flon_exe_func *rcv;
} name_function;

static name_function *name_functions[] = {
  &(name_function){ "invoke", exe_invoke, rcv_invoke },
  NULL
};

static int flon_receive_j(fdja_value *msg)
{
  // TODO
  // TODO: hydrate

  return 1; // failure
}

static int flon_execute_j(fdja_value *msg)
{
  fdja_value *x = fdja_lookup(msg, "execute");
  char *name = fdja_lookup_string(x, "0", NULL);
  flon_exe_func *func = NULL;

  fgaj_d("name: '%s'", name);

  for (size_t i = 0; name_functions[i] != NULL; ++i)
  {
    name_function *nf = name_functions[i];
    if (strcmp(nf->name, name) == 0) { func = nf->exe; break; }
  }

  if (func == NULL)
  {
    fgaj_e("don't know how to execute \"%s\"", name); return 1;
  }

  // TODO: hydrate
  // TODO: create node...

  return func(msg);
}

int flon_execute(const char *path)
{
  fdja_value *msg = fdja_parse_obj_f(path);

  if (msg == NULL) { fgaj_r("couldn't read %s", path); return 1; }

  //printf(">>>\n%s\n<<<\n", fdja_to_json(j));

  if (fdja_lookup(msg, "execute")) return flon_execute_j(msg);
  if (fdja_lookup(msg, "receive")) return flon_receive_j(msg);

  fgaj_e("no 'execute' or 'receive' key in %s", path);
  return 1; // failure
}

