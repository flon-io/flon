
//
// specifying flon-executor
//
// Thu Dec  4 07:33:39 JST 2014
//

#include "flutil.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_tools.h"
#include "fl_common.h"
#include "fl_executor.h"


context "flon-executor"
{
  before each
  {
    int r;

    char *exid = NULL;
    char *fep = NULL;

    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");
  }
  after each
  {
    free(fep);
    free(exid);
  }

  describe "flon_schedule_msg()"
  {
    it "schedules an at msg"
//    {
//      exid = flon_generate_exid("xtest.i");
//      fep = flon_exid_path(exid);
//
//      flu_writeall(
//        "var/spool/exe/exe_%s.json", exid,
//        "{"
//          "point: execute\n"
//          "tree: [ invoke, { _0: stamp, color: blue }, [] ]\n"
//          "exid: %s\n"
//          "payload: {\n"
//            "hello: world\n"
//          "}\n"
//        "}",
//        exid
//      );
//
//      r = flon_execute(exid);
//
//      expect(r i== 0);
//
//      expect(flu_fstat("var/spool/exe/exe_%s.json", exid) == 0);
//      expect(flu_fstat("var/run/%s/processed/exe_%s.json", fep, exid) == 'f');
//
//      expect(flu_fstat("var/spool/dis/inv_%s-0.json", exid) == 'f');
//
//      fdja_value *v = fdja_parse_f("var/spool/dis/inv_%s-0.json", exid);
//
//      expect(v != NULL);
//      expect(fdja_ls(v, "exid", NULL) ===f exid);
//      expect(fdja_ls(v, "nid", NULL) ===f "0");
//      expect(fdja_ls(v, "payload.hello", NULL) ===f "world");
//      expect(fdja_ls(v, "payload.args.color", NULL) ===f "blue");
//      fdja_free(v);
//
//      //puts(fdja_f_todc("var/run/%s/run.json", fep));
//      v = fdja_parse_f("var/run/%s/run.json", fep);
//
//      expect(v != NULL);
//      expect(fdja_ls(v, "exid", NULL) ===f exid);
//
//      expect(fdja_to_json(fdja_l(v, "nodes.0.tree")) ===f ""
//        "[\"invoke\",{\"_0\":\"stamp\",\"color\":\"blue\"},[]]");
//
//      fdja_free(v);
//    }

    it "schedules a cron msg"
  }
}

