
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

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "execute: [ invoke, { _0: stamp, color: blue }, [] ]\n"
        "exid: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        exid
      );

      int r = flon_execute(exid);

      expect(r == 0);

      expect(flu_fstat("var/spool/exe/exe_%s.json", exid) == 0);
      expect(flu_fstat("var/spool/processed/exe_%s.json", exid) == 'f');

      expect(flu_fstat("var/spool/inv/inv_%s-0-0.json", exid) == 'f');

      //puts(flu_readall(inv_path));

      fdja_value *v = fdja_parse_f("var/spool/inv/inv_%s-0-0.json", exid);

      expect(v != NULL);
      expect(fdja_ls(v, "exid", NULL) ===f exid);
      expect(fdja_ls(v, "nid", NULL) ===f "0-0");
      expect(fdja_ls(v, "payload.hello", NULL) ===f "world");
      expect(fdja_ls(v, "payload.args.color", NULL) ===f "blue");
      fdja_free(v);

      v = fdja_parse_f("var/run/%s.json", exid);

      expect(v != NULL);
      expect(fdja_ls(v, "exid", NULL) ===f exid);

      expect(fdja_to_json(fdja_l(v, "trees.original")) ===f ""
        "[\"invoke\",{\"_0\":\"stamp\",\"color\":\"blue\"},[]]");

      fdja_free(v);

      // over.

      free(exid);
    }
  }
}

