
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

// https://github.com/flon-io/djan

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

typedef struct fdja_value {
  char *key; // when set, the value is an entry...
  char type; // or short type; ?
  char *source;
  size_t soff;
  size_t slen; // if slen == 0, the source string is "owned" bv the value
  struct fdja_value *sibling;
  struct fdja_value *child;
} fdja_value;

fdja_value *fdja_parse(char *input);
//fdja_value *fdja_parse_fragment(char *input, size_t offset, size_t length);

#define fdja_v(input) fdja_parse(input)

fdja_value *fdja_parse_radial(char *input);

fdja_value *fdja_parse_obj(char *input);
fdja_value *fdja_parse_obj_f(const char *path);

#define fdja_o(input) fdja_parse_obj(input)

char *fdja_to_json(fdja_value *v);
char *fdja_to_djan(fdja_value *v);
//char *fdja_to_radial(fdja_value *v);

//int fdja_to_json_f(fdja_value *v, const char *path);

/*
 * Frees the fdja_value resources. If the fdja_value has children, they
 * will be freed as well.
 */
void fdja_value_free(fdja_value *v);

/*
 * Returns a copy of the string representation of the fdja_value.
 */
char *fdja_string(fdja_value *v);

/*
 * Returns the string value behind the fdja_value. For a string fdja_value,
 * the enclosing double quotes will not be included and the string will be
 * unescaped.
 */
char *fdja_to_string(fdja_value *v);

long long fdja_to_int(fdja_value *v);
long double fdja_to_double(fdja_value *v);

size_t fdja_size(fdja_value *v);

fdja_value *fdja_value_at(fdja_value *v, long n);

fdja_value *fdja_lookup(fdja_value *v, const char *path);

char *fdja_lookup_string(fdja_value *v, const char *path, char *def);
long long fdja_lookup_int(fdja_value *v, const char *path, long long def);
int fdja_lookup_boolean(fdja_value *v, const char *path, int def);
int fdja_lookup_bool(fdja_value *v, const char *path, int def);

int fdja_push(fdja_value *array, fdja_value *v);
int fdja_set(fdja_value *object, const char *key, fdja_value *v);

int fdja_splice(fdja_value *array, size_t start, size_t count, ...);

//int fdja_pset(fdja_value *start, const char *path, fdja_value *v);
  // pset(v, "cars.-1", v1) to push in cars array

#endif // DJAN_H

