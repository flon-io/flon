
//
// specifying flon-invoker
//
// Mon Oct 20 09:23:20 JST 2014
//

//#include <stdiot.h>

#include "flutil.h"
#include "fl_common.h"


context "common: ids"
{
  before each
  {
    flon_configure_j(fdja_c(
      "unit: {\n"
      "  gid: g0\n"
      "  id: u0\n"
      //"  time: local\n"
      "}\n"
      "invoker: {\n"
      "  max_processes: 2\n"
      "  xyz: nada\n"
      "}\n"
      "executor: {\n"
      "  p0: ../tst\n"
      "  p1: /var\n"
      "}\n"
    ));
  }

  describe "flon_generate_exid()"
  {
    it "returns an exid"
    {
      char *i = flon_generate_exid("test");

      //printf("      exid: %s\n", i);

      expect(i != NULL);
      expect(i ^== "test-g0.u0-20");

      free(i);
    }
  }
}

