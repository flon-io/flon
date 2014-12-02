
//
// specifying flon-dispatcher
//
// Tue Dec  2 12:03:36 JST 2014
//

#include "flutil.h"
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

  describe "wait"
  {
    it "waits for the given time duration"
    {
      exid = flon_generate_exid("n.wait.0");

      hlp_launch(
        exid,
        "wait 2s\n"
        "",
        "{ hello: wait }");

      //flon_pp_execution(exid);

      result = hlp_wait(exid, "terminated", NULL, 1);

      flon_pp_execution(exid);

      expect(result != NULL);

      //hlp_sighup_dispatcher();
        // idea...
    }
  }
}

//    it "sets a field"
//    {
//      exid = flon_generate_exid("n.set.0");
//
//      hlp_launch(
//        exid,
//        "set f.a: 1\n"
//        "",
//        "{ hello: set }");
//
//      result = hlp_wait(exid, "terminated", NULL, 1);
//
//      //flon_pp_execution(exid);
//
//      expect(result != NULL);
//      //flu_putf(fdja_todc(result));
//
//      expect(fdja_lj(result, "payload") ===F fdja_vj(""
//        "{ hello: set, a: 1 }"));
//    }

