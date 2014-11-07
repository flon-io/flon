
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

// https://github.com/flon-io/gajeta

// gajeta.h

#ifndef FLON_GAJETA_H
#define FLON_GAJETA_H

#include <stdio.h>
#include <stdarg.h>

#include "flutil.h"


#define FGAJ_VERSION "1.0.0"

// 10 't' trace
// 20 'd' debug
// 30 'i' info
// 40 'w' warn
// 50 'e' error

/*
 * Logger function type.
 */
typedef void fgaj_logger(char level, const char *subject, const char *msg);


//
// configuration

typedef struct fgaj_conf {
  char color;  // 't', 'T', 'f' for true, True and false respectively
  char level;  // defaults to 30 (info)
  short utc;   // 1 = true, defaults to 0
  char *host;  // defaults to result of gethostname()
  fgaj_logger *logger;  // logger function
  void *out;            // logging destination
  void *params;         // whatever suits the logger func
} fgaj_conf;

/* Returns the configuration global var.
 */
fgaj_conf *fgaj_conf_get();

/* Resets the logger. Used for testing.
 */
void fgaj_conf_reset();

/* Reads the env and sets the configuration accordingly.
 * Is called behind the scene, but is available here for cases when
 * one wants to do custom configuration and then give the env the
 * last word (by calling this method).
 */
void fgaj_read_env();


//
// loggers

/* Default logger function.
 */
void fgaj_color_file_logger(
  char level, const char *subject, const char *msg);

/* Simple logger function. Used for testing. Logs to a char[] placed in
 * fgaj_conf_get()->out
 */
void fgaj_string_logger(
  char level, const char *subject, const char *msg);

/* Logger for specs, logging in grey, towards the right side of the screen.
 */
void fgaj_grey_logger(
  char level, const char *subject, const char *msg);


//
// logging functions (and macros)

/* A raw logging method, accepts the level as a char ('e', 'w', 'd', ...) or
 * as a number (10, 20, ...) and the pref / msg.
 *
 * It accepts 'r' as a [virtual] level as well (like 'e' but appends the
 * result of strerror(errno) to the message)
 */
void fgaj_log(
  char level,
  const char *file, int line, const char *func,
  const char *format, ...);

// the ellipsis in there cover "format and ellipsis"...

#define fgaj_t(...) fgaj_log('t', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define fgaj_d(...) fgaj_log('d', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define fgaj_i(...) fgaj_log('i', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define fgaj_w(...) fgaj_log('w', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define fgaj_e(...) fgaj_log('e', __FILE__, __LINE__, __func__, __VA_ARGS__)

#define fgaj_r(...) fgaj_log('r', __FILE__, __LINE__, __func__, __VA_ARGS__)

#define fgaj_l(level, ...) \
  fgaj_log(level, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define fgaj_ll(level, subject, ...) \
  fgaj_log(level, subject, -1, NULL, __VA_ARGS__)

//
// sometimes one wants to log an error at the trace level...

void fgaj_rlog(
  char level, short err,
  const char *file, int line, const char *func,
  const char *format, ...);

#define fgaj_tr(...) \
  fgaj_rlog('t', 1, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define fgaj_dr(...) \
  fgaj_rlog('d', 1, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define fgaj_ir(...) \
  fgaj_rlog('i', 1, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define fgaj_wr(...) \
  fgaj_rlog('w', 1, __FILE__, __LINE__, __func__, __VA_ARGS__)


//
// helper functions
//
// they are only relevant for people implementing logger functions.

/* Turns a level char like 'i', 'W', 20 to a string like "INFO", "WARN",
 * "DEBUG", respectively.
 */
char *fgaj_level_to_string(char level);

/* Use this function to free the string obtained from fgaj_level_to_string().
 * This function only frees if the result is from the heap.
 */
void fgaj_level_string_free(char *s);

/* Turns a level char like 'i', 'W', ... into a numeric level like 30, 40, ...
 */
char fgaj_normalize_level(char level);

/* Turns a string like 'i', 'info', 'INFO', '30', ... into a numeric level
 * like 30, 40, ...
 */
char fgaj_parse_level(char *s);

/* Returns a (malloc'ed) string detailing the current time.
 */
char *fgaj_now();

#endif // FLON_GAJETA_H

