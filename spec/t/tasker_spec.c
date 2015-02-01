
//
// specifying flon-tasker
//
// Fri Oct  3 11:24:25 JST 2014
//

#include "flutil.h"
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
  }

  describe "flon_task()"
  {
    it "tasks"
    {
      char *exid = flon_generate_exid("ttest");
      char *nid = "0_0_7";
      char *path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "tree: [ task, { _0: stamp }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r == 0);

      r = hlp_wait_for_file('f', "var/spool/dis/ret_%s-%s.json", exid, nid, 3);
      expect(r i== 1);
      //
      flu_msleep(300);

      expect(flu_canopath(".") $==f "/tst/");

      expect(flu_fstat("var/spool/tsk/tsk_%s-%s.json", exid, nid) c== 'f');
        // it's still here, it's the dispatcher's work to nuke it

      expect(flu_fstat("var/spool/dis/ret_%s-%s.json", exid, nid) c== 'f');

      fdja_value *v = fdja_parse_f("var/spool/dis/ret_%s-%s.json", exid, nid);
      //fdja_putdc(v);

      expect(fdja_ls(v, "hello", NULL) ===f "world");
      expect(fdja_l(v, "stamp") != NULL);

      fdja_free(v);

      flu_unlink("var/spool/tsk/tsk_%s-%s.json", exid, nid);
      flu_unlink("var/spool/dis/ret_%s-%s.json", exid, nid);

      free(exid);
      free(path);
    }
  }
}

