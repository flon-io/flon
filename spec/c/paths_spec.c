
//
// specifying fl_paths
//
// Fri Nov 28 17:04:56 JST 2014
//

//#include <stdio.h>

#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_common.h"


context "fl_paths:"
{
  before each
  {
    flon_configure_j(fdja_c(
      "unit: {\n"
      "  gid: g0\n"
      "  id: u0\n"
      //"  time: local\n"
      "}\n"
    ));
  }

  describe "flon_path()"
  {
    it "returns the second argument made canonical if it's present"
    {
      expect(flon_path("nada", "surf") ===F flu_canopath("surf"));
    }

    it "returns the reworked first argument if it ends in 'bin'"
    {
      expect(flon_path("../tst/bin/xxx", NULL) ===F flu_canopath("../tst"));
      expect(flon_path("bin/xxx", NULL) ===F flu_canopath("."));
    }

    it "returns canonical('.') else"
    {
      expect(flon_path("x/y/z", NULL) ===F flu_canopath("."));
    }
  }

  describe "flon_exid_path()"
  {
    it "returns NULL if the exid can't be determined"
    {
      expect(flon_exid_path("stuck") == NULL);
    }

    it "returns the path to the given exid / nid"
    {
      expect(flon_exid_path("xtest.pn-u0-20141021.0803.kurukuru") ===f ""
        "xtest.pn/ku/xtest.pn-u0-20141021.0803.kurukuru");

      expect(flon_exid_path("xtest.pn-u0-20141021.0803.karufuru-0_1-f") ===f ""
        "xtest.pn/ka/xtest.pn-u0-20141021.0803.karufuru");

      expect(flon_exid_path("rcv_xtest.pn-u0-20141021.0803.koruluru-0_1-f.json") ===f ""
        "xtest.pn/ko/xtest.pn-u0-20141021.0803.koruluru");
    }
  }

  describe "flon_var_path()"
  {
    before all
    {
      flu_system(
        "mkdir -p var/run/c.test.nada/ro/c.test.nada-u0-20150303.1504.ronada");
    }
    after all
    {
      flu_system(
        "rm -fR var/run/c.test.nada/ro/c.test.nada-u0-20150303.1504.ronada");
    }

    it "returns NULL if the exid can't be determined"
    {
      expect(flon_var_path("stuck") == NULL);
    }

    it "returns var/archive/{fep} by default"
    {
      expect(flon_var_path("c.test.nada-u0-20150303.1504.purenada") ===f ""
        "var/archive/c.test.nada/pu/c.test.nada-u0-20150303.1504.purenada");
    }

    it "returns var/run/{fep} if the execution is live"
    {
      expect(flon_var_path("c.test.nada-u0-20150303.1504.ronada") ===f ""
        "var/run/c.test.nada/ro/c.test.nada-u0-20150303.1504.ronada");
    }
  }

  describe "flon_find_json()"
  {
    before each
    {
      for (size_t i = 0; i < 3; ++i)
      {
        char *dom = flu_sprintf("d%zu.listjsonfiles", i);
        char *exid = flon_generate_exid(dom);
        free(dom);
        char *fep = flon_exid_path(exid);
        flu_system("mkdir -p tmp/%s", fep);
        flu_writeall("tmp/%s/f%zu.json", fep, i, "nada");
        flu_writeall("tmp/%s/f%zub.json", fep, i, "again");
        free(fep);
        free(exid);
      }
    }
    after each
    {
      flu_system("rm -fR tmp/");
    }

    it "lists the json files in a exid path spread"
    {
      flu_list *l = flon_find_json("tmp/");

      expect(l != NULL);
      expect(l->size zu== 6);

      flu_list_isort(l, (int (*)(const void *, const void *))strcmp);

      expect(l->first->item $== "/f0.json");
      expect(l->first->next->item $== "/f0b.json");
      expect(l->first->next->next->item $== "/f1.json");

      flu_list_free_all(l);
    }
  }
}

