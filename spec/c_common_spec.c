
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
    flon_configure_j(fdja_o(""
      "unit: {\n"
      "  id: u96\n"
      "  group_id: g7\n"
      "}\n"
      "\n"
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

  describe "flon_conf_path()"
  {
    it "returns an absolute path"
    {
      expect(flon_conf_path("executor.p9", "x") $==f "/tmp/x");
      expect(flon_conf_path("executor.p9", "/x") === "/x");
      expect(flon_conf_path("executor.p9", NULL) === NULL);

      expect(flon_conf_path("executor.p0", NULL) $==f "/tmp/../tst");
      expect(flon_conf_path("executor.p1", NULL) ===f "/var");
    }
  }

  describe "flon_generate_id()"
  {
    it "returns an id"
    {
      char *id = flon_generate_id();

      //printf("id: >%s<\n", id);
      expect(id != NULL);
      expect(id ^== "u96_g7_");

      free(id);
    }
  }
}

