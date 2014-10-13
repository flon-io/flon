
//
// specifying flon-executor
//
// Wed Sep 24 06:20:10 JST 2014
//

#include "flutil.h"
#include "gajeta.h"
#include "fl_common.h"
#include "fl_executor.h"


context "flon-executor"
{
  before each
  {
    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "7p";

    chdir("../tst");
    flon_configure(".");
  }

  describe "flon_execute()"
  {
    it "executes an invocation"
    {
      char *id = flon_generate_id();
      char *path = flu_sprintf("var/spool/exe/exe_%s.json", id);

      flu_writeall(
        path,
        "execute: [ invoke, { _0: stamp, color: blue }, [] ]\n"
        "id: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        id
      );

      int r = flon_execute(path);

      expect(r == 0);
    }
  }
}

