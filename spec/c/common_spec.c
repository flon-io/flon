
//
// specifying flon-invoker
//
// Fri Oct  3 14:21:21 JST 2014
//

//#include <stdiot.h>

#include "flutil.h"
#include "fl_common.h"


context "fl_common:"
{
  before each
  {
    flon_configure_j(fdja_c(
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
      expect(flon_conf_path("executor.p9", "/x") ===f "/x");
      expect(flon_conf_path("executor.p9", NULL) === NULL);

      expect(flon_conf_path("executor.p0", NULL) $==f "/flon/tst");
      expect(flon_conf_path("executor.p1", NULL) ===f "/var");
    }
  }

  describe "flon_configure()"
  {
    it "parses from {root}/etc/flon.json"
    {
      flon_configure("../tst");

      expect(flon_conf_path("_root", NULL) $==f "/flon/tst");
    }
  }

  describe "flon_conf_is()"
  {
    it "returns 1 if the desired value matches the actual value"
    {
      expect(flon_conf_is("invoker.xyz", "nada") == 1);
    }

    it "returns 0 else"
    {
      expect(flon_conf_is("invoker.xyz", "nemo") == 0);
    }

    it "turns actual values into strings for the comparison"
    {
      expect(flon_conf_is("invoker.max_processes", "2") == 1);
      expect(flon_conf_is("invoker.max_processes", "20") == 0);
    }

    it "accepts NULL (not set) as a desired value"
    {
      expect(flon_conf_is("executor.p2", "x") == 0);
      expect(flon_conf_is("executor.p2", NULL) == 1);
    }
  }
}

