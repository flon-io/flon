
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

    flon_empty_timers();

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

      //flu_system("tree var/ -I www");

      expect(r i== 2);

      expect(flu_fstat("var/spool/dis/%s", name) c== 0);
      expect(flu_fstat("var/run/%s/processed/%s", fep, name) c== 'f');

      expect(flu_fstat("var/spool/tdis/%s", fep) c== 'd');

      char *fn = flu_pline("ls var/spool/tdis/%s", fep);
      expect(fn ^== "at-20141128.105313-dtest.");

      fdja_value *v = fdja_parse_f("var/spool/tdis/%s/%s", fep, fn);
      //flu_putf(fdja_todc(v));

      expect(fdja_ls(v, "point", NULL) ===f "schedule");
      expect(fdja_ls(v, "at", NULL) ===f "20141128.105313");

      fdja_free(v);
      free(fn);

      flu_list *at = flon__timer('a');
      flu_list *ct = flon__timer('c');

      expect(at->size zu== 1);
      expect(ct->size zu== 0);
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

      //flu_system("tree var/ -I www");

      expect(r i== 2);

      expect(flu_fstat("var/spool/dis/%s", name) c== 0);
      expect(flu_fstat("var/run/%s/processed/%s", fep, name) c== 'f');

      char *fn = flu_pline("ls var/spool/tdis/%s", fep);
      expect(fn ^== "cron-KiAqICogKiAq-dtest.");

      fdja_value *v = fdja_parse_f("var/spool/tdis/%s/%s", fep, fn);
      //flu_putf(fdja_todc(v));

      expect(fdja_ls(v, "point", NULL) ===f "schedule");
      expect(fdja_ls(v, "cron", NULL) ===f "* * * * *");

      fdja_free(v);
      free(fn);

      flu_list *at = flon__timer('a');
      flu_list *ct = flon__timer('c');

      expect(at->size zu== 0);
      expect(ct->size zu== 1);
    }

    it "rejects schedules missing 'at' or 'cron'"
    it "rejects schedules with an invalid at time"
    it "rejects schedules with an invalid cron string"
  }

  describe "flon_load_timers()"
  {
    before each
    {
      flu_system("rm -fR var/spool/tdis");

      for (size_t i = 0; i < 6; ++i)
      {
        char *dom = flu_sprintf("dtest.flt%i", i % 2);
        char *exid = flon_generate_exid(dom);
        char *fep = flon_exid_path(exid);

        flu_mkdir_p(
          "var/spool/tdis/%s", fep, 0755);
        flu_writeall(
          "var/spool/tdis/%s/cron-KiAqICogKiAq-%s-0_2.json", fep, exid,
          "{"
            "point: schedule, cron: \"* * * * *\"\n"
            "exid: %s, nid: 0_2\n"
            "msg: { point: execute, exid: %s, nid: 0_2_0 }\n"
          "}", exid, exid);
        flu_writeall(
          "var/spool/tdis/%s/at-20141130.065728-%s-0_2.json", fep, exid,
          "{"
            "point: schedule, at: 20141130.065728\n"
            "exid: %s, nid: 0_2\n"
            "msg: { point: execute, exid: %s, nid: 0_2_0 }\n"
          "}", exid, exid);

        free(dom); free(exid); free(fep);
      }
      //flu_system("tree var/spool/tdis");
    }

    it "scans var/spool/tdis and loads timers"
    {
      flon_load_timers();

      flu_list *at = flon__timer('a');
      flu_list *ct = flon__timer('c');

      expect(at->size zu== 6);
      expect(ct->size zu== 6);

      expect(((flon_timer *)at->first->item)->ts === "20141130.065728");
      expect(((flon_timer *)at->last->item)->ts === "20141130.065728");

      expect(((flon_timer *)ct->first->item)->ts === "* * * * *");
      expect(((flon_timer *)ct->last->item)->ts === "* * * * *");
    }
  }
}

