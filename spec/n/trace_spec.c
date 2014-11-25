
//
// specifying flon-dispatcher
//
// Wed Oct 29 06:11:29 JST 2014
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
    if (exid) free(exid);
    if (result) fdja_free(result);
  }

  describe "trace"
  {
    it "adds its first argument to a 'trace' array"
    {
      exid = flon_generate_exid("n.trace.firstarg");

      hlp_launch(
        exid,
        "trace a\n"
        "",
        "{ hello: trace }");

      result = hlp_wait(exid, "receive", "0", 3);

      //flon_prettyprint(exid);

      expect(result != NULL);

      //puts(fdja_todc(result));

      expect(fdja_ls(result, "point", NULL) ===f "receive");
      expect(fdja_ls(result, "nid", NULL) ===f "0");
      expect(fdja_ls(result, "from", NULL) ===f "0");

      fdja_value *pl = fdja_l(result, "payload");

      expect(fdja_tod(pl) ===f ""
        "{ hello: trace, trace: [ a ] }");
    }
  }
}

