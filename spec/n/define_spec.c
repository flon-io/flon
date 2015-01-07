
//
// specifying flon-dispatcher
//
// Wed Jan  7 21:33:28 JST 2015
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
    fdja_value *v = NULL;
  }
  after each
  {
    free(exid);
    fdja_free(result);
    fdja_free(v);
  }

  describe "define"
  {
    it "sets its tree in a variable"
    {
      exid = flon_generate_exid("n.define.0");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub\n"
        "    trace b\n"
        "    trace a\n"
        "  error 'stop here'\n"
        "",
        "{}");

      result = hlp_wait(exid, "failed", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);

      //flu_putf(fdja_todc(result));

      v = hlp_read_run_json(exid);
      flu_putf(fdja_todc(v));

      fdja_value *vars = fdja_l(v, "nodes.0.vars");

      expect(fdja_size(vars) zu== 1);
    }
  }
}

