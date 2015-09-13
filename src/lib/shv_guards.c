
//
// Copyright (c) 2013-2015, John Mettraux, jmettraux+flon@gmail.com
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

// https://github.com/flon-io/shervin

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "shv_protected.h"


int fshv_path_match(fshv_env *e, int sub, const char *path)
{
  char *rpath = NULL;
  if (sub) rpath = flu_list_get(e->bag, "**");
  else rpath = e->req->uri->path;

  //printf(
  //  "fshv_path_match() sub%d path >%s<\nrpath >%s<\n",
  //  sub, path, rpath);

  if (rpath == NULL) return 0;

  if (*path != '/')
  {
    char m = tolower(path[0]);
    if (tolower(path[1]) == 'u') m = 'u';
    char rm = e->req->method;
    if (rm == 'h') rm = 'g';
    if (m != rm) return 0;
    path = strchr(path, ' ') + 1;
  }

  int success = 1;
  flu_list *bag = flu_list_malloc();

  while (1)
  {
    //printf("***\n");
    //printf("path  >%s<\n", path);
    //printf("rpath >%s<\n", rpath);

    char *slash = strchr(path, '/');
    char *rslash = strchr(rpath, '/');

    if (slash == NULL) slash = strchr(path, '\0');
    if (rslash == NULL) rslash = strchr(rpath, '\0');

    if (*path == ':')
    {
      char *k = strndup(path + 1, slash - path - 1);
      //printf("k >%s<\n", k);
      flu_list_set(bag, k, strndup(rpath, rslash - rpath));
      free(k);
    }
    else if (strcmp(path, "**") == 0)
    {
      flu_list_set(bag, "**", strdup(rpath)); break;
    }
    else
    {
      //printf("s - p : %zu / rs - rp : %zu\n", slash - path, rslash - rpath);
      //printf("p >%s<  rp >%s<\n", path, rpath);
      if (slash - path != rslash - rpath) { success = 0; break; }
      if (strncmp(path, rpath, slash - path) != 0) { success = 0; break; }
    }

    if (*slash == 0 && *rslash == 0) break;

    if (*rslash == 0 && strcmp(slash, "/**") == 0)
    {
      flu_list_set(bag, "**", strdup("")); break;
    }
    if (*slash == 0 || *rslash == 0)
    {
      success = 0; break;
    }

    path = slash + 1;
    rpath = rslash + 1;
  }

  if (success)
  {
    for (flu_node *n = bag->first; n; n = n->next)
    {
      flu_list_set(e->bag, n->key, n->item);
    }

    flu_list_free(bag);
  }
  else
  {
    flu_list_free_all(bag);
  }

  return success;
}

int fshv_path_match_and_auth(
  fshv_env *env, int sub, const char *path, const char *realm)
{
  return 0;
}

//commit 2e039a2191f1ff3db36d3297a775c3a1f58841e0
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Sep 13 06:32:55 2015 +0900
//
//    bring back all specs to green
//    
//    (one yellow remaining though)
