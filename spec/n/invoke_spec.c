
//
// specifying flon-dispatcher
//
// Thu Oct 30 12:39:39 JST 2014
//

#include "fl_ids.h"
#include "fl_tools.h"
#include "fl_common.h"
#include "n_helpers.h"


context "instruction:"
{
  before all
  {
    chdir("../tst");
    flon_configure(".");

    dispatcher_start();
  }
  after all
  {
    dispatcher_stop();
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

      launch(
        exid,
        "invoke stamp\n"
        "",
        "{ hello: invoke }");

      result = ewait(exid, 'r', "0", 10);

      //flon_prettyprint(exid);

      expect(result != NULL);

      //puts(fdja_todc(result));

      expect(fdja_ls(result, "receive", NULL) ===f "1");
      expect(fdja_ls(result, "nid", NULL) ===f "0");
      expect(fdja_ls(result, "from", NULL) == NULL);

      expect(fdja_ls(result, "payload.hello", NULL) ===f "invoke");
      expect(fdja_ls(result, "payload.stamp", NULL) ^==f "20");
    }
  }
}

