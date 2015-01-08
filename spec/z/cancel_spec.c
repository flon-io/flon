
//
// specifying flon
//
// Sat Dec  6 10:19:55 JST 2014
//

#include "flutil.h"
#include "fl_ids.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "flon and cancel:"
{
  before all
  {
    hlp_dispatcher_start();
  }

  before each
  {
    char *exid = NULL;
    fdja_value *res = NULL;
    fdja_value *v = NULL;
  }
  after each
  {
    free(exid);
    fdja_free(res);
    fdja_free(v);
  }

  describe "an execution"
  {
    it "may be cancelled"
    {
      exid = flon_generate_exid("z.cancel.0");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace x\n"
        "  wait 1h\n"
        "  trace y\n"
        "",
        "{ hello: cancel.0 }");

      res = hlp_wait(exid, "execute", "0_1", 4);

      //flon_pp_execution(exid);

      expect(res != NULL);
      //flu_putf(fdja_todc(res));

      expect(fdja_lj(res, "payload.trace") ===F fdja_vj("[ x ]"));

      fdja_free(res);

      hlp_cancel(exid, NULL);

      res = hlp_wait(exid, "terminated", NULL, 4);

      //flon_pp_execution(exid);

      expect(res != NULL);
      //flu_putf(fdja_todc(res));

      v = hlp_read_run_json(exid);
      expect(v == NULL);

      v = hlp_read_archive_run_json(exid);
      expect(v != NULL);
      //flu_putf(fdja_todc(v));

      fdja_value *nodes = fdja_l(v, "nodes");
      expect(fdja_size(nodes) zu== 0);
    }
  }
}

