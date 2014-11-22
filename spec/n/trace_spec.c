
//
// specifying flon-dispatcher
//
// Wed Oct 29 06:11:29 JST 2014
//

#include "fl_ids.h"
#include "fl_common.h"
#include "n_helpers.h"


context "instruction:"
{
  before all
  {
    chdir("../tst");
    flon_configure(".");

    dispatcher_start();
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

      launch(
        exid,
        "trace a\n"
        "",
        "{ hello: trace }");

      result = ewait(exid, 'r', "0", 3);

      //flon_prettyprint(exid);

      expect(result != NULL);

      //puts(fdja_todc(result));

      expect(fdja_ls(result, "receive", NULL) ===f "1");
      expect(fdja_ls(result, "nid", NULL) ===f "0");
      expect(fdja_ls(result, "from", NULL) ===f "0");

      fdja_value *pl = fdja_l(result, "payload");

      expect(fdja_tod(pl) ===f ""
        "{ hello: trace, trace: [ a ] }");
    }
  }
}

