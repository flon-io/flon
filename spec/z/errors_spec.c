
//
// specifying flon
//
// Sun Nov 23 07:07:13 JST 2014
//

#include "fl_ids.h"
#include "feu_helpers.h"


context "flon and errors"
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
    if (exid) free(exid);
    if (result) fdja_free(result);
  }

  describe "an execution with an unknown instruction"
  {
    it "stalls"
    {
      exid = flon_generate_exid("z.errors.unkown_inst");

      hlp_launch(
        exid,
        "nada a\n"
        "",
        "{ hello: unknown }");

      result = hlp_wait(exid, 'r', "0", 3);

      //flon_prettyprint(exid);

      expect(result != NULL);

      puts(fdja_todc(result));

      //expect(fdja_ls(result, "receive", NULL) ===f "1");
      //expect(fdja_ls(result, "nid", NULL) ===f "0");
      //expect(fdja_ls(result, "from", NULL) ===f "0");

      //fdja_value *pl = fdja_l(result, "payload");

      //expect(fdja_tod(pl) ===f ""
      //  "{ hello: trace, trace: [ a ] }");
    }
  }
}

