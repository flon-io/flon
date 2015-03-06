
//
// specifying flon
//
// Tue Feb  3 13:24:47 JST 2015
//

#include "fl_ids.h"
#include "feu_helpers.h"


context "flon and misc:"
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

  describe "passing 'vars' at launch"
  {
    it "sets them in the root node"
    {
      exid = flon_generate_exid("z.icall");

      hlp_launch_v(
        exid,
        "sequence\n"
        "  trace $(v.name)\n"
        "",
        "{}",
        "{ name: 'billy the kid' }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ \"billy the kid\" ] }");
    }
  }

  describe "an unknown reference"
  {
    it "is turned into a string"
    {
      exid = flon_generate_exid("z.unknownref");

      hlp_launch(
        exid,
        "nada\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: nada }");
    }
  }

  describe "a lonely value"
  {
    it "is returned"
    {
      exid = flon_generate_exid("z.lonelyval");

      hlp_launch(
        exid,
        "7\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: 7 }");
    }
  }
}

