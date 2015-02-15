
//
// specifying instructions
//
// Thu Feb 12 06:09:49 JST 2015
//

#include "flutil.h"
#include "fl_ids.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "instruction:"
{
  before all
  {
    hlp_dispatcher_start();
  }

  before each
  {
    char *exid = NULL;
    fdja_value *result = NULL;
  }
  after each
  {
    free(exid);
    fdja_free(result);
  }

  describe "map"
  {
    context "on array"
    {
      it "fails on a missing array"
      {
        exid = flon_generate_exid("n.map.array.missing");

        hlp_launch(
          exid,
          "map\n"
          "  1 + $(ret)\n"
          "",
          "{}");

        result = hlp_wait(exid, "failed", NULL, 7);

        expect(result != NULL);
        //puts(fdja_todc(result));

        expect(fdja_ls(result, "error.msg", NULL) ^==f ""
          "no values to map from: ");
      }

      it "iterates"
      {
        exid = flon_generate_exid("n.map.array");

        hlp_launch(
          exid,
          "map [ 1 2 3 ]\n"
          "  $(i) + $(ret)\n"
          "",
          "{ i: 4 }");

        result = hlp_wait(exid, "terminated", NULL, 7);
        //flon_pp_execution(exid);

        expect(result != NULL);
        //puts(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ i: 4, ret: [ 5, 6, 7 ] }");
      }

      it "iterates over an $(f.ret) array"
      {
        exid = flon_generate_exid("n.map.array.ret");

        hlp_launch(
          exid,
          "sequence\n"
          "  [ 1 2 3 ]\n"
          "  map\n"
          "    $(i) + $(ret)\n"
          "",
          "{ i: 6 }");

        result = hlp_wait(exid, "terminated", NULL, 7);
        //flon_pp_execution(exid);

        expect(result != NULL);
        //puts(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ i: 6, ret: [ 7, 8, 9 ] }");
      }

      it "iterates and makes v.key and v.index available"
      {
        exid = flon_generate_exid("n.map.array.v.key");

        hlp_launch(
          exid,
          "sequence\n"
          "  [ 1 2 3 ]\n"
          "  map\n"
          "    $(v.key) * $(v.index) * $(ret)\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 7);
        //flon_pp_execution(exid);

        expect(result != NULL);
        //puts(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: [ 0, 2, 12 ] }");
      }

      it "iterates and maps to a callable"
      {
        exid = flon_generate_exid("n.map.array.callable");

        hlp_launch(
          exid,
          "sequence\n"
          "  define adder v.v, v.k, v.i\n"
          "    $(v.v) + $(v.k) + $(v.i)\n"
          "  map [ 1, 2, 3 ] adder\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 7);
        //flon_pp_execution(exid);

        expect(result != NULL);
        //puts(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: [ 1, 4, 7 ] }");
      }
    }

    context "on object"
    {
      it "iterates"
      {
        exid = flon_generate_exid("n.map.object");

        hlp_launch(
          exid,
          "map { a:1, b: 2, c: 3 }\n"
          "  val '$(v.index)-$(v.key):$(ret)'\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 7);
        //flon_pp_execution(exid);

        expect(result != NULL);
        //puts(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: [ 0-a:1, 1-b:2, 2-c:3 ] }");
      }

      it "iterates over an $(f.ret) object"
      it "iterates and maps an object to a callable"
    }
  }
}

