
//
// specifying flon
//
// Tue Nov 25 21:43:41 JST 2014
//

#include "flutil.h"
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
  }
  after each
  {
    free(exid);
    if (result) fdja_free(result);
  }

  describe "an execution"
  {
    it "emits a 'launched' event upon starting"
    {
      exid = flon_generate_exid("z.events.launched");

      hlp_launch(
        exid,
        "trace x\n"
        "",
        "{ hello: launched }");

      result = hlp_wait(exid, "launched", NULL, 2);

      //flon_pp_execution(exid);

      expect(result != NULL);
      flu_putf(fdja_todc(result));
    }

    it "emits a 'terminated' event when it stops" // OR 'ceased' ?????????
    {
      exid = flon_generate_exid("z.events.terminated");

      hlp_launch(
        exid,
        "trace x\n"
        "",
        "{ hello: terminated }");

      result = hlp_wait(exid, "terminated", NULL, 2);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ls(result, "point", NULL) ===f ""
        "terminated");
      expect(fdja_lj(result, "payload") ===F fdja_vj(""
        "{ hello: terminated, trace: [ x ] }"));
    }
  }
}

