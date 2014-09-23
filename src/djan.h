
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

#ifndef DJAN_H
#define DJAN_H

#include <stddef.h>

// 's' string
// 'n' number
// 'o' object
// 'a' array
// 't' true
// 'f' false
// '0' null or '\0'

typedef struct dja_value {
  char *key; // when set, the value is an entry...
  char type; // or short type; ?
  char *source;
  size_t soff;
  size_t slen; // if slen == 0, the source string is "owned" bv the value
  struct dja_value *sibling;
  struct dja_value *child;
} dja_value;

dja_value *dja_parse(char *input);
dja_value *dja_parse_fragment(char *input, size_t offset, size_t length);

dja_value *dja_parse_radial(char *input);

char *dja_dump(dja_value *v);
char *dja_to_json(dja_value *v);
char *dja_to_djan(dja_value *v);
char *dja_to_radial(dja_value *v);

/*
 * Frees the dja_value resources. If the dja_value has children, they
 * will be freed as well.
 */
void dja_value_free(dja_value *v);

/*
 * Returns a copy of the string representation of the dja_value.
 */
char *dja_string(dja_value *v);

/*
 * Returns the string value behind the dja_value. For a string dja_value,
 * the enclosing double quotes will not be included and the string will be
 * unescaped.
 */
char *dja_to_string(dja_value *v);

/*
 * Returns a string representation of the value v.
 */
char *dja_value_to_string(dja_value *v);

int dja_to_int(dja_value *v);
double dja_to_double(dja_value *v);

size_t dja_size(dja_value *v);
dja_value *dja_value_at(dja_value *v, long n);
dja_value *dja_lookup(dja_value *v, const char *path);
char *dja_lookup_string(dja_value *v, const char *path);

int dja_push(dja_value *array, dja_value *v);
int dja_set(dja_value *object, const char *key, dja_value *v);

#endif // DJAN_H

