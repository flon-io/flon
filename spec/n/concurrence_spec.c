
//
// specifying instructions
//
// Mon Jan  5 07:18:28 JST 2015
//

#include "fl_ids.h"
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

  describe "concurrence"
  {
    it "chains two instructions"
    {
      exid = flon_generate_exid("n.concurrence.2t");

      hlp_launch(
        exid,
        "concurrence override\n"
        "  trace a\n"
        "  trace b\n"
        "",
        "{ hello: world }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);

      //fdja_putdc(result);

      expect(fdja_ls(result, "point", NULL) ===f "terminated");
      expect(fdja_ls(result, "nid", NULL) ===f "0");

      fdja_value *pl = fdja_l(result, "payload");

      expect(fdja_tod(pl) ===f ""
        "{ hello: world, trace: [ b ] }");
        //"{ hello: world, trace: [ a, b ] }");
    }

    it "runs ok when empty"
    {
      exid = flon_generate_exid("n.concurrence.0t");

      hlp_launch(
        exid,
        "concurrence\n"
        "",
        "{ hello: emptiness }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);

      expect(fdja_ld(result, "point", NULL) ===f "terminated");
      expect(fdja_ls(result, "nid", NULL) ===f "0");

      fdja_value *pl = fdja_l(result, "payload");

      expect(fdja_tod(pl) ===f ""
        "{ hello: emptiness }");
    }
  }
}

