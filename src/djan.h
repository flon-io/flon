
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
#include <stdarg.h>


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
  size_t slen;
  short sowner; // if 1 then this fdja_value "owns" the source (has to free it)
  struct fdja_value *sibling;
  struct fdja_value *child;
} fdja_value;

fdja_value *fdja_parse(char *input);
fdja_value *fdja_dparse(char *input);
fdja_value *fdja_parse_f(const char *path, ...);

fdja_value *fdja_v(char *format, ...);
char *fdja_vj(char *format, ...);

/* Wraps a string in a fdja_value.
 */
fdja_value *fdja_s(char *format, ...);

fdja_value *fdja_parse_radial(char *input);
fdja_value *fdja_dparse_radial(char *input);
fdja_value *fdja_parse_radial_f(const char *path, ...);

fdja_value *fdja_parse_obj(char *input);
fdja_value *fdja_dparse_obj(char *input);
fdja_value *fdja_parse_obj_f(const char *path, ...);

fdja_value *fdja_c(char *input, ...);

//fdja_value *fdja_a(fdja_value *v0, ...);
//fdja_value *fdja_o(char *k0, fdja_value *v0, ...);

fdja_value *fdja_clone(fdja_value *v);

char *fdja_to_json(fdja_value *v);

enum // flags for fdja_to_djan()
{
  FDJA_F_COLOR    = 1 << 0, // colour output
  FDJA_F_ONELINE  = 1 << 1  // result will come in one line
};

char *fdja_to_djan(fdja_value *v, int flags);
#define fdja_tod(v) fdja_to_djan(v, 2)
#define fdja_todc(v) fdja_to_djan(v, 1)

//char *fdja_to_radial(fdja_value *v);

int fdja_to_json_f(fdja_value *v, const char *path, ...);

/*
 * Frees the fdja_value resources. If the fdja_value has children, they
 * will be freed as well.
 */
void fdja_value_free(fdja_value *v);
#define fdja_free(v) fdja_value_free(v)

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

fdja_value *fdja_vlookup(fdja_value *v, const char *path, va_list ap);
fdja_value *fdja_lookup(fdja_value *v, const char *path, ...);
fdja_value *fdja_lookup_c(fdja_value *v, const char *path, ...);

char *fdja_lookup_string(fdja_value *v, const char *path, ...);
char *fdja_lookup_string_dup_default(fdja_value *v, const char *path, ...);
long long fdja_lookup_int(fdja_value *v, const char *path, ...);
int fdja_lookup_boolean(fdja_value *v, const char *path, ...);
int fdja_lookup_bool(fdja_value *v, const char *path, ...);
  //
  // the last arg is the default value

#define fdja_l(...) fdja_lookup(__VA_ARGS__)
#define fdja_lc(...) fdja_lookup_c(__VA_ARGS__)
#define fdja_ls(...) fdja_lookup_string(__VA_ARGS__)
#define fdja_lsd(...) fdja_lookup_string_dup_default(__VA_ARGS__)
#define fdja_li(...) fdja_lookup_int(__VA_ARGS__)
#define fdja_lb(...) fdja_lookup_bool(__VA_ARGS__)

char *fdja_lj(fdja_value *v, const char *path, ...);

int fdja_push(fdja_value *array, fdja_value *v);
int fdja_set(fdja_value *object, const char *key, fdja_value *v);

int fdja_merge(fdja_value *dst, fdja_value *src);

int fdja_splice(fdja_value *array, long long start, size_t count, ...);

int fdja_pset(fdja_value *start, const char *path, ...);
  // pset(v, "cars.-1", v1) to push in cars array
  // pset(v, "cars.%i", -1, v1) too

int fdja_psetf(fdja_value *start, const char *path, ...);

#endif // DJAN_H

