
//
// specifying flon-dispatcher
//
// Wed Mar  4 16:21:02 JST 2015
//

#include "flutil.h"
#include "flutim.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_common.h"
#include "fl_dispatcher.h"

#include "flon_helpers.h"


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
    it "dispatches tasks"
    {
      exid = flon_generate_exid("dtest.tsk");
      fep = flon_exid_path(exid);
      name = flu_sprintf("tsk_%s-0_2.json", exid);

      int r = flu_writeall(
        "var/spool/dis/tsk_%s-0_2.json", exid,
        "{"
          "point: task\n"
          "task: { state: created, for: stamp, from: executor }\n"
          "tree: [ task, { _0: stamp }, [] ]\n"
          "exid: %s\n"
          "nid: 0_2\n"
          "payload: { hello: world }\n"
        "}", exid
      );
      expect(r i== 1);

      r = flon_dispatch(name);
      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

      r = hlp_wait_for_file('f', "var/spool/dis/tsk_%s-0_2.json", exid, 2);
      expect(r i== 1); // ret file found

      flu_msleep(210); // give it time to write the file

      char rf = flu_fstat("var/spool/rejected/rcv_dtest.tsk_%s-0_2.json", exid);
      expect(rf c== 0);
        // if not, check that there isn't a straggling dispatcher still
        // executing

      fdja_value *v = fdja_parse_f("var/spool/dis/tsk_%s-0_2.json", exid);
      expect(v != NULL);
      //flu_putf(fdja_todc(v));
      expect(fdja_l(v, "stamp", NULL) != NULL);
      fdja_free(v);

      s = flu_readall("var/archive/%s/taskers/0_2-stamp.log", fep);
        // since it's not really running, it logs to var/archive/...

      //printf(">>>\n%s<<<\n", s);

      expect(s != NULL);
      expect(s >== " ran >ruby stamp.rb<");
      expect(s >== " stamp.rb over.");

      expect(flu_fstat("var/spool/dis/%s", name) == 'f');

      flu_unlink("var/spool/dis/tsk_%s-0_2.json", exid);
      flu_unlink("var/spool/tsk/tsk_%s-0_2.json", exid);
      flu_unlink("var/log/tsk/%s-0_2.txt", exid);
    }

    it "receives task returns"
    {
      int r = -1;
      exid = flon_generate_exid("dtest.rir");
      fep = flon_exid_path(exid);
      name = flu_sprintf("tsk_%s-0_7-f.json", exid);

      r = flu_writeall(
        "var/spool/tsk/%s", name,
        "{"
          "point: task\n"
          "task: { state: created, for: stamp, from: executor }\n"
          "tree: [ task, { _0: stamp }, [] ]\n"
          "exid: %s\n"
          "nid: 0_7-f\n"
          "payload: { hello: dtest.rir }\n"
        "}", exid
      );
      expect(r i== 1);

      r = flu_writeall(
        "var/spool/dis/%s", name,
        "{"
          "hello: dtest.rir\n"
        "}"
      );
      expect(r i== 1);

      // dispatch for the ret_

      r = flon_dispatch(name);
      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

      r = hlp_wait_for_file('f', "var/spool/dis/rcv_%s-0_7-f.json", exid, 3);
      expect(r i== 1); // rcv_ file found

      expect(flu_fstat("var/spool/tsk/%s", name) == 0);

      // dispatch for the received tsk_

      free(name); name = flu_sprintf("rcv_%s-0_7-f.json", exid);

      r = flon_dispatch(name);
      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

      r = hlp_wait_for_file('f', "var/archive/%s/exe.log", fep, 2);
      expect(r i== 1); // exe.log found

      s = flu_readall("var/archive/%s/exe.log", fep);
      //printf("exe.log >>>\n%s\n<<<\n", s);
      expect(s >== ": node not found");

      // check that the received task got rejected (no execution going on)

      expect(flu_fstat("var/spool/rejected/rcv_%s-0_7-f.json", exid) == 'f');
    }
  }
}

