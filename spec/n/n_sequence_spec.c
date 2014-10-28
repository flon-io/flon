
//
// specifying flon-dispatcher
//
// Fri Oct 24 12:45:24 JST 2014
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
  after all
  {
    dispatcher_stop();
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

  describe "sequence"
  {
    it "chains two instructions"
    {
      exid = flon_generate_exid("n.sequence.2t");

      result = launch(
        exid,
        "sequence\n"
        "  trace a\n"
        "  trace b\n"
        "",
        "{ hello: world }");

      expect(result != NULL);

      expect(fdja_ls(result, "receive", NULL) ===f "1");
      expect(fdja_ls(result, "nid", NULL) ===f "0");
      expect(fdja_ls(result, "from", NULL) ===f "0_1");

      fdja_value *pl = fdja_l(result, "payload");

      expect(fdja_tod(pl) ===f ""
        "{ hello: world, trace: [ a, b ] }");
    }
  }
}

