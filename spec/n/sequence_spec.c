
//
// specifying flon-dispatcher
//
// Fri Oct 24 12:45:24 JST 2014
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

  describe "sequence"
  {
    it "chains two instructions"
    {
      exid = flon_generate_exid("n.sequence.2t");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace a\n"
        "  trace b\n"
        "",
        "{ hello: world }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);

      //flu_putf(fdja_todc(result));

      expect(fdja_ls(result, "point", NULL) ===f "terminated");
      expect(fdja_ls(result, "nid", NULL) ===f "0");

      fdja_value *pl = fdja_l(result, "payload");

      expect(fdja_tod(pl) ===f ""
        "{ hello: world, trace: [ a, b ] }");
    }

    it "runs ok when empty"
    {
      exid = flon_generate_exid("n.sequence.0t");

      hlp_launch(
        exid,
        "sequence\n"
        "",
        "{ hello: emptiness }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);

      expect(fdja_ld(result, "point", NULL) ===f "terminated");
      expect(fdja_ls(result, "nid", NULL) ===f "0");

      fdja_value *pl = fdja_l(result, "payload");

      expect(fdja_tod(pl) ===f ""
        "{ hello: emptiness }");
    }
  }
}

