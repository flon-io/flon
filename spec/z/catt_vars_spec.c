
//
// specifying flon
//
// Mon Feb  2 15:19:43 JST 2015
//

#include "fl_ids.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "flon and catts:"
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

  describe "'vars'"
  {
    it "sets local variables"
    {
      exid = flon_generate_exid("z.vars.object");

      hlp_launch(
        exid,
        "sequence vars: { color: red }\n"
        "  trace '$(nid) $(v.color)'\n"
        "  sequence vars: { color: blue }\n"
        "    trace '$(nid) $(v.color)'\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //puts(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ \"0_0 red\", \"0_1_0 blue\" ] }");
    }

    it "sets an empty local scope `vars`"
    {
      exid = flon_generate_exid("z.vars.nothing");

      hlp_launch(
        exid,
        "sequence vars: { color: red }\n"
        "  sequence vars\n"
        "    set v.color: blue\n"
        "    trace '$(nid) $(v.color)'\n"
        "  trace '$(nid) $(v.color)'\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //puts(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ \"0_0_1 blue\", \"0_1 red\" ] }");
    }

    it "sets an empty local scope `vars: true`"
    {
      exid = flon_generate_exid("z.vars.true");

      hlp_launch(
        exid,
        "sequence vars: { color: red }\n"
        "  sequence vars: true\n"
        "    set v.color: blue\n"
        "    trace '$(nid) $(v.color)'\n"
        "  trace '$(nid) $(v.color)'\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //puts(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ \"0_0_1 blue\", \"0_1 red\" ] }");
    }

    it "doesn't overwrite an existing set of 'vars'"
  }
}

