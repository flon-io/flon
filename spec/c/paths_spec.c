
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

  describe "flon_exid_path()"
  {
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
    }
  }
}

