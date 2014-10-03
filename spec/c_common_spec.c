
//
// specifying flon-invoker
//
// Fri Oct  3 14:21:21 JST 2014
//

#include "fl_common.h"


context "common"
{
  before each
  {
    flon_configure_j(fdja_parse_obj(rdz_strdup(""
      "invoker: {\n"
      "  max_processes: 2\n"
      "  xyz: nada\n"
      "}\n"
    )));
  }

  describe "flon_conf()"
  {
    it "returns a fdja_value"
    {
      expect(flon_conf("invoker.max_processes")->type == 'n');
    }

    it "returns NULL when not found"
    {
      expect(flon_conf("nada") == NULL);
    }
  }
}

