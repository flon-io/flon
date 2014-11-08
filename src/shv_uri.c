
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

// https://github.com/flon-io/shervin

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>

#include "flutil.h"
#include "aabro.h"
#include "shv_protected.h"

//#include "gajeta.h"


fabr_parser *uri_parser = NULL;


static void shv_init_uri_parser()
{
  fabr_parser *scheme =
    fabr_n_rex("scheme", "https?");
  fabr_parser *host =
    fabr_n_rex("host", "[^:/]+");
  fabr_parser *port =
    fabr_seq(fabr_string(":"), fabr_n_rex("port", "[1-9][0-9]+"), NULL);

  fabr_parser *path =
    fabr_n_rex("path", "[^\\?#]+");
  fabr_parser *quentry =
    fabr_n_seq("quentry",
      fabr_n_rex("key", "[^=&#]+"),
      fabr_seq(fabr_string("="), fabr_n_rex("val", "[^&#]+"), fabr_r("?")),
      NULL);
  fabr_parser *query =
    fabr_n_seq("query",
      quentry,
      fabr_seq(fabr_string("&"), quentry, fabr_r("*")),
      NULL);
  fabr_parser *fragment =
    fabr_n_rex("fragment", ".+");

  fabr_parser *shp =
    fabr_seq(
      scheme,
      fabr_string("://"),
      host,
      port, fabr_q("?"),
      NULL);

  uri_parser =
    fabr_seq(
      shp, fabr_q("?"),
      path,
      fabr_seq(fabr_string("?"), query, fabr_r("?")),
      fabr_seq(fabr_string("#"), fragment, fabr_r("?")),
      NULL);
}

flu_dict *shv_parse_uri(char *uri)
{
  if (uri_parser == NULL) shv_init_uri_parser();

  fabr_tree *r = fabr_parse(uri, 0, uri_parser);
  //fabr_tree *r = fabr_parse_f(uri, 0, uri_parser, ABR_F_ALL);
  fabr_tree *t = NULL;

  //printf("uri >%s<\n", uri);
  //puts(fabr_tree_to_string(r, uri, 1));

  flu_dict *d = flu_list_malloc();

  t = fabr_tree_lookup(r, "scheme");
  if (t != NULL) flu_list_set(d, "_scheme", fabr_tree_string(uri, t));
  t = fabr_tree_lookup(r, "host");
  if (t != NULL) flu_list_set(d, "_host", fabr_tree_string(uri, t));
  t = fabr_tree_lookup(r, "port");
  if (t != NULL) flu_list_set(d, "_port", fabr_tree_string(uri, t));

  t = fabr_tree_lookup(r, "path");
  flu_list_set(d, "_path", fabr_tree_string(uri, t));
  //printf("_path >%s<\n", flu_list_get(d, "_path"));

  flu_list *l = fabr_tree_list_named(r, "quentry");
  for (flu_node *n = l->first; n != NULL; n = n->next)
  {
    t = fabr_tree_lookup((fabr_tree *)n->item, "key");
    char *k = fabr_tree_string(uri, t);

    t = fabr_tree_lookup((fabr_tree *)n->item, "val");
    char *v = fabr_tree_string(uri, t);
    char *vv = flu_urldecode(v, -1);

    flu_list_set(d, k, vv);

    free(k); // since flu_list_set() copies it
    free(v);
  }
  flu_list_free(l);

  t = fabr_tree_lookup(r, "fragment");
  if (t != NULL) flu_list_set(d, "_fragment", fabr_tree_string(uri, t));

  fabr_tree_free(r);

  return d;
}

flu_dict *shv_parse_host_and_path(char *host, char *path)
{
  if (host == NULL) return shv_parse_uri(path);

  char *s = NULL;

  if (strncmp(host, "http://", 7) == 0 || strncmp(host, "https://", 8) == 0)
    s = flu_sprintf("%s%s", host, path);
  else
    s = flu_sprintf("http://%s%s", host, path);

  flu_dict *d = shv_parse_uri(s);

  free(s);

  return d;
}

