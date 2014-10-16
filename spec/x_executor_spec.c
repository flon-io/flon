
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
      char *exid = flon_generate_id();
      char *exe_path = flu_sprintf("var/spool/exe/exe_%s.json", exid);

      flu_writeall(
        exe_path,
        "execute: [ invoke, { _0: stamp, color: blue }, [] ]\n"
        "exid: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        exid
      );

      int r = flon_execute(exid);

      expect(r == 0);

      expect(flu_fstat("var/spool/processed/exe_%s.json", exid) == 'f');

      char *inv_path = flu_sprintf("var/spool/inv/inv_%s__0_0.json", exid);

      expect(flu_fstat(inv_path) == 'f');
      // TODO: check payload for 'color: blue'

      char *log_path = flu_sprintf("var/log/exe/%s.json", exid);

      expect(flu_fstat(inv_path) == 'f');
      // TODO: check for logged activity
    }
  }
}

