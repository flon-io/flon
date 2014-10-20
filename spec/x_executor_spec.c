
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
    int r;

    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");
  }

  describe "flon_execute()"
  {
    it "executes an invocation"
    {
      char *exid = flon_generate_exid("xtest.i");

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "execute: [ invoke, { _0: stamp, color: blue }, [] ]\n"
        "exid: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      expect(flu_fstat("var/spool/exe/exe_%s.json", exid) == 0);
      expect(flu_fstat("var/spool/processed/exe_%s.json", exid) == 'f');

      expect(flu_fstat("var/spool/inv/inv_%s-0.json", exid) == 'f');

      fdja_value *v = fdja_parse_f("var/spool/inv/inv_%s-0.json", exid);

      expect(v != NULL);
      expect(fdja_ls(v, "exid", NULL) ===f exid);
      expect(fdja_ls(v, "nid", NULL) ===f "0");
      expect(fdja_ls(v, "payload.hello", NULL) ===f "world");
      expect(fdja_ls(v, "payload.args.color", NULL) ===f "blue");
      fdja_free(v);

      //puts(flu_readall("var/run/%s.json", exid));
      v = fdja_parse_f("var/run/%s.json", exid);

      expect(v != NULL);
      expect(fdja_ls(v, "exid", NULL) ===f exid);

      expect(fdja_to_json(fdja_l(v, "nodes.0.tree")) ===f ""
        "[\"invoke\",{\"_0\":\"stamp\",\"color\":\"blue\"},[]]");

      fdja_free(v);

      // over.

      free(exid);
    }

    it "executes an invocation return"
    {
      // at first let's start an execution, with an invocation

      char *exid = flon_generate_exid("xtest.ir");

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "execute: [ invoke, { _0: stamp, color: blue }, [] ]\n"
        "exid: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      // let's manually return to the execution

      expect(flu_unlink("var/spool/inv/inv_%s-0.json", exid) == 0);

      flu_writeall(
        "var/spool/exe/ret_%s-0.json", exid,
        "receive: 1\n"
        "exid: %s\n"
        "nid: 0\n"
        "payload: {\n"
          "hello: hiroshima\n"
        "}\n",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      expect(flu_fstat("var/spool/exe/%s-0.json", exid) == 0);
      expect(flu_fstat("var/run/%s.json", exid) == 0);
      expect(flu_fstat("var/run/processed/%s.json", exid) == 'f');

      // TODO
      expect(1 == 0); // need to check for "hello: hiroshima"
    }
  }
}

