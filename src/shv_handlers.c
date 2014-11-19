
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

// https://github.com/flon-io/shervin

// handlers

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>

#include "flutil.h"
#include "gajeta.h"
#include "shervin.h"
#include "shv_protected.h"


static char *shv_determine_content_type(const char *path)
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
  else r = "text/plain";

  return strdup(r);
}

ssize_t shv_serve_file(
  shv_response *res, flu_dict *params, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *pa = flu_vpath(path, ap);
  va_end(ap);

  struct stat sta;
  if (stat(pa, &sta) != 0) { free(pa); return -1; }
  if (S_ISDIR(sta.st_mode)) { free(pa); return 0; }

  res->status_code = 200;

  char *h = flu_list_get(params, "header");
  if (h == NULL) h = flu_list_get(params, "h");
  if (h == NULL) h = "X-Accel-Redirect";

  flu_list_set(
    res->headers, "shv_content_length", flu_sprintf("%zu", sta.st_size));

  flu_list_set(
    res->headers, "content-type", shv_determine_content_type(pa));

  flu_list_set(
    res->headers, "shv_file", strdup(pa));
  flu_list_set(
    res->headers, h, pa);

  return sta.st_size;
}

int shv_dir_handler(shv_request *req, shv_response *res, flu_dict *params)
{
  char *p = flu_list_get(req->routing_d, "**");
  if (p == NULL) return 0;

  //fgaj_d("p: %s", p);

  if (strstr(p, "..")) return 0;

  char *r = flu_list_get(params, "root");
  if (r == NULL) r = flu_list_get(params, "r");
  if (r == NULL) return 0;
  //printf("r: %s\n", r);

  char *path = flu_path("%s/%s", r, p);

  ssize_t s = shv_serve_file(res, params, path);

  if (s < 0) { free(path); return 0; }

  if (s == 0)
  {
    char *i = flu_list_getd(params, "index", "index.html");
    flu_list *is = flu_split(i, ",");

    for (flu_node *n = is->first; n; n = n->next)
    {
      char *p = flu_path("%s/%s", path, (char *)n->item);
      s = shv_serve_file(res, params, p);
      free(p);
      if (s > 0) break;
    }

    flu_list_free_all(is);
  }

  free(path);

  return s < 1 ? 0 : 1;
}

