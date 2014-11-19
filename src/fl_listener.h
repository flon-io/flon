
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

// for specs

#ifndef FL_LISTENER_H
#define FL_LISTENER_H

#include "shervin.h"


int flon_auth_filter(shv_request *req, shv_response *res, flu_dict *params);
int flon_auth_enticate(char *user, char *pass);

int flon_dom_matches(const char *dom, const char *pat);

int flon_may(char right, const char *user, const char *dom);
int flon_may_r(char right, shv_request *req, const char *dom);

flu_list *flon_list_executions(const char *user, const char *path);

int flon_i_handler( // /i
  shv_request *req, shv_response *res, flu_dict *params);

int flon_in_handler( // /i/in
  shv_request *req, shv_response *res, flu_dict *params);

int flon_exes_handler( // /i/executions
  shv_request *req, shv_response *res, flu_dict *params);
int flon_exe_handler( // /i/executions/:domain or /:exid
  shv_request *req, shv_response *res, flu_dict *params);
int flon_exe_sub_handler( // /i/executions/:exid/log or /msg-log or /msgs
  shv_request *req, shv_response *res, flu_dict *params);
int flon_exe_msg_handler( // /i/executions/:exid/msgs/:mid
  shv_request *req, shv_response *res, flu_dict *params);

int flon_metrics_handler( // /i/metrics
  shv_request *req, shv_response *res, flu_dict *params);

#endif // FL_LISTENER_H

