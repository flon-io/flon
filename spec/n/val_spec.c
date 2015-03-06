
//
// specifying instructions
//
// Thu Jan 15 13:49:16 JST 2015
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

  describe "val"
  {
    it "sets its value in the field 'ret'"
    {
      exid = flon_generate_exid("n.val");

      hlp_launch(
        exid,
        "val 3\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: 3 }");
    }
  }
}

