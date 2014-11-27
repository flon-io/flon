
//
// specifying flon-dispatcher
//
// Thu Oct 30 12:39:39 JST 2014
//

#include "flutil.h"
#include "fl_ids.h"
#include "fl_tools.h"
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

  describe "invoke"
  {
    it "invokes an external piece of code"
    {
      exid = flon_generate_exid("n.invoke.main");

      hlp_launch(
        exid,
        "invoke stamp\n"
        "",
        "{ hello: invoke }");

      result = hlp_wait(exid, "receive", "0", 2);

      //flon_pp_execution(exid);
      //hlp_cat_inv_log(exid);

      expect(result != NULL);
      //puts(fdja_todc(result));

      expect(fdja_ls(result, "point", NULL) ===f "receive");
      expect(fdja_ls(result, "nid", NULL) ===f "0");
      expect(fdja_ls(result, "from", NULL) == NULL);

      expect(fdja_ls(result, "payload.hello", NULL) ===f "invoke");
      expect(fdja_ls(result, "payload.stamp", NULL) ^==f "20");
    }

    it "expands its arguments"
    {
      exid = flon_generate_exid("n.invoke.expand");

      hlp_launch(
        exid,
        "invoke copyargs $(air), swiss: $(air.1), luft: $(air.2)\n"
        "",
        "{ air: [ sr, lh, em ] }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flu_putf(hlp_last_msg(exid));
      flon_pp_execution(exid);

      expect(result != NULL);
      puts(fdja_todc(result));

      expect(fdja_lj(result, "payload.args1") ===F fdja_vj(""
        "{ _0: copyargs, _1: [ sr, lh, em ], swiss: sr, luft: lh }"));
    }
  }
}

