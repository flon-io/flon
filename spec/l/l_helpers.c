
//
// helpers for l/ specs
//
// Tue Nov 18 16:40:44 JST 2014
//

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <unistd.h>

#include "flutil.h"
#include "fl_ids.h"
#include "fl_dispatcher.h"
#include "l_helpers.h"


void llog(char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *fo = flu_sprintf("[1;30m.:n:. %s[0;0m\n", format);
  vprintf(fo, ap);
  free(fo);
  va_end(ap);
}

void hlp_clean_tst()
{
  int i = system("make -C .. ctst > /dev/null");
  if (i != 0) printf("... the clean up command failed ...");
}

void hlp_start_execution(char *domain)
{
  char *exid = flon_generate_exid(domain);
  char *name = flu_sprintf("exe_%s.json", exid);

  int r = flu_writeall(
    "var/spool/dis/%s", name,
    "{"
      "execute: [ invoke { _0: null } [] ]\n"
      "exid: %s\n"
      "payload: {\n"
        "hello: %s\n"
      "}\n"
    "}", exid, domain
  );
  if (r != 1) { perror("failed to write exe_ file"); exit(1); }

  r = flon_dispatch(name);
  if (r != 2) { perror("failed to dispatch exe_ file"); exit(2); }

  free(exid);
  free(name);
}

