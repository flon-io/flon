
//
// specifying instructions
//
// Wed Jan  7 21:33:28 JST 2015
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
    it "sets args, nid and counter in a variable"
    {
      exid = flon_generate_exid("n.define.vanilla");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub a0, a1\n"
        "    trace a\n"
        "    trace b\n"
        "  stall\n"
        "",
        "{}");

      result = hlp_wait(exid, "execute", "0_1", 5);

      expect(result != NULL);
      //fdja_putdc(result);

      v = hlp_read_run_json(exid);
      //fdja_putdc(v);

      expect(v != NULL);

      fdja_value *v1 = fdja_l(v, "nodes.0.vars");
      expect(fdja_size(v1) zu== 1);
      //fdja_putdc(v);

      v1 = fdja_l(v, "nodes.0.vars.sub.args");
      expect(fdja_tod(v1) ===f "[ a0, a1 ]");

      v1 = fdja_l(v, "nodes.0.vars.sub.nid");
      expect(fdja_tod(v1) ===f "0_0");
    }

    it "is OK when there are no args"
    {
      exid = flon_generate_exid("n.define.noargs");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub\n"
        "    trace a\n"
        "  stall\n"
        "",
        "{}");

      result = hlp_wait(exid, "execute", "0_1", 5);

      expect(result != NULL);
      //fdja_putdc(result);

      v = hlp_read_run_json(exid);
      //fdja_putdc(v);

      expect(v != NULL);

      fdja_value *v1 = fdja_l(v, "nodes.0.vars.sub.args");
      expect(fdja_tod(v1) ===f "[]");

      v1 = fdja_l(v, "nodes.0.vars.sub.nid");
      expect(fdja_tod(v1) ===f "0_0");
    }

    it "is OK when the name and args are extrapolated"
    {
      exid = flon_generate_exid("n.define.extrapo");

      hlp_launch(
        exid,
        "sequence\n"
        "  define $(sname) $(aname)\n"
        "    trace a\n"
        "  stall\n"
        "",
        "{ sname: sub0, aname: arg0 }");

      result = hlp_wait(exid, "execute", "0_1", 7);

      expect(result != NULL);
      //fdja_putdc(result);

      v = hlp_read_run_json(exid);
      //fdja_putdc(v);

      expect(v != NULL);

      fdja_value *v1 = fdja_l(v, "nodes.0.vars.sub0.args");
      expect(fdja_tod(v1) ===f "[ arg0 ]");

      v1 = fdja_l(v, "nodes.0.vars.sub0.nid");
      expect(fdja_tod(v1) ===f "0_0");
    }

    //it "returns 'anonymous functions'"
    //{
    //  exid = flon_generate_exid("n.define.anon");
    //  hlp_launch(
    //    exid,
    //    "sequence\n"
    //    "  define null bravo charly\n"
    //    "    trace a\n"
    //    //"  stall\n"
    //    "",
    //    "{}");
    //  result = hlp_wait(exid, "terminated", NULL, 7);
    //  expect(result != NULL); //fdja_putdc(result);
    //}
  }
}

