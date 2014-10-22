
//
// specifying fl_id
//
// Mon Oct 20 09:23:20 JST 2014
//

//#include <stdiot.h>

#include "flutil.h"
#include "djan.h"
#include "fl_ids.h"


context "fl_id"
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

  describe "flon_parse_nid()"
  {
    before each
    {
      fdja_value *v = NULL;
    }
    after each
    {
      if (v) fdja_free(v);
    }

    it "parses simple nids"
    {
      v = flon_parse_nid("0_1_2");

      expect(fdja_to_djan(v) ===f ""
        "{ nid: 0_1_2, node: 0_1_2 }");
    }

    it "parses nids with counters"
    {
      v = flon_parse_nid("0_1_2-ff");

      expect(fdja_to_djan(v) ===f ""
        "{ nid: 0_1_2-ff, node: 0_1_2, counter: ff }");
    }

    it "parses complete nids"
    {
      v = flon_parse_nid("xtest.pn-u0-20141021.0803.chatsidiseba-0_1_2-ff");

      expect(fdja_to_djan(v) ===f ""
        "{ domain: xtest.pn, feu: u0,"
        " tid: 20141021.0803.chatsidiseba,"
        " nid: 0_1_2-ff, node: 0_1_2, counter: ff }");
    }

    it "returns NULL if it cannot parse"
    {
      expect(flon_parse_nid("/") == NULL);
    }

    it "accepts filenames as input"
    {
      v = flon_parse_nid(
        "ret_xtest.pn-u0-20141021.0803.kurukuru-0_1_2-ff.json");

      expect(fdja_to_djan(v) ===f ""
        "{ msg: ret_,"
        " domain: xtest.pn, feu: u0,"
        " tid: 20141021.0803.kurukuru,"
        " nid: 0_1_2-ff, node: 0_1_2, counter: ff,"
        " ftype: .json"
        " }");
    }
  }
}

