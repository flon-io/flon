
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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "flutim.h"
#include "gajeta.h"
#include "shv_protected.h"


//
// misc

char fshv_method_to_char(char *s)
{
  if (strncmp(s, "GET", 3) == 0) return 'g';
  if (strncmp(s, "PUT", 3) == 0) return 'u';
  if (strncmp(s, "POST", 4) == 0) return 'p';
  if (strncmp(s, "HEAD", 4) == 0) return 'h';
  if (strncmp(s, "TRACE", 5) == 0) return 't';
  if (strncmp(s, "DELETE", 6) == 0) return 'd';
  if (strncmp(s, "OPTIONS", 7) == 0) return 'o';
  if (strncmp(s, "CONNECT", 7) == 0) return 'c';
  return '?';
}

char *fshv_char_to_method(char c)
{
  if (c == 'g') return "GET";
  if (c == 'u') return "PUT";
  if (c == 'p') return "POST";
  if (c == 'h') return "HEAD";
  if (c == 't') return "TRACE";
  if (c == 'd') return "DELETE";
  if (c == 'o') return "OPTIONS";
  if (c == 'c') return "CONNECT";
  return "???";
}

static char *fshv_determine_content_type(const char *path)
{
  // TODO: utf-8? "text/html; charset=UTF-8"
  // TODO: manage that with a conf file

  char *suffix = strrchr(path, '.');
  char *r = NULL;

  if (suffix == NULL) r = "text/plain";
  else if (strcmp(suffix, ".txt") == 0) r = "text/plain";
  else if (strcmp(suffix, ".js") == 0) r = "application/javascript";
  else if (strcmp(suffix, ".json") == 0) r = "application/json";
  else if (strcmp(suffix, ".css") == 0) r = "text/css";
  else if (strcmp(suffix, ".scss") == 0) r = "text/css"; // ?
  else if (strcmp(suffix, ".html") == 0) r = "text/html";
  else if (strcmp(suffix, ".pdf") == 0) r = "application/pdf";
  else r = "text/plain";

  return strdup(r);
}

ssize_t fshv_serve_file(fshv_env *env, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *pa = flu_vpath(path, ap);
  va_end(ap);

  struct stat sta;
  if (stat(pa, &sta) != 0) { free(pa); return -1; }
  if (S_ISDIR(sta.st_mode)) { free(pa); return 0; }

  env->res->status_code = 200;

  flu_list_set(
    env->res->headers, "fshv_content_length", flu_sprintf("%zu", sta.st_size));
  flu_list_set(
    env->res->headers, "content-type", fshv_determine_content_type(pa));
  flu_list_set(
    env->res->headers, "fshv_file", strdup(pa));

  char *h = flu_list_getod(env->conf, "accel-header", "X-Accel-Redirect");
  flu_list_set(env->res->headers, h, pa);

  return sta.st_size;
}


//
// auth

void fshv_set_user(fshv_env *env, const char *realm, const char *user)
{
  flu_list_setk(env->bag, flu_sprintf("_%s_user", realm), strdup(user), 0);
}

char *fshv_get_user(fshv_env *env, const char *realm)
{
  if (realm)
  {
    char *k = flu_sprintf("_%s_user", realm);
    char *r = flu_list_get(env->bag, k);
    free(k);

    return r;
  }

  for (flu_node *fn = env->bag->first; fn; fn = fn->next)
  {
    if (*fn->key != '_') continue;
    char *u = strrchr(fn->key, '_');
    if (u == NULL || strcmp(u, "_user") != 0) continue;

    return fn->item;
  }

  return NULL;
}

//commit 4f600185cfdd86e14d35ea326de3121ffa4ea561
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Oct 18 15:19:12 2015 +0900
//
//    implement fshv_malloc_x()
