
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

      result = hlp_wait(exid, "terminated", NULL, 7);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //puts(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f "{ ret: [] }");
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

    it "iterates over an object"
    it "iterates over $(f.ret) by default"
    it "iterates and maps to a callable"
  }
}

