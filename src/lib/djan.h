
//
// Copyright (c) 2013-2015, John Mettraux, jmettraux+flon@gmail.com
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

#ifndef FLON_DJAN_H
#define FLON_DJAN_H

#include <stdio.h>
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
  ssize_t ind; // only used by radial
} fdja_value;

fdja_value *fdja_array_malloc();
fdja_value *fdja_object_malloc();

fdja_value *fdja_parse(char *input);
fdja_value *fdja_dparse(char *input);
fdja_value *fdja_fparse(FILE *f);
fdja_value *fdja_parse_f(const char *path, ...);

fdja_value *fdja_v(const char *format, ...);
char *fdja_vj(const char *format, ...);

/* Wraps a string in a fdja_value.
 */
fdja_value *fdja_s(const char *format, ...);

/* Wraps a string as a symbol, directly owns it.
 * Warning: performs no escaping on the input!
 */
fdja_value *fdja_sym(char *s);

/* Composes an array from its args. Expects a final NULL.
 */
fdja_value *fdja_a(fdja_value *v0, ...);

/* Composes an object. Expects a final NULL.
 * May compose the keys as well.
 */
fdja_value *fdja_o(char *k0, ...);

fdja_value *fdja_parse_radial(char *input, const char *origin);
fdja_value *fdja_dparse_radial(char *input);
fdja_value *fdja_fparse_radial(FILE *fm, const char *origin);
fdja_value *fdja_parse_radial_f(const char *path, ...);

/* Expect a size_t (start line number) as last argument.
 */
fdja_value *fdja_parse_r(const char *format, ...);

fdja_value *fdja_parse_obj(char *input);
fdja_value *fdja_dparse_obj(char *input);
fdja_value *fdja_fparse_obj(FILE *f);
fdja_value *fdja_parse_obj_f(const char *path, ...);

/* Parses a 'conf' object (surrounding {} optional).
 */
fdja_value *fdja_c(const char *input, ...);

//fdja_value *fdja_a(fdja_value *v0, ...);
//fdja_value *fdja_o(char *k0, fdja_value *v0, ...);

fdja_value *fdja_clone(fdja_value *v);

/* Returns 1 if v is a string, a sqstring or a symbol.
 */
int fdja_is_stringy(fdja_value *v);

char *fdja_to_json(fdja_value *v);

int fdja_to_json_f(fdja_value *v, const char *path, ...);

enum // flags for fdja_to_djan()
{
  FDJA_F_COLOR    = 1 << 0, // colour output
  FDJA_F_ONELINE  = 1 << 1, // result will come in one line
  FDJA_F_COMPACT  = 1 << 2, // outputs [1,2,3] instead of [ 1, 2, 3 ]
  FDJA_F_OBJ      = 1 << 3, // do not output {} for top object
  FDJA_F_NULL     = 1 << 4  // output NULL in uppercase [red]
};
  // compact implies oneline...

/* Low level functions, available for dumpers that need it...
 */
void fdja_to_j(FILE *f, fdja_value *v, size_t depth);
void fdja_to_d(FILE *f, fdja_value *v, int flags, size_t depth);

char *fdja_to_djan(fdja_value *v, int flags);
#define fdja_tod(v) fdja_to_djan(v, 2)
#define fdja_todc(v) fdja_to_djan(v, 1)
char *fdja_f_todc(const char *path, ...);

#define fdja_putj(v) flu_putf(fdja_to_json(v));
#define fdja_putd(v) flu_putf(fdja_to_djan(v, 2 | 16));
#define fdja_putdc(v) flu_putf(fdja_to_djan(v, 1 | 16));

//char *fdja_to_radial(fdja_value *v);

/* Frees the fdja_value resources. If the fdja_value has children, they
 * will be freed as well.
 */
void fdja_value_free(fdja_value *v);
#define fdja_free(v) fdja_value_free(v)

/* Returns a copy of the string representation of the fdja_value.
 */
char *fdja_string(fdja_value *v);

/* Returns a copy of the string representation *content* of the fdja_value.
 * Mostly like fdja_string(), but for strings returns what's inside of the
 * surrounding double or single quotes.
 */
char *fdja_str(fdja_value *v);

/* Returns a pointer to the value start position in the source string.
 */
char *fdja_src(fdja_value *v);

/* Like fdja_src(), but skips the initial " or ' if present.
 */
char *fdja_srk(fdja_value *v);

/* Returns 0 if v holds a string or a symbol and the text matches s.
 * Returns what strcmp() would return else.
 *
 * Warning: the comparison is performed on the escaped version of the string!
 */
