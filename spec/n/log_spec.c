
//
// specifying instructions
//
// Wed Feb 11 05:53:44 JST 2015
//

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
    char *fep = NULL;
    char *s = NULL;
    fdja_value *result = NULL;
  }
  after each
  {
    free(exid);
    free(fep);
    free(s);
    fdja_free(result);
  }

  describe "log"
  {
    it "logs its attributes"
    {
      exid = flon_generate_exid("n.log");
      fep = flon_exid_path(exid);

      hlp_launch(
        exid,
        "sequence\n"
        "  log i 'something went wrong'\n"
        "  log w 'something went wrong' $(x) $(y)\n"
        "",
        "{ x: 123, y: [ 4, 5, 6 ] }");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      s = flu_readall("var/archive/%s/exe.log", fep);
      //printf("exe.log >>>\n%s\n<<<\n", s);

      expect(s >== " INFO sfeu:2:log \"something went wrong\"\n");
      expect(s >== " WARN sfeu:3:log \"something went wrong\" 123 [ 4, 5, ");
    }
  }
}

