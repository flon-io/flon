
//
// specifying flon-dispatcher
//
// Tue Dec  2 12:03:36 JST 2014
//

#include "flutil.h"
#include "flutim.h"
#include "fl_ids.h"
#include "fl_paths.h"
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

    char *exid = NULL;
    char *fep = NULL;
    fdja_value *v = NULL;
    fdja_value *result = NULL;
  }
  after each
  {
    free(exid);
    free(fep);
    fdja_free(v);
    fdja_free(result);
  }

  describe "wait"
  {
    it "waits for the given time duration"
    {
      exid = flon_generate_exid("n.wait.0");
      //fep = flon_exid_path(exid);

      hlp_launch(
        exid,
        "wait 2s\n"
        "",
        "{ hello: wait }");

      //flon_pp_execution(exid);

      flu_msleep(250);
        // so that the execution actually starts

      v = hlp_read_node(exid, "0");
      //flu_putf(fdja_todc(v));
      expect(fdja_lj(v, "timers") ===f "true");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);

      double d = hlp_determine_delta(exid);

      expect(d > 1.2);
      expect(d < 3.0);

      expect(fdja_lj(result, "payload") ===F fdja_vj("{ hello : wait }"));
    }

    context "when cancelled"
    {
      it "unschedules its timer"
      {
        exid = flon_generate_exid("n.wait.cancel");

        hlp_launch(
          exid,
          "wait 1h\n"
          "",
          "{ hello: wait.cancel }");

        result = hlp_wait(exid, "scheduled", NULL, 2);

        flon_pp_execution(exid);

        expect(result != NULL);

        // TODO: queue cancel message
      }
    }
  }
}

