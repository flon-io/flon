
//
// specifying flon
//
// Sun Nov 23 07:07:13 JST 2014
//

#include "fl_ids.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "flon and errors"
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

  describe "an execution with an unknown instruction"
  {
    it "stalls"
    {
      exid = flon_generate_exid("z.errors.unkown_inst");

      hlp_launch(
        exid,
        "nada a\n"
        "",
        "{ hello: unknown }");

      result = hlp_wait(exid, "failed", "0", 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //puts(fdja_todc(result));

      v = hlp_read_run_json(exid);
      //flu_putf(fdja_todc(v));

      expect(fdja_ls(v, "nodes.0.status", NULL) ===f ""
        "failed");
      expect(fdja_ls(v, "nodes.0.note", NULL) ===f ""
        "unknown instruction 'nada'");
    }
  }
}

