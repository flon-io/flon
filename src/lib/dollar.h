
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

// https://github.com/flon-io/dollar

#ifndef FLON_DOLLAR_H
#define FLON_DOLLAR_H


typedef char *fdol_lookup(void *data, const char *path);

/* Given a string, a lookup function and some data understood by that
 * function, expands the $(...) in the string and returns a new string.
 */
char *fdol_expand(const char *s, void *data, fdol_lookup *func);

/* Like fdol_expand() but makes sure that every expansion at the first
 * level is wrapped in double quotes.
 * Useful when preparing command invocations.
 */
char *fdol_quote_expand(const char *s, void *data, fdol_lookup *func);

/* Expects a flu_dict in its data argument. Returns the looked up
 * char * value (or NULL).
 */
char *fdol_dlup(void *data, const char *path);

#endif // FLON_DOLLAR_H

//commit 647ebfb04cb9f4a4ebfe2073ec7c6bb6a7228b3c
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Fri Jan 9 14:37:59 2015 +0900
//
//    upgrade dependencies
