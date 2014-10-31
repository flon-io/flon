
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

// https://github.com/flon-io/bocla

// bocla.h

#ifndef FLON_BOCLA_H
#define FLON_BOCLA_H

#include "flutil.h"

typedef struct fcla_response {
  short status_code;
  flu_list *headers;
  char *body;
} fcla_response;

void fcla_response_free(fcla_response *r);

fcla_response *fcla_get_h(char *uri, flu_list *headers);

fcla_response *fcla_get(char *uri);
fcla_response *fcla_head(char *uri);
fcla_response *fcla_delete(char *uri);

fcla_response *fcla_post(char *uri, flu_list *headers, char *body);
fcla_response *fcla_post_f(char *uri, flu_list *headers, char *path);
fcla_response *fcla_put(char *uri, flu_list *headers, char *body);
fcla_response *fcla_put_f(char *uri, flu_list *headers, char *path);


//
// helpers

flu_list *fcla_extract_headers(char *head);

#endif // FLON_BOCLA_H

