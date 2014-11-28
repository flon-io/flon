
//
// specifying fl_paths
//
// Fri Nov 28 17:04:56 JST 2014
//

//#include <stdio.h>

//#include "flutil.h"
//#include "djan.h"
#include "fl_paths.h"


context "fl_paths:"
{
  //before each
  //{
  //  flon_configure_j(fdja_c(
  //    "unit: {\n"
  //    "  gid: g0\n"
  //    "  id: u0\n"
  //    //"  time: local\n"
  //    "}\n"
  //    "invoker: {\n"
  //    "  max_processes: 2\n"
  //    "  xyz: nada\n"
  //    "}\n"
  //    "executor: {\n"
  //    "  p0: ../tst\n"
  //    "  p1: /var\n"
  //    "}\n"
  //  ));
  //}

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
}

