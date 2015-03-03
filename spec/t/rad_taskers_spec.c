
//
// specifying flon-tasker
//
// Tue Mar  3 06:36:16 JST 2015
//

#include "flutil.h"
#include "flutim.h"
#include "gajeta.h"
#include "djan.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_tasker.h"

#include "flon_helpers.h"


context "flon-tasker"
{
  before each
  {
    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");

    char *s = NULL;
    fdja_value *v = NULL;

    char *exid = NULL;
    char *path = NULL;
  }
  after each
  {
    free(s);
    fdja_free(v);

    free(exid);
    free(path);
  }

  context "with .rad taskers"
  {
    it "tasks"
    {
      exid = flon_generate_exid("ttest");
      char *nid = "0_7";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "task: { state: created, for: fahrrad, from: executor }\n"
        "tree: [ task, { _0: fahrrad }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: { en: bicycle }\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r i== 0);

      r = hlp_wait_for_file('f', "var/spool/dis/tsk_%s-%s.json", exid, nid, 3);

      v = fdja_parse_f("var/spool/dis/tsk_%s-%s.json", exid, nid);
      //fdja_putdc(v);

      expect(fdja_ld(v, "task") ===f ""
        "{ "
          "state: completed, "
          "for: fahrrad, "
          "from: usr/local/tsk/any/fahrrad/fahr.rad, "
          "event: completion "
        "}");
      expect(fdja_ld(v, "payload") ===f ""
        "{ en: bicycle, ret: rad, fahr: rad }");
    }

    it "prevents 'task' use in the .rad"
  }
}

