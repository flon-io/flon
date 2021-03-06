
//
// Copyright (c) 2013-2016, John Mettraux, jmettraux+flon@gmail.com
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

// for specs

#ifndef FL_LISTENER_H
#define FL_LISTENER_H

#include "shervin.h"


char *flon_auth_enticate(
  fshv_env *e, const char *realm, const char *user, const char *pass);

int flon_dom_matches(const char *dom, const char *pat);
int flon_is_subdomain(const char *root, const char *dom);

int flon_may(char right, const char *user, const char *dom);
int flon_may_r(char right, fshv_env *env, const char *dom);

flu_list *flon_list_executions(
  const char *user, const char *path, const char *dom);

int flon_i_handler(fshv_env *e); // /i

int flon_in_handler(fshv_env *e); // /i/in

int flon_exes_handler(fshv_env *e); // /i/executions
int flon_exe_handler(fshv_env *e); // /i/executions/:domain or /:exid
int flon_exe_sub_handler(fshv_env *e); // /i/executions/:exid/log or /msg-log or /msgs
int flon_msg_handler(fshv_env *e); // /i/executions/:exid/msgs/:mid

int flon_metrics_handler(fshv_env *e); // /i/metrics

#endif // FL_LISTENER_H

