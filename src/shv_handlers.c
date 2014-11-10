
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


static char *shv_determine_content_type(char *path)
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

char *shv_look_for_index(char *path, flu_dict *params, struct stat *sta)
{
  size_t l = strlen(path); if (path[l - 1] == '/') path[l - 1] = 0;

  char *i = flu_list_get(params, "index");
  if (i == NULL) i = "index.html";
  flu_list *is = flu_split(i, ",");

  char *r = NULL;

  for (flu_node *n = is->first; n; n = n->next)
  {
    char *rr = flu_sprintf("%s/%s", path, (char *)n->item);
    int x = stat(rr, sta);
    if (x == 0 && S_ISREG(sta->st_mode)) { r = rr; break; }
    free(rr);
  }

  flu_list_free_all(is);

  return r;
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

  //fgaj_d("r: %s", r);

  char *path = flu_sprintf("%s/%s", r, p);

  struct stat sta;
  if (stat(path, &sta) != 0) { free(path); return 0; }

  if (S_ISDIR(sta.st_mode))
  {
    char *p2 = shv_look_for_index(path, params, &sta);

    if (p2) { free(path); path = p2; }
    else { fgaj_d("we don't serve dirs %s", path); free(path); return 0; }
  }

  char *cp = flu_canopath(path); fgaj_d("path: %s", cp); free(cp);

  res->status_code = 200;

  char *h = flu_list_get(params, "header");
  if (h == NULL) h = flu_list_get(params, "h");
  if (h == NULL) h = "X-Accel-Redirect";

  flu_list_set(
    res->headers, "shv_content_length", flu_sprintf("%zu", sta.st_size));

  flu_list_set(
    res->headers, "content-type", shv_determine_content_type(path));

  flu_list_set(
    res->headers, "shv_file", strdup(path));
  flu_list_set(
    res->headers, h, path);

  return 1;
}

