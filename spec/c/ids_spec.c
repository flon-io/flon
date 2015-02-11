
//
// specifying fl_ids
//
// Mon Oct 20 09:23:20 JST 2014
//

//#include <stdio.h>

//#include "flutil.h"
//#include "djan.h"
#include "fl_ids.h"


context "fl_ids:"
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

    it "parses from uris"
    {
      v = flon_parse_nid(
        "http://x.flon.io/i/executions/org.nada-u0-20141118.1201.latsakocheba");

      expect(fdja_tod(v) ===f ""
        "{"
        " exid: org.nada-u0-20141118.1201.latsakocheba,"
        " domain: org.nada,"
        " feu: u0,"
        " tid: 20141118.1201.latsakocheba,"
        " uri: "
          "http://x.flon.io/i/executions/"
            "org.nada-u0-20141118.1201.latsakocheba "
        "}");
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

  describe "flon_parse_exid()"
  {
    it "extracts an exid from a string"
    {
      expect(flon_parse_exid("0_1_2-ff") == NULL);

      expect(
        flon_parse_exid(
          "xtest.pn-u0-20141021.0803.chatsidiseba-0_1_2-ff") ===f ""
        "xtest.pn-u0-20141021.0803.chatsidiseba");
      expect(
        flon_parse_exid(
          "http://x.flon.io/i/executions/org.nada-u0-20141118.1201.latsakocheba") ===f ""
        "org.nada-u0-20141118.1201.latsakocheba");
    }

    it "extracts an exid from a msg filename"
    {
      expect(
        flon_parse_exid(
          "ret_xtest.pn-u0-20141021.0803.kurukuru-0_1_2-ff.json") ===f ""
        "xtest.pn-u0-20141021.0803.kurukuru");
      expect(
        flon_parse_exid(
          "sch_xtest.pn-u0-20141021.0803.karako-0_1_2.json") ===f ""
        "xtest.pn-u0-20141021.0803.karako");
      expect(
        flon_parse_exid(
          "exe_nada.pn-u0-20141021.0803.karako.jon") ===f ""
        "nada.pn-u0-20141021.0803.karako");
    }

    it "extracts an exid from a schedule msg filename"
    {
      expect(
        flon_parse_exid(
          "at-20141130.105800-dtest.trig-u0-20141207.0156.kagemusha-0_0.json"
        ) ===f ""
        "dtest.trig-u0-20141207.0156.kagemusha");
    }
  }

  describe "flon_nid_depth()"
  {
    it "returns the depth of a nid"
    {
      expect(flon_nid_depth("1") zu== 0);
      expect(flon_nid_depth("1_2") zu== 1);
      expect(flon_nid_depth("1_2_0") zu== 2);
      expect(flon_nid_depth("1_2-f") zu== 1);

      expect(flon_nid_depth("x_nada-u0-20141021.0803.karufuru-0_1-f") zu== 1);
    }

    it "returns 0 for non-nids"
    {
      expect(flon_nid_depth("/") zu== 0);
    }
  }

  describe "flon_nid_child()"
  {
    it "returns the nid for the nth child"
    {
      expect(flon_nid_child("0", 0) ===f "0_0");
      expect(flon_nid_child("0", 2) ===f "0_2");

      expect(flon_nid_child("0_1-7", 2) ===f "0_1_2-7");
    }
  }

  describe "flon_nid_next()"
  {
    it "returns the next nid"
    {
      expect(flon_nid_next("0", 1) == NULL);
      expect(flon_nid_next("0_1", 1) ===f "0_2");
      expect(flon_nid_next("1_2", 1) ===f "1_3");
      expect(flon_nid_next("1_2_3", 1) ===f "1_2_4");

      expect(flon_nid_next("1_2_3-2", 1) ===f "1_2_4-2");

      expect(flon_nid_next("1_2_3-2", -1) ===f "1_2_2-2");
    }
  }

  describe "flon_nid_index()"
  {
    it "returns the last part of the nid (but not the counter)"
    {
      expect(flon_nid_index("0") zu== 0);
      expect(flon_nid_index("12") zu== 18);
      expect(flon_nid_index("0_0") zu== 0);
      expect(flon_nid_index("0_13") zu== 19);
      expect(flon_nid_index("0-7") zu== 0);
      expect(flon_nid_index("0_1-7") zu== 1);
      expect(flon_nid_index("0_1a-7") zu== 26);
    }
  }

  describe "flon_exid_domain()"
  {
    it "returns the domain part of an exid"
    {
      expect(flon_exid_domain("xxx-0_1") ===f "xxx");
    }

    it "returns NULL if it can't find the domain part"
    {
      expect(flon_exid_domain("xxx") == NULL);
    }
  }
}

