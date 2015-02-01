
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

// https://github.com/flon-io/aabro

#ifndef FLON_AABRO_H
#define FLON_AABRO_H

#include "flutil.h"

//
// fabr_parser

typedef struct fabr_parser {
  char *id;
  char *name;
  short type;
  char *string;
  ssize_t min; ssize_t max;
  struct fabr_parser **children;
} fabr_parser;

/* Frees the given parser (and its children parsers).
 */
void fabr_parser_free(fabr_parser *p);

/* Returns a string representation of the parser (and its children).
 */
char *fabr_parser_to_string(fabr_parser *p);

/* Returns a string representation of the parser
 * (but doesn't dive into its children).
 */
char *fabr_parser_to_s(fabr_parser *p);

//
// fabr_tree

typedef struct fabr_tree {
  char *name;
  short result; // -1 error, 0 failure, 1 success
  size_t offset;
  size_t length;
  char *note; // set in case of error
  fabr_parser *parser;
  struct fabr_tree *sibling;
  struct fabr_tree *child;
} fabr_tree;

void fabr_tree_free(fabr_tree *t);

/* Returns a string representation (JSON) of the fabr_tree, from t to its
 * leaves. If the input is given, the parsed strings are displayed at
 * the leaves.
 */
char *fabr_tree_to_string(fabr_tree *t, const char *input, short color);

/* Returns a string representation (JSON) of the fabr_tree.
 * The children are not displayed. If the tree is a leaf and the input
 * is not NULL, the parsed string is displayed, else the children count
 * is displayed.
 */
char *fabr_tree_to_str(fabr_tree *t, const char *input, short color);

/* Returns a copy of the string behind the fabr_tree.
 * Returns an empty string if the tree is not a successful one.
 */
char *fabr_tree_string(const char *input, fabr_tree *t);

/* Returns a pointer to the beginning of the tree in the input directly.
 * Does not return a new char*.
 * Returns the pointer even if the tree is not a successful one.
 */
char *fabr_tree_str(char *input, fabr_tree *t);

//
// fabr_parser builders
//
// Calling those methods build parsers.
//
// The ellipsis methods (alt, seq) actually expect NULL as their
// last argument to stop iterating (over their arguments).

fabr_parser *fabr_string(const char *s);
fabr_parser *fabr_range(const char *range);
fabr_parser *fabr_rex(const char *s);

fabr_parser *fabr_rep(fabr_parser *p, ssize_t min, ssize_t max);
fabr_parser *fabr_alt(fabr_parser *p, ...);
fabr_parser *fabr_altg(fabr_parser *p, ...);
fabr_parser *fabr_seq(fabr_parser *p, ...);

fabr_parser *fabr_n_alt(const char *name, fabr_parser *p, ...);
fabr_parser *fabr_n_altg(const char *name, fabr_parser *p, ...);
fabr_parser *fabr_n_range(const char *name, const char *range);
fabr_parser *fabr_n_rex(const char *name, const char *s);

fabr_parser *fabr_n_rep(const char *name, fabr_parser *p, ssize_t min, ssize_t max);
fabr_parser *fabr_n_seq(const char *name, fabr_parser *p, ...);
fabr_parser *fabr_n_string(const char *name, const char *s);

fabr_parser *fabr_name(const char *name, fabr_parser *p);

fabr_parser *fabr_n(const char *name);

fabr_parser *fabr_r(const char *code);
fabr_parser *fabr_n_r(const char *name, const char *code);
fabr_parser *fabr_q(const char *code);
fabr_parser *fabr_n_q(const char *name, const char *code);

//fabr_parser *fabr_not(fabr_parser *p);
//fabr_parser *fabr_presence(fabr_parser *p);
//fabr_parser *fabr_absence(fabr_parser *p);

#define fabr_str(s) fabr_string(s)

//
// entry point

/* Parses the input. returns a fabr_tree with result 0 if not all the
 * input could be parsed until its end.
 */
fabr_tree *fabr_parse_all(
  const char *input, size_t offset, fabr_parser *p);

/* Parses as much as it can from the given input (starting at offset).
 * The length of the resulting fabr_tree can be shorter than the length
 * of the input.
 */
fabr_tree *fabr_parse(
  const char *input, size_t offset, fabr_parser *p);

enum // flags for fabr_parse_f
{
  FABR_F_PRUNE  = 1 << 0, // don't prune failed trees, defaults to true
  FABR_F_ALL    = 1 << 1, // parse all, defaults to false
  FABR_F_MATCH  = 1 << 2  // prune everything, leave only root and result
};

/* Parses with a given input, offset and a configuration struct.
 */
fabr_tree *fabr_parse_f(
  const char *input, size_t offset, fabr_parser *p, int flags);

/* Simply responds 1: yes, the input was parsed successfully, 0: no, the
 * input was not parsed successfully.
 */
int fabr_match(const char *input, fabr_parser *p);

//
// helper functions

/* Given a fabr_tree resulting from a parse run, returns the error message
 * or NULL if none.
 */
char *fabr_error_message(fabr_tree *t);

/* Starting from tree t, returns the first sub-tree that bears the
 * given name. Depth first.
 * If the name is NULL, will yield the first sub-tree that bears a name.
 */
fabr_tree *fabr_tree_lookup(fabr_tree *t, const char *name);

/* Starting from tree t's children, returns the first sub-tree that bears
 * the given name. Depth first.
 * If the name is NULL, will yield the first sub-tree that bears a name.
 */
fabr_tree *fabr_subtree_lookup(fabr_tree *t, const char *name);

/* The model for a function that, given a tree, returns an integer.
 *
 * -1: no, don't go on with my children
 *  0: no, but please go on with my children if I have any
 *  1: success (collect me, but not any of my children)
 */
typedef short fabr_tree_func(const fabr_tree *);

/* Given a tree (starting point) and a fabr_tree_func, collects all the
 * [sub-trees] that return 1 when the function is called on them.
 */
flu_list *fabr_tree_list(fabr_tree *t, fabr_tree_func *f);

/* Given a tree (starting point) and a fabr_tree_func, collects all the
 * [sub-trees] that have a result to 1 and the given name.
 */
flu_list *fabr_tree_list_named(fabr_tree *t, const char *name);

/* Like fabr_tree_list() but returns directly an array of fabr_tree*.
 */
fabr_tree **fabr_tree_collect(fabr_tree *t, fabr_tree_func *f);

/* Returns the child at the given index, or NULL if there is none there.
 */
fabr_parser *fabr_p_child(fabr_parser *p, size_t index);

/* Returns the child at the given index, or NULL if there is none there.
 */
fabr_tree *fabr_t_child(fabr_tree *t, size_t index);

#endif // FLON_AABRO_H

