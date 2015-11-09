
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

// https://github.com/flon-io/aabro

#ifndef FLON_AABRO_H
#define FLON_AABRO_H

#include "flutil.h"



#define FABR_VERSION "2.0.0"

//
// fabr_tree

typedef struct fabr_tree {
  char *name;
  short result; // -1 error, 0 nomatch, 1 success
  size_t offset;
  size_t length;
  char *note; // set in case of error
  char *parter; // "str-01" for example
  size_t rexlen;
  struct fabr_tree *sibling;
  struct fabr_tree *child;
} fabr_tree;

void fabr_tree_free(fabr_tree *t);

/* Prunes a tree of its 0 (failed) branches. Doesn't prune -1 (error)
 * branches (and of course doesn't prune 1 (success) branches).
 */
void fabr_prune(fabr_tree *t);

/* Returns a string representation (JSON) of the fabr_tree, from t to its
 * leaves. If the input is given, the parsed strings are displayed at
 * the leaves.
 */
char *fabr_tree_to_string(fabr_tree *t, const char *input, short color);

/* Short for `char *st = fabr_tree_to_string(t, i, c); puts(st); free(st);`
 */
void fabr_puts_tree(fabr_tree *t, const char *input, short color);


/* Returns a string representation (JSON) of the fabr_tree.
 * The children are not displayed. If the tree is a leaf and the input
 * is not NULL, the parsed string is displayed, else the children count
 * is displayed.
 */
char *fabr_tree_to_str(fabr_tree *t, const char *input, short color);

/* Like fabr_puts_tree() but the last arg accept flags (1 for colours,
 * 2 for children, so 3 for both).
 */
void fabr_tree_puts(fabr_tree *t, const char *input, short flags);
#define fabr_puts(t, input, flags) fabr_tree_puts(t, input, flags)

/* Returns a copy of the string behind the fabr_tree.
 * Returns an empty string if the tree is not a successful one.
 */
char *fabr_tree_string(const char *input, fabr_tree *t);

/* Returns a pointer to the beginning of the tree in the input directly.
 * Does not return a new char*.
 * Returns the pointer even if the tree is not a successful one.
 */
char *fabr_tree_str(const char *input, fabr_tree *t);

/* Attempts to extract a long long from the pointed at string.
 */
long long fabr_tree_llong(const char *input, fabr_tree *t, int base);

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

/* Like fabr_tree_lookup(), but returns a copy of the resulting tree's
 * string.
 * Returns NULL if it didn't find the subtree.
 */
char *fabr_lookup_string(const char *input, fabr_tree *t, const char *name);

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

/* Like fabr_tree_list() but doesn't consider the root of t.
 */
flu_list *fabr_tree_list_cn(fabr_tree *t, fabr_tree_func *f);

/* Given a tree (starting point) and a fabr_tree_func, collects all the
 * [sub-trees] that have a result to 1 and the given name.
 */
flu_list *fabr_tree_list_named(fabr_tree *t, const char *name);

/* Like fabr_tree_list_named() but doesn't consider the root of t.
 */
flu_list *fabr_tree_list_named_cn(fabr_tree *t, const char *name);

/* Like fabr_tree_list() but returns directly an array of fabr_tree*.
 */
fabr_tree **fabr_tree_collect(fabr_tree *t, fabr_tree_func *f);

/* Returns the child at the given index, or NULL if there is none there.
 */
fabr_tree *fabr_t_child(fabr_tree *t, size_t index);

/* fabr_t_path(t, 0, 1, -1); returns the second subtree of the first subtree
 * of t.
 */
fabr_tree *fabr_t_path(fabr_tree *t, size_t index, ...);


//
// fabr_input

typedef struct {
  char *string;
  size_t offset;
  int flags;
} fabr_input;


//
// parsers

typedef fabr_tree *fabr_parser(fabr_input *);

//
// parters (partial parsers)

fabr_tree *fabr_str(
  char *name, fabr_input *i, char *str);

fabr_tree *fabr_seq(
  char *name, fabr_input *i, fabr_parser *p, ...);

fabr_tree *fabr_altgr(
  char *name, fabr_input *i, short greedy, fabr_parser *p, ...);
#define fabr_altg(name, i, p, ...) \
  fabr_altgr(name, i, 1, p, __VA_ARGS__)
#define fabr_alt(name, i, p, ...) \
  fabr_altgr(name, i, 0, p, __VA_ARGS__)

fabr_tree *fabr_rep(
  char *name, fabr_input *i, fabr_parser *p, size_t min, size_t max);

fabr_tree *fabr_rng(
  char *name, fabr_input *i, char *range);

fabr_tree *fabr_rex(
  char *name, fabr_input *i, char *rex);

fabr_tree *fabr_eseq(
  char *name, fabr_input *i,
  fabr_parser *startp, fabr_parser *eltp, fabr_parser *sepp, fabr_parser *endp);

#define fabr_jseq(name, i, eltp, sepp) \
  fabr_eseq(name, i, NULL, eltp, sepp, NULL)

fabr_tree *fabr_rename(
  char *name, fabr_input *i, fabr_parser *p);

fabr_tree *fabr_eos(char *name, fabr_input *i);

fabr_tree *fabr_all(
  char *name, fabr_input *i, fabr_parser *p);

fabr_tree *fabr_qmark(fabr_input *);
fabr_tree *fabr_star(fabr_input *);
fabr_tree *fabr_plus(fabr_input *);


//
// helpers

fabr_tree *fabr_parse(const char *input, fabr_parser *p);
fabr_tree *fabr_parse_all(const char *input, fabr_parser *p);

enum // flags for fabr_parse_f
{
  FABR_F_PRUNE  = 1 << 0, // don't prune failed trees, defaults to true
  FABR_F_ALL    = 1 << 1, // parse all, defaults to false
  FABR_F_MATCH  = 1 << 2  // prune everything, leave only root and result
};

fabr_tree *fabr_parse_f(const char *input, fabr_parser *p, int flags);

/* Returns 1 if the all the input is matched by the parser.
 * Returns 0 if the match fails, returns -1 in case of error.
 */
int fabr_match(const char *input, fabr_parser *p);

#endif // FLON_AABRO_H

//commit 7198da86d267f621942f9bbcbcc3bbc24df1497b
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Wed Aug 26 06:38:57 2015 +0900
//
//    implement fabr_tree_llong()
