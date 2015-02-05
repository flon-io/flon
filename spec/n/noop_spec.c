
//
// specifying instructions
//
// Fri Feb  6 06:37:34 JST 2015
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

  describe "noop"
  {
    it "does nothing (no operation)"
    {
      exid = flon_generate_exid("n.noop");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace a\n"
        "  noop\n"
        "  trace c\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //puts(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f "{ trace: [ a, c ] }");
    }
  }
}

