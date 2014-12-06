
//
// specifying flon-dispatcher
//
// Tue Dec  2 12:03:36 JST 2014
//

#include "flutil.h"
#include "flutim.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_common.h"
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
    char *ts = NULL;
    fdja_value *v = NULL;
    fdja_value *result = NULL;
  }
  after each
  {
    free(exid);
    free(fep);
    free(ts);
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

      result = hlp_wait(exid, "launched", NULL, 5);
      flu_msleep(140);

      expect(result != NULL);

      //flon_pp_execution(exid);

      v = hlp_read_node(exid, "0");
      //flu_putf(fdja_todc(v));

      expect(v != NULL);

      ts = fdja_ls(v, "timers.0.at", NULL);

      expect(ts != NULL);

      fdja_free(v); v = hlp_read_timer(exid, "0", "at", ts);
      //flu_putf(fdja_todc(v));

      expect(v != NULL);
      expect(fdja_lj(v, "tree0") ===F fdja_vj("[ wait, { _0: 2s }, [ 0 ] ]"));
      expect(fdja_lj(v, "tree1") ===F fdja_vj("[ wait, { _0: 2s }, [ 0 ] ]"));

      fdja_free(result);
      result = hlp_wait(exid, "terminated", NULL, 5);

      //flon_pp_execution(exid);

      expect(result != NULL);

      double d = hlp_determine_delta(exid);

      expect(d > 1.2);
      expect(d < 3.2);

      expect(fdja_lj(result, "payload") ===F fdja_vj("{ hello : wait }"));
    }

    context "when cancelled"
    {
      it "unschedules its timer"
      {
        exid = flon_generate_exid("n.wait.cancel");
        fep = flon_exid_path(exid);

        hlp_launch(
          exid,
          "wait 1h\n"
          "",
          "{ hello: wait.cancel }");

        result = hlp_wait(exid, "launched", NULL, 5);

        //flon_pp_execution(exid);

        expect(result != NULL);

        fdja_value *msg = fdja_v("{ point: cancel }");
        int r = flon_lock_write(msg, "var/spool/dis/can_%s-0.json", exid);
        fdja_free(msg);
        //
        expect(r i== 1);

        fdja_free(result); result = hlp_wait(exid, "terminated", NULL, 5);

        //flon_pp_execution(exid);

        expect(result != NULL);

        expect(flu_pline("ls var/spool/tdis/%s 2>&1", fep) >==f ""
          "No such file or directory");
      }
    }
  }
}

