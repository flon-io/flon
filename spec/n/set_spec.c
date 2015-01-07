
//
// specifying flon-dispatcher
//
// Sat Nov 22 14:45:56 JST 2014
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

  describe "set"
  {
    it "sets a field"
    {
      exid = flon_generate_exid("n.set.fld");

      hlp_launch(
        exid,
        "set f.a: 1\n"
        "",
        "{ hello: set }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_lj(result, "payload") ===F fdja_vj(""
        "{ hello: set, a: 1 }"));
    }

    it "sets a field, by default"
    {
      exid = flon_generate_exid("n.set.fld.default");

      hlp_launch(
        exid,
        "set a: 2\n"
        "",
        "{ hello: set }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_lj(result, "payload") ===F fdja_vj(""
        "{ hello: set, a: 2 }"));
    }

    it "sets a field, via an expanded key"
    {
      exid = flon_generate_exid("n.set.fld.expankey");

      hlp_launch(
        exid,
        "set $(k): 3\n"
        "",
        "{ k: number }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_lj(result, "payload") ===F fdja_vj(""
        "{ k: number, number: 3 }"));
    }

    it "sets a variable"
    {
      exid = flon_generate_exid("n.set.var");

      hlp_launch(
        exid,
        "sequence\n"
        "  set v.a: 3\n"
        "  trace $(v.a)\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_lj(result, "payload") ===F fdja_vj(""
        "{ trace: [ 3 ] }"));
    }
  }
}