int fdja_strcmp(fdja_value *v, const char *s);

/* Like fdja_strcmp(), but restricts itself to the first n chars.
 * If n < 0, behaves like fdja_strcmp().
 */
int fdja_strncmp(fdja_value *v, const char *s, ssize_t n);

/* Returns the string value behind the fdja_value. For a string fdja_value,
 * the enclosing double quotes will not be included and the string will be
 * unescaped.
 */
char *fdja_to_string(fdja_value *v);

/* Returns a string representation of the value.
 */
char *fdja_value_to_s(fdja_value *v);

long long fdja_to_int(fdja_value *v);
long double fdja_to_double(fdja_value *v);

size_t fdja_size(fdja_value *v);

/* Returns 0 if the values are the same. Returns 0 if the values have
 * the same JSON representation.
 *
 * Returns strcmp(to_json(a), to_json(b)) else.
 *
 * Returns 0 if the two pointers hold NULL, returns -1 if one of the pointers
 * hold NULL.
 */
int fdja_cmp(fdja_value *a, fdja_value *b);

fdja_value *fdja_value_at(fdja_value *v, long long n);
#define fdja_at(v, n) fdja_value_at(v, n)

#define fdja_atc(v, n) fdja_clone(fdja_value_at(v, n))

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

ssize_t fdja_lookup_size(fdja_value *v, const char *path, ...);

#define fdja_l(...) fdja_lookup(__VA_ARGS__)
#define fdja_lc(...) fdja_lookup_c(__VA_ARGS__)
#define fdja_ls(...) fdja_lookup_string(__VA_ARGS__)
#define fdja_lsd(...) fdja_lookup_string_dup_default(__VA_ARGS__)
#define fdja_li(...) fdja_lookup_int(__VA_ARGS__)
#define fdja_lb(...) fdja_lookup_bool(__VA_ARGS__)
#define fdja_lk(...) *fdja_srk(fdja_lookup(__VA_ARGS__))
#define fdja_lz(...) fdja_lookup_size(__VA_ARGS__)

char *fdja_lj(fdja_value *v, const char *path, ...);
char *fdja_ld(fdja_value *v, const char *path, ...);

/* Adds a value at the end of the array.
 * Returns that value.
 */
fdja_value *fdja_push(fdja_value *array, fdja_value *v);

/* Adds a value at the beginning of the array.
 * Returns that value.
 */
fdja_value *fdja_unshift(fdja_value *array, fdja_value *v);

/* Combines val into a fdja_value then removes its first occurence
 * from the array.
 * Returns 1 if an occurence was found (and removed), 0 else.
 * Returns -1 if it couldn't turn val, ... into a fdja_value.
 */
int fdja_unpush(fdja_value *array, const char* val, ...);

/* Sets a value in an object.
 * If the key is prefixed with \b (backslash b), the entry is placed
 * at the beginning of the object.
 * If the value is NULL, the keyed entry gets removed.
 *
 * The last argument must be a fdja_value *.
 *
 * Returns the value just set.
 */
fdja_value *fdja_set(fdja_value *object, const char *key, ...);

/* Like fdja_set() but assumes keys are in alphabetical order. Will thus
 * insert its entry right before the first key that is "bigger" than the
 * inserted key.
 */
fdja_value *fdja_oset(fdja_value *object, const char *key, ...);

int fdja_merge(fdja_value *dst, fdja_value *src);

int fdja_splice(fdja_value *array, long long start, size_t count, ...);

/* Last arg is a fdja_value *.
 *
 * Returns the value just set.
 */
fdja_value *fdja_pset(fdja_value *start, const char *path, ...);
  // pset(v, "cars.-1", v1) to push in cars array
  // pset(v, "cars.%i", -1, v1) too
  // pset(v, "cars.1.\bkey", val) to place entry at beginning of object

/* Last arguments are passed to fdja_v().
 *
 * Returns the value just set.
 */
fdja_value *fdja_psetv(fdja_value *start, const char *path, ...);

/* Empties old and replaces its content with the content of new.
 * The key and the child/sibling links of old are preserved.
 *
 * New gets discarded (freed) in the end.
 */
void fdja_replace(fdja_value *old, fdja_value *new);

#endif // FLON_DJAN_H

//commit d6e7a9a20c829c37fbb15fd43b6d9d3089f66212
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Tue Feb 10 14:06:39 2015 +0900
//
//    implement fdja_parse_r()
