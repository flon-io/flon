
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

#ifndef FL_IDS_H
#define FL_IDS_H

#include "djan.h"


char *flon_generate_exid(const char *domain);

fdja_value *flon_parse_nid(const char *s);

char *flon_get_exid(const char *s);
char *flon_get_nid(const char *s);

char *flon_exid_domain(const char *exid);

char *flon_nid_next(const char *nid, int increment);
char *flon_nid_parent(const char *nid, int chop);
char *flon_nid_child(const char *nid, int n);
size_t flon_nid_depth(const char *nid);
size_t flon_nid_index(const char *nid);

//void flon_stamp(fdja_value *o, const char *key);

char *flon_point_to_prefix(const char *point);

/* Returns 1 if the receiver's id is "natural" parent id for the emitter id.
 */
int flon_is_plain_receive(fdja_value *msg);

#endif // FL_IDS_H

