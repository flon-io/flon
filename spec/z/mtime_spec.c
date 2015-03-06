
//
// specifying flon
//
// Wed Feb 11 14:48:17 JST 2015
//

#include "fl_ids.h"
#include "feu_helpers.h"


context "flon and mtime:"
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

  describe "a node mtime"
  {
    it "is set each time the node is modified"
    {
      exid = flon_generate_exid("z.mtime");

      hlp_launch(
        exid,
        "nada a\n"
        "",
        "{}");

      result = hlp_wait(exid, "failed", "0", 3);

      expect(result != NULL);
      //fdja_putdc(result);

      v = hlp_read_run_json(exid);
      //fdja_putdc(v);

      expect(fdja_ls(v, "nodes.0.mtime", NULL) ^===f "20");
    }
  }
}

