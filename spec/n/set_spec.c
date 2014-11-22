
//
// specifying flon-dispatcher
//
// Sat Nov 22 14:45:56 JST 2014
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

  describe "set"
  {
    it "sets a field"
//    {
//      exid = flon_generate_exid("n.set.0");
//
//      hlp_launch(
//        exid,
//        "set f.a: 1\n"
//        "",
//        "{}");
//
//      result = hlp_wait(exid, 'r', "0", 3);
//
//      //flon_prettyprint(exid);
//
//      expect(result != NULL);
//
//      puts(fdja_todc(result));
//
//      //expect(fdja_ls(result, "receive", NULL) ===f "1");
//      //expect(fdja_ls(result, "nid", NULL) ===f "0");
//      //expect(fdja_ls(result, "from", NULL) ===f "0");
//
//      //fdja_value *pl = fdja_l(result, "payload");
//
//      //expect(fdja_tod(pl) ===f ""
//      //  "{ hello: trace, trace: [ a ] }");
//    }

    it "sets a variable"
  }
}

