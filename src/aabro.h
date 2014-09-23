
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

#ifndef AABRO_H
#define AABRO_H

#include "flutil.h"

//
// abr_parser

typedef struct abr_parser {
  char *id;
  char *name;
  short type;
  char *string;
  ssize_t min; ssize_t max;
  struct abr_parser **children;
} abr_parser;

/* Frees the given parser (and its children parsers).
 */
void abr_parser_free(abr_parser *p);

/* Returns a string representation of the parser (and its children).
 */
char *abr_parser_to_string(abr_parser *p);

/* Returns a string representation of the parser
 * (but doesn't dive into its children).
 */
char *abr_parser_to_s(abr_parser *p);

//
// abr_tree

typedef struct abr_tree {
  char *name;
  short result; // -1 error, 0 failure, 1 success
  size_t offset;
  size_t length;
  char *note; // set in case of error
  abr_parser *parser;
  struct abr_tree *sibling;
  struct abr_tree *child;
} abr_tree;

void abr_tree_free(abr_tree *t);

/* Returns a string representation (JSON) of the abr_tree.
 */
char *abr_tree_to_string(abr_tree *t);

/* Same as abr_tree_to_string(), but successful leaves have their text
 * printed, instead of the "[]" standing for "no children".
 * Useful when debugging a parser.
 */
char *abr_tree_to_string_with_leaves(const char *input, abr_tree *t);

/* Returns a copy of the string behind the abr_tree.
 * Returns an empty string if the tree is not a successful one.
 */
char *abr_tree_string(const char *input, abr_tree *t);

/* Returns a pointer to the beginning of the tree in the input directly.
 * Does not return a new char*.
 * Returns the pointer even if the tree is not a successful one.
 */
char *abr_tree_str(char *input, abr_tree *t);

//
// abr_parser builders
//
// Calling those methods build parsers.
//
// The ellipsis methods (alt, seq) actually expect NULL as their
// last argument to stop iterating (over their arguments).

abr_parser *abr_string(const char *s);
abr_parser *abr_range(const char *range);
abr_parser *abr_rex(const char *s);

abr_parser *abr_rep(abr_parser *p, ssize_t min, ssize_t max);
abr_parser *abr_alt(abr_parser *p, ...);
abr_parser *abr_seq(abr_parser *p, ...);

abr_parser *abr_n_alt(const char *name, abr_parser *p, ...);
abr_parser *abr_n_range(const char *name, const char *range);
abr_parser *abr_n_rex(const char *name, const char *s);

abr_parser *abr_n_rep(const char *name, abr_parser *p, ssize_t min, ssize_t max);
abr_parser *abr_n_seq(const char *name, abr_parser *p, ...);
abr_parser *abr_n_string(const char *name, const char *s);

abr_parser *abr_name(const char *name, abr_parser *p);

abr_parser *abr_n(const char *name);

abr_parser *abr_r(const char *code);
abr_parser *abr_n_r(const char *name, const char *code);
abr_parser *abr_q(const char *code);
abr_parser *abr_n_q(const char *name, const char *code);

//abr_parser *abr_not(abr_parser *p);
//abr_parser *abr_presence(abr_parser *p);
//abr_parser *abr_absence(abr_parser *p);

//
// entry point

/* Parses the input. returns an abr_tree with result 0 if not all the
 * input could be parsed until its end.
 */
abr_tree *abr_parse_all(
  const char *input, size_t offset, abr_parser *p);

/* Parses as much as it can from the given input (starting at offset).
 * The length of the resulting abr_tree can be shorter than the length
 * of the input.
 */
abr_tree *abr_parse(
  const char *input, size_t offset, abr_parser *p);

enum // flags for abr_parse_f
{
  ABR_F_PRUNE  = 1 << 0, // don't prune failed trees, defaults to true
  ABR_F_ALL    = 1 << 1 // parse all, defaults to false
};

/* Parses with a given input, offset and a configuration struct.
 */
abr_tree *abr_parse_f(
  const char *input, size_t offset, abr_parser *p, int flags);

//
// helper functions

/* Given an abr_tree resulting from a parse run, returns the error message
 * or NULL if none.
 */
char *abr_error_message(abr_tree *t);

/* Starting from tree t, returns the first sub-tree that bears the
 * given name.
 */
abr_tree *abr_tree_lookup(abr_tree *t, const char *name);

/* The model for a function that, given a tree, returns an integer.
 *
 * -1: no, don't go on with my children
 *  0: no, but please go on with my children if I have any
 *  1: success (collect me, but not any of my children)
 */
typedef short abr_tree_func(const abr_tree *);

/* Given a tree (starting point) and an abr_tree_func, collects all the
 * [sub-trees] that return 1 when the function is called on them.
 */
flu_list *abr_tree_list(abr_tree *t, abr_tree_func *f);

/* Given a tree (starting point) and an abr_tree_func, collects all the
 * [sub-trees] that have a result to 1 and the given name.
 */
flu_list *abr_tree_list_named(abr_tree *t, const char *name);

/* Like abr_tree_list() but returns directly an array of abr_tree*.
 */
abr_tree **abr_tree_collect(abr_tree *t, abr_tree_func *f);

/* Returns the child at the given index, or NULL if there is none there.
 */
abr_parser *abr_p_child(abr_parser *p, size_t index);

/* Returns the child at the given index, or NULL if there is none there.
 */
abr_tree *abr_t_child(abr_tree *t, size_t index);

#endif // AABRO_H

