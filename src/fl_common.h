
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

#ifndef FL_COMMON_H
#define FL_COMMON_H

#include "djan.h"


void flon_setup_logging(const char *context);

int flon_configure(char *root);
void flon_configure_j(fdja_value *obj);

fdja_value *flon_conf(const char *key);
int flon_conf_boolean(const char *key, int def);
long long flon_conf_int(const char *key, long long def);
char *flon_conf_string(const char *key, char *def);
char *flon_conf_path(const char *key, char *def);
int flon_conf_is(const char *key, const char *val);

char *flon_conf_uid();

fdja_value *flon_try_parse(char mode, const char *path, ...);
int flon_lock_write(fdja_value *v, const char *path, ...);

#endif // FL_COMMON_H

