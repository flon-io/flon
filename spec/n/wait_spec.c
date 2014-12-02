
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
    hlp_reset_tst();
    //hlp_sighup_dispatcher(); //TODO

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

      result = hlp_wait(exid, "terminated", NULL, 3);

      flon_pp_execution(exid);

      expect(result != NULL);
    }
  }
}

