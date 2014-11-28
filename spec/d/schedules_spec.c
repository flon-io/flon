
//
// specifying flon-dispatcher
//
// Fri Nov 28 10:47:15 JST 2014
//

#include "flutil.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


context "flon-dispatcher"
{
  before each
  {
    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");

    char *exid = NULL;
    char *fep = NULL;
    char *nid = NULL;
    char *name = NULL;
    char *path = NULL;
    char *s = NULL;
  }
  after each
  {
    free(exid);
    free(fep);
    free(nid);
    free(name);
    free(path);
    free(s);
  }

  describe "flon_dispatch()"
  {
    it "stores at schedules"
    {
      exid = flon_generate_exid("dtest.sch.one");
      fep = flon_exid_path(exid);
      name = flu_sprintf("sch_%s-0_2.json", exid);

      int r = flu_mkdir_p("var/run/%s/processed", fep, 0755);
      expect(r i== 0);

      r = flu_writeall(
        "var/spool/dis/%s", name,
        "{"
          "point: schedule\n"
          "at: \"20141128.105313\"\n"
          "exid: %s\n"
          "nid: 0_2\n"
          "msg: {\n"
            "point: receive\n"
            "exid: %s\n"
            "nid: 0_2\n"
          "}\n"
        "}", exid, exid
      );
      expect(r i== 1);

      r = flon_dispatch(name);

      flu_system("tree var/ -I www");

      expect(r i== 2);

      expect(flu_fstat("var/spool/dis/%s", name) c== 0);
      expect(flu_fstat("var/run/%s/processed/%s", fep, name) c== 'f');

      expect(flu_fstat("var/spool/tdis/%s", fep) c== 'd');
    }

    it "stores cron schedules"
    {
      exid = flon_generate_exid("dtest.sch.one");
      fep = flon_exid_path(exid);
      name = flu_sprintf("sch_%s-0_2.json", exid);

      int r = flu_mkdir_p("var/run/%s/processed", fep, 0755);
      expect(r i== 0);

      r = flu_writeall(
        "var/spool/dis/%s", name,
        "{"
          "point: schedule\n"
          "cron: \"* * * * *\"\n"
          "exid: %s\n"
          "nid: 0_2\n"
          "msg: {\n"
            "point: execute\n"
            "exid: %s\n"
            "nid: 0_2_0\n"
          "}\n"
        "}", exid, exid
      );
      expect(r i== 1);

      r = flon_dispatch(name);

      flu_system("tree var/ -I www");

      expect(r i== 2);

      expect(flu_fstat("var/spool/dis/%s", name) c== 0);
      expect(flu_fstat("var/run/%s/processed/%s", fep, name) c== 'f');

      expect(flu_fstat("var/spool/tdis/%s", fep) c== 'd');
    }

    it "rejects schedules missing 'at' or 'cron'"
    it "rejects schedules with an invalid at time"
    it "rejects schedules with an invalid cron string"
  }
}

