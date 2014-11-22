
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

      launch(
        exid,
        "sequence\n"
        "  trace a\n"
        "  trace b\n"
        "",
        "{ hello: world }");

      result = ewait(exid, 'r', "0", 10);

      expect(result != NULL);

      expect(fdja_ls(result, "receive", NULL) ===f "1");
      expect(fdja_ls(result, "nid", NULL) ===f "0");
      expect(fdja_ls(result, "from", NULL) ===f "0_1");

      fdja_value *pl = fdja_l(result, "payload");

      expect(fdja_tod(pl) ===f ""
        "{ hello: world, trace: [ a, b ] }");
    }

    it "runs ok when empty"
    {
      exid = flon_generate_exid("n.sequence.0t");

      launch(
        exid,
        "sequence\n"
        "",
        "{ hello: emptiness }");

      result = ewait(exid, 'r', "0", 10);

      //flon_prettyprint(exid);

      expect(result != NULL);

      expect(fdja_ld(result, "receive", NULL) ===f "1");
      expect(fdja_ls(result, "nid", NULL) ===f "0");
      expect(fdja_ls(result, "from", NULL) ===f "0");

      fdja_value *pl = fdja_l(result, "payload");

      expect(fdja_tod(pl) ===f ""
        "{ hello: emptiness }");
    }
  }
}

