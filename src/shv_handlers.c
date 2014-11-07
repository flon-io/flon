
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

// handlers, and guards

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gajeta.h"
#include "shervin.h"
#include "shv_protected.h"


//
// guards

int shv_filter_guard(shv_request *req, shv_response *res, flu_dict *params)
{
  return 1;
}

int shv_any_guard(shv_request *req, shv_response *res, flu_dict *params)
{
  return 1;
}

int shv_path_guard(shv_request *req, shv_response *res, flu_dict *params)
{
  char *path = (char *)flu_list_get(params, "path");
  char *rpath = (char *)flu_list_get(req->uri_d, "_path");

  if (*path != '/')
  {
    char m = tolower(path[0]);
    if (tolower(path[1]) == 'u') m = 'u';
    char rm = req->method;
    if (rm == 'h') rm = 'g';
    if (m != rm) return 0;
    path = strchr(path, ' ') + 1;
  }

  int success = 1;

  while (1)
  {
    char *slash = strchr(path, '/');
    char *rslash = strchr(rpath, '/');

    if (slash == NULL) slash = strchr(path, '\0');
    if (rslash == NULL) rslash = strchr(rpath, '\0');

    if (*path == ':')
    {
      char *k = strndup(path + 1, slash - path - 1);
      flu_list_set(req->routing_d, k, strndup(rpath, rslash - rpath));
      free(k);
    }
    else if (strcmp(path, "**") == 0)
    {
      flu_list_set(req->routing_d, "**", strdup(rpath)); break;
    }
    else
    {
      if (strncmp(path, rpath, slash - path) != 0) { success = 0; break; }
    }

    if (*slash == 0 && *rslash == 0) break;

    if (*rslash == 0 && strcmp(slash, "/**") == 0)
    {
      flu_list_set(req->routing_d, "**", strdup("")); break;
    }
    if (*slash == 0 || *rslash == 0)
    {
      success = 0; break;
    }

    path = slash + 1;
    rpath = rslash + 1;
  }

  return success;
}


//
// handlers

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
  else if (strcmp(suffix, ".html") == 0) r = "text/html";

  return strdup(r);
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

  fgaj_d("path: %s", flu_canopath(path));

  struct stat sta;
  if (stat(path, &sta) != 0) { free(path); return 0; }

  if (S_ISDIR(sta.st_mode))
  {
    fgaj_d("we don't serve dirs %s", path); free(path); return 0;
  }

  res->status_code = 200;

  char *h = flu_list_get(params, "header");
  if (h == NULL) h = flu_list_get(params, "h");
  if (h == NULL) h = strdup("X-Accel-Redirect");

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

