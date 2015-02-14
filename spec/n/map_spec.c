
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
    it "doesn't iterate over an empty array"
    {
      exid = flon_generate_exid("n.map.array.empty");

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

    it "iterates over an array"
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

    it "makes the current array index available via v.key and v.index"
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

    it "iterates over an object"
    it "iterates over an $(f.ret) object"
    it "iterates and maps an array to a callable"
    it "iterates and maps an object to a callable"
  }
}

