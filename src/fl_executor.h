
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

// for specs

#ifndef FL_EXECUTOR_H
#define FL_EXECUTOR_H

#include "aabro.h"
#include "djan.h"


extern char *execution_id;
extern char *execution_path;
extern fdja_value *execution;


void flon_execute(const char *exid);

void flon_queue_msg(
  const char *type, const char *nid, const char *from_nid, fdja_value *m);

void flon_schedule_msg(
  const char *type, const char *ts, const char *nid,
  fdja_value *tree0, fdja_value *tree1,
  fdja_value *msg);

void flon_unschedule_msg(
  const char *type, const char *ts, const char *nid);


fdja_value *flon_execut(
  fdja_value *tree, fdja_value *payload, fdja_value *vars);

//
// instructions

typedef char flon_instruction(fdja_value *, fdja_value *);

int flon_rewrite_tree(fdja_value *node, fdja_value *msg);

char flon_call_instruction(char dir, fdja_value *node, fdja_value *msg);
  //
  // return codes:
  //
  // 'k' ok
  // 'v' over, reply to parent
  // 'r' error

//
// nodes

fdja_value *flon_node(const char *nid);
fdja_value *flon_node_tree(const char *nid);
fdja_value *flon_node_tree_clone(const char *nid);

char *flon_parent_nid(const char *nid);

#endif // FL_EXECUTOR_H

