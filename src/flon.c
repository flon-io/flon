
//
// Copyright (c) 2013-2016, John Mettraux, jmettraux+flon@gmail.com
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

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "flutil.h"
#include "flu64.h"
#include "tsifro.h"
#include "fl_ids.h"
#include "fl_flon.h"
#include "fl_paths.h"
#include "fl_scope.h"
#include "fl_common.h"


char *original_dir = NULL;


static int print_usage()
{
  fprintf(stderr, "" "\n");
  fprintf(stderr, "# flon" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  flon is a jack-of-all-trades executable for flon" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  ## exid generation" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "    flon [-d {dir}] exid {domain}" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  Generates an exid." "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  ## password hashing" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "    flon hash {pass} [work_factor]" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  Hashes a password." "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  ## base64 encoding" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "    flon c64 {text}" "\n");
  fprintf(stderr, "    flon d64 {base64_text}" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  Encodes or decodes a text to base 64\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  ## execution listing" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "    flon list [fragment_of_exid]" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  ## execution pretty-printing" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "    flon scope [fragment_of_exid]" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  Pretty prints the first execution whose exid contains\n");
  fprintf(stderr, "  the given fragment.\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  ## launching" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "    flon launch {file.flon}" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  Parses a .flon file and launches execution accordingly.\n");
  fprintf(stderr, "" "\n");

  return 1;
}

static int list(char **args)
{
  flu_list *paths = flon_list_expaths(args[1]);

  for (flu_node *n = paths->first; n; n = n->next)
  {
    printf("* %s\n", (char *)n->item);
  }

  return 0;
}

static int scope(char **args)
{
  char *exid = flon_lookup_exid(args[1]);

  printf("-----------------------------------------------------------------\n");
  printf("fragment  >%s<\n", args[1]);
  printf("exid      >%s<\n", exid);
  printf("--------------------------------------------------- flon scope --\n");

  if (exid == NULL)
  {
    fprintf(stderr, "\n ** no execution matching >%s< **\n\n", args[1]);
    return 1;
  }

  flon_pp_execution(exid);

  return 0;
}

static int exid(char **args)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "\n ** missing domain **\n\n");
    return print_usage();
  }

  flon_configure(".");

  printf("%s\n", flon_generate_exid(args[1]));

  return 0;
}

static int hash(char **args)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "\n** missing password to hash **\n\n");
    return print_usage();
  }

  int work_factor = 9;
  if (args[2]) work_factor = strtol(args[2], NULL, 10);

  char *salt = ftsi_generate_bc_salt(NULL, work_factor);
  if (salt == NULL)
  {
    fprintf(stderr, "\n** failed to generate salt **\n\n");
    return 2;
  }

  printf("%s\n", ftsi_bc_hash(args[1], salt));

  return 0;
}

static int c64(char **args)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "\n** missing text to encode **\n\n");
    return print_usage();
  }

  printf("%s\n", flu64_encode(args[1], -1));

  return 0;
}

static int d64(char **args)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "\n** missing base64 text to decode **\n\n");
    return print_usage();
  }

  printf("%s\n", flu64_decode(args[1], -1));

  return 0;
}

static int launch(char **args)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "\n** missing path to .flon file **\n\n");
    return print_usage();
  }

  flon_configure(".");

  char *path = flu_path("%s/%s", original_dir, args[1]);

  char *exid = flon_launch(path);

  if (exid == NULL)
  {
    fprintf(stderr, "\n** read failure for %s **\n\n", path);
    return 2;
  }

  printf("%s\n", exid);

  free(exid);
  free(path);

  return 0;
}

static int unknown(char **args)
{
  fprintf(stderr, "\n** unknown command '%s' **\n\n", args[0]);
  return print_usage();
}

int main(int argc, char *argv[])
{
  original_dir = flu_canopath(".");

  char *d = NULL;
  char **args = calloc(argc + 1, sizeof(char *));

  for (size_t i = 1, j = 0; i < argc; i++)
  {
    if (strcmp(argv[i], "-d") == 0 && i + 1 < argc)
      d = argv[++i];
    else
      args[j++] = argv[i];
  }

  d = flon_path(argv[0], d);

  if (chdir(d) != 0)
  {
    fprintf(stderr, "\n** couldn't chdir to %s **\n\n", d);
    return 2;
  }

  char *a = args[0];

  if (a == NULL)
  {
    print_usage();
    return 1;
  }

  if (strcmp(a, "exid") == 0)
    return exid(args);
  if (strcmp(a, "hash") == 0 || strcmp(a, "pass") == 0)
    return hash(args);
  if (strcmp(a, "c64") == 0 || strcmp(a, "b64") == 0)
    return c64(args);
  if (strcmp(a, "d64") == 0)
    return d64(args);
  if (strcmp(a, "list") == 0)
    return list(args);
  if (strcmp(a, "scope") == 0)
    return scope(args);
  if (strcmp(a, "launch") == 0)
    return launch(args);

  // else
  return unknown(args);
}

