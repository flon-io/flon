
//
// specifying flon-dispatcher
//
// Fri Nov 28 10:47:15 JST 2014
//

#include "flutil.h"
#include "flutim.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_dispatcher.h"
#include "feu_helpers.h"


context "flon-dispatcher and schedules:"
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
    //hlp_reset_tst('t');

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

      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

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

      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

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

    it "unstores at schedules"
    {
      exid = flon_generate_exid("dtest.unsch.at");
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
          "msg: { point: execute, exid: \"%s\", nid: 0_2_0 }\n"
        "}", exid, exid
      );
      expect(r i== 1);
      //
      r = flon_dispatch(name);
      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

      r = flu_writeall(
        "var/spool/dis/%s", name,
        "{"
          "point: unschedule\n"
          "at: \"20141128.105313\"\n"
          "exid: %s\n"
          "nid: 0_2\n"
        "}", exid
      );
      expect(r i== 1);

      r = flon_dispatch(name);

      flu_system("tree var/ -I www");

      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

      expect(flu_fstat("var/spool/dis/%s", name) c== 0);
      expect(flu_fstat("var/run/%s/processed/%s", fep, name) c== 'f');

      expect(flu_fstat("var/spool/tdis/%s", fep) c== 0);

      flu_list *at = flon__timer('a');
      flu_list *ct = flon__timer('c');

      expect(at->size zu== 0);
      expect(ct->size zu== 0);
    }

    it "unstores cron schedules"
    {
      exid = flon_generate_exid("dtest.unsch.cron");
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
          "msg: { point: execute, exid: \"%s\", nid: 0_2_0 }\n"
        "}", exid, exid
      );
      expect(r i== 1);
      //
      r = flon_dispatch(name);
      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

      r = flu_writeall(
        "var/spool/dis/%s", name,
        "{"
          "point: unschedule\n"
          "cron: \"* * * * *\"\n"
          "exid: %s\n"
          "nid: 0_2\n"
        "}", exid
      );
      expect(r i== 1);

      r = flon_dispatch(name);

      flu_system("tree var/ -I www");

      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

      expect(flu_fstat("var/spool/dis/%s", name) c== 0);
      expect(flu_fstat("var/run/%s/processed/%s", fep, name) c== 'f');

      expect(flu_fstat("var/spool/tdis/%s", fep) c== 0);

      flu_list *at = flon__timer('a');
      flu_list *ct = flon__timer('c');

      expect(at->size zu== 0);
      expect(ct->size zu== 0);
    }
  }

  describe "flon_load_timers()"
  {
    before each
    {
      flu_empty_dir("var/spool/tdis");

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

  describe "flon_trigger()"
  {
    before each
    {
      hlp_reset_tst('t');

      long long nows = 1417381080; // 20141130.205800 utc
      char *ns = flu_sstamp(nows, 1, 's');

      exid = flon_generate_exid("dtest.trig");
      fep = flon_exid_path(exid);

      flu_mkdir_p("var/run/%s/processed", fep, 0755);
      flu_mkdir_p("var/spool/tdis/%s", fep, 0755);

      char *ts = "20141130.205800";
      flu_writeall(
        "var/spool/tdis/%s/at-%s-%s-0_0.json", fep, ts, exid,
        "{"
          "point: schedule, at: \"%s\"\n"
          "exid: %s, nid: 0_0\n"
          "msg: { point: execute, exid: %s, nid: 0_0_0 }\n"
        "}", ts, exid, exid);

      ts = "20141130.205900";
      flu_writeall(
        "var/spool/tdis/%s/at-%s-%s-0_1.json", fep, ts, exid,
        "{"
          "point: schedule, at: \"%s\"\n"
          "exid: %s, nid: 0_1\n"
          "msg: { point: execute, exid: %s, nid: 0_1_0 }\n"
        "}", ts, exid, exid);

      //flu_system("tree var/spool/tdis");
    }
    after each
    {
      free(ns);
    }

    it "triggers matching timers"
    {
      //flu_system("tree var/spool/dis");

      flon_load_timers();

      expect(hlp_count_jsons("var/spool/dis") zu== 0);

      // trigger first timer

      flon_trigger(nows);

      //flu_system("tree var/ -I www");

      fdja_value *v = fdja_parse_f("var/spool/dis/exe_%s-0_0_0.json", exid);
      //flu_putf(fdja_todc(v));

      expect(v != NULL);
      expect(fdja_ls(v, "point", NULL) ===f "execute");
      expect(fdja_ls(v, "trigger.now", NULL) ===f ns);
      expect(fdja_ls(v, "trigger.ts", NULL) ===f ns);

      char *fn = flu_sprintf("%s/at-%s-%s-0_0.json", fep, ns, exid);
      expect(fdja_ls(v, "trigger.fn", NULL) ===F fn);

      fdja_free(v);

      v = fdja_parse_f("var/run/%s/processed/at-%s-%s-0_0.json", fep, ns, exid);
      //flu_putf(fdja_todc(v));

      expect(v != NULL);
      expect(fdja_ls(v, "point", NULL) ===f "schedule");

      fdja_free(v);

      //flu_system("tree var/ -I www");

      expect(flu_fstat("var/run/%s/at-%s-%s-0_0.json", fep, ns, exid) == 0);

      expect(flon__timer('a')->size zu== 1);
      expect(flon__timer('c')->size zu== 0);

      // trigger second timer

      nows += 60; // 20141130.205900 utc
      free(ns); ns = flu_sstamp(nows, 1, 's');

      flon_trigger(nows);

      expect(flu_fstat("var/spool/dis/exe_%s-0_1_0.json", exid) c== 'f');

      //flu_system("tree var/ -I www");

      expect(flu_fstat("var/spool/tdis/%s", fep) c== 0);
    }
  }
}

