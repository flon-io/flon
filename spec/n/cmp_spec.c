
//
// specifying instructions
//
// Mon Jan 12 11:41:31 JST 2015
//

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

  describe "cmp"
  {
    it "compares numbers with > (hit)"
    {
      exid = flon_generate_exid("n.cmp.0");

      hlp_launch(
        exid,
        ">\n"
        "  val $(x)\n"
        "  val 3\n"
        "",
        "{ x: 4 }");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ x: 4, ret: true }");
    }

    it "compares numbers with > (miss)"
    {
      exid = flon_generate_exid("n.cmp.1");

      hlp_launch(
        exid,
        ">\n"
        "  val $(x)\n"
        "  val 8\n"
        "",
        "{ x: 7 }");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ x: 7, ret: false }");
    }

    it "accepts a one-liner  > $(a) b"
  }
}

