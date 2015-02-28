
//
// specifying flon-dispatcher
//
// Mon Oct  6 16:13:47 JST 2014
//

#include "flutil.h"
#include "flutim.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_common.h"
#include "fl_dispatcher.h"
#include "fl_tools.h"

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
          "task: { state: created, for: stamp }\n"
          "tree: [ task, { _0: stamp }, [] ]\n"
          "exid: %s\n"
          "nid: 0_2\n"
          "payload: {\n"
            "hello: world\n"
          "}\n"
        "}", exid
      );
      expect(r i== 1);

      r = flon_dispatch(name);
      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

      r = hlp_wait_for_file('f', "var/spool/dis/tsk_%s-0_2.json", exid, 2);
      expect(r i== 1); // ret file found

      flu_msleep(210); // give it time to write the file

      fdja_value *v = fdja_parse_f("var/spool/dis/tsk_%s-0_2.json", exid);
      expect(v != NULL);
      //flu_putf(fdja_todc(v));
      expect(fdja_l(v, "stamp", NULL) != NULL);
      fdja_free(v);

      s = flu_readall("var/log/%s/tsk_%s-0_2.log", fep, exid);
      //printf(">>>\n%s<<<\n", s);
      expect(s != NULL);
      expect(s >== " ran >ruby stamp.rb<");
      expect(s >== " stamp.rb over.");

      expect(flu_fstat("var/spool/dis/%s", name) == 'f');

      flu_unlink("var/spool/dis/tsk_%s-0_2.json", exid);
      flu_unlink("var/spool/tsk/tsk_%s-0_2.json", exid);
      flu_unlink("var/log/tsk/%s-0_2.txt", exid);
    }

    it "rejects files it doesn't understand"
    {
      exid = flon_generate_exid("dtest.rju");
      name = flu_sprintf("tsk_%s.json", exid);
      path = flu_sprintf("var/spool/dis/%s", name);

      int r = flu_writeall(path, "NADA");
      expect(r i== 1);

      r = flon_dispatch(name);
      expect(r i== -1); // -1 rejected / 1 seen, failed / 2 dispatched

      s = flu_readall("var/spool/rejected/tsk_%s.json", exid);
      expect(s >== "NADA");
      expect(s >== "# reason:");

      flu_unlink("var/spool/rejected/tsk_%s.json", exid);
    }

    it "rejects files it doesn't know how to dispatch"
    {
      exid = flon_generate_exid("dtest.rjk");
      name = flu_sprintf("nada_%s.json", exid);
      path = flu_sprintf("var/spool/dis/%s", name);

      int r = flu_writeall(path, "{ na: da }", exid);
      expect(r i== 1);

      r = flon_dispatch(name);
      expect(r i== -1); // -1 rejected / 1 seen, failed / 2 dispatched

      s = flu_readall("var/spool/rejected/nada_%s.json", exid);
      expect(s ^== "{ na: da }\n# reason: unknown file prefix (2");

      flu_unlink("var/spool/rejected/tsk_%s.json", exid);
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
          "state: created\n"
          "taskee: stamp\n"
          "tree: [ task, { _0: stamp }, [] ]\n"
          "exid: %s\n"
          "nid: 0_7-f\n"
          "payload: {\n"
            "hello: dtest.rir\n"
          "}\n"
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

    it "doesn't launch a new executor if the previous is still here"
    {
      int r;
      exid = flon_generate_exid("d_test.xid");
      fep = flon_exid_path(exid);
      name = flu_sprintf("exe_%s.json", exid);

      r = flu_writeall(
        "var/spool/dis/exe_%s.json", exid,
        "{"
          "point: execute\n"
          "tree:"
            "[ sequence {} [ [ sequence {} [ [ trace { _0: a } [] ] ] ] ] ]\n"
          "exid: %s\n"
          "payload: {\n"
            "hello: d_test.xid\n"
          "}\n"
        "}", exid
      );
      expect(r i== 1);

      r = flu_writeall("var/run/%s.pid", exid, "%i", getpid());
      expect(r i== 1);
        //
        // pass the current pid (existing process to prevent executor from
        // being forked...

      r = flon_dispatch(name);
      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

      //printf("var/run/%s\n", fep);
      expect(flu_fstat("var/run/%s", fep) == 0);
      expect(flu_fstat("var/archive/%s", fep) == 0);
        // execution never ran
    }
  }
}

