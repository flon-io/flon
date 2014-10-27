
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

      expect(fdja_tod(v) ===f ""
        "{ nid: 0_1_2, node: 0_1_2 }");
    }

    it "parses nids with counters"
    {
      v = flon_parse_nid("0_1_2-ff");

      expect(fdja_tod(v) ===f ""
        "{ nid: 0_1_2-ff, node: 0_1_2, counter: ff }");
    }

    it "parses complete nids"
    {
      v = flon_parse_nid("xtest.pn-u0-20141021.0803.chatsidiseba-0_1_2-ff");

      expect(fdja_tod(v) ===f ""
        "{"
        " exid: xtest.pn-u0-20141021.0803.chatsidiseba,"
        " domain: xtest.pn, feu: u0,"
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

      expect(fdja_tod(v) ===f ""
        "{ msg: ret_,"
        " exid: xtest.pn-u0-20141021.0803.kurukuru,"
        " domain: xtest.pn, feu: u0,"
        " tid: 20141021.0803.kurukuru,"
        " nid: 0_1_2-ff, node: 0_1_2, counter: ff,"
        " ftype: .json"
        " }");
    }
  }

  describe "flon_exid_path"
  {
    it "returns the path to the given exid / nid"
    {
      expect(flon_exid_path("xtest.pn-u0-20141021.0803.kurukuru") ===f ""
        "ku/xtest.pn-u0-20141021.0803.kurukuru");

      expect(flon_exid_path("xtest.pn-u0-20141021.0803.karufuru-0_1-f") ===f ""
        "ka/xtest.pn-u0-20141021.0803.karufuru");

      expect(flon_exid_path("rcv_xtest.pn-u0-20141021.0803.koruluru-0_1-f.json") ===f "ko/xtest.pn-u0-20141021.0803.koruluru");
    }
  }
}

