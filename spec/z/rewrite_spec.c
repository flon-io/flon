
//
// specifying flon
//
// Mon Jan 19 05:59:55 JST 2015
//

#include "fl_ids.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "flon and tree rewrite:"
{
  before all
  {
    hlp_dispatcher_start();
  }

  before each
  {
    char *exid = NULL;
    fdja_value *result = NULL;
    fdja_value *v = NULL;
  }
  after each
  {
    free(exid);
    fdja_free(result);
    fdja_free(v);
  }

  describe "flon"
  {
    it "rewrites and execute  $(a) > 3"
    {
      exid = flon_generate_exid("z.rewrite.cmp");

      hlp_launch(
        exid,
        "$(a) > 3\n"
        "",
        "{ a: 4 }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //puts(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ a: 4, ret: true }");
    }

    it "rewrites and execute  $(a) $(op) 3"

    it "respects the '_' attribute"
    {
      exid = flon_generate_exid("z.rewrite.cmp");

      hlp_launch(
        exid,
        "trace $(nid) or (trace $(nid) or trace $(nid))\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      flon_pp_execution(exid);

      expect(result != NULL);
      //puts(fdja_todc(result));

      expect(fdja_ld(result, "payload.trace") ===f ""
        "[ 0_g, 0_i, 0_j ]");
    }

    it "respects the '_' attribute also when -{counter}"
  }
}

