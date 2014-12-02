
//
// specifying flon-dispatcher
//
// Mon Oct  6 16:13:47 JST 2014
//

#include "flutil.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_paths.h"
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
    it "dispatches invocations"
    {
      exid = flon_generate_exid("dtest.inv");
      fep = flon_exid_path(exid);
      name = flu_sprintf("inv_%s-0_2.json", exid);

      int r = flu_writeall(
        "var/spool/dis/inv_%s-0_2.json", exid,
        "{"
          "point: invoke\n"
          "tree: [ stamp, {}, [] ]\n"
          "exid: %s\n"
          "nid: 0_2\n"
          "payload: {\n"
            "hello: world\n"
          "}\n"
        "}", exid
      );
      expect(r i== 1);

      r = flon_dispatch(name);
      expect(r i== 2);

      sleep(1);

      fdja_value *v = fdja_parse_f("var/spool/dis/ret_%s-0_2.json", exid);
      expect(v != NULL);
      //flu_putf(fdja_todc(v));
      expect(fdja_l(v, "stamp", NULL) != NULL);
      fdja_free(v);

      s = flu_readall("var/log/%s/inv_%s-0_2.log", fep, exid);
      //printf(">>>\n%s<<<\n", s);
      expect(s != NULL);
      expect(s >== " invoked >ruby stamp.rb<");
      expect(s >== " stamp.rb over.");

      expect(flu_fstat("var/spool/dis/%s", name) == 0);

      flu_unlink("var/spool/dis/ret_%s-0_2.json", exid);
      flu_unlink("var/spool/inv/inv_%s-0_2.json", exid);
      flu_unlink("var/log/inv/%s-0_2.txt", exid);
    }

    it "rejects files it doesn't understand"
    {
      exid = flon_generate_exid("dtest.rju");
      name = flu_sprintf("inv_%s.json", exid);
      path = flu_sprintf("var/spool/dis/%s", name);

      int r = flu_writeall(path, "NADA");
      expect(r i== 1);

      r = flon_dispatch(name);
      expect(r i== -1);

      s = flu_readall("var/spool/rejected/inv_%s.json", exid);
      expect(s >== "NADA");
      expect(s >== "# reason:");

      flu_unlink("var/spool/rejected/inv_%s.json", exid);
    }

    it "rejects files it doesn't know how to dispatch"
    {
      exid = flon_generate_exid("dtest.rjk");
      name = flu_sprintf("inv_%s.json", exid);
      path = flu_sprintf("var/spool/dis/%s", name);

      int r = flu_writeall(
        path,
        "{"
          "nada: [ stamp, {}, [] ]\n"
          "id: %s\n"
          "payload: {\n"
            "hello: world\n"
          "}\n"
        "}", exid
      );
      expect(r i== 1);

      r = flon_dispatch(name);
      expect(r i== -1);

      s = flu_readall("var/spool/rejected/inv_%s.json", exid);
      expect(s ^== "{nada: [ stamp");

      flu_unlink("var/spool/rejected/inv_%s.json", exid);
    }

    it "receives invocation returns"
    {
      int r = -1;
      exid = flon_generate_exid("dtest.rir");
      fep = flon_exid_path(exid);
      name = flu_sprintf("ret_%s-0_7-f.json", exid);

      r = flu_writeall(
        "var/spool/inv/inv_%s-0_7-f.json", exid,
        "{"
          "point: invoke\n"
          "tree: [ stamp, {}, [] ]\n"
          "exid: %s\n"
          "nid: 0_7-f\n"
          "payload: {\n"
            "hello: dtest.rir\n"
          "}\n"
        "}", exid
      );
      expect(r i== 1);

      r = flu_writeall(
        "var/spool/dis/ret_%s-0_7-f.json", exid,
        "{"
          "hello: dtest.rir\n"
        "}"
      );
      expect(r i== 1);

      // dispatch for the ret_

      r = flon_dispatch(name);
      expect(r i== 2);

      sleep(1);

      expect(flu_fstat("var/spool/inv/inv_%s-0_7-f.json", exid) == 0);
      expect(flu_fstat("var/spool/dis/ret_%s-0_7-f.json", exid) == 0);

      expect(flu_fstat("var/spool/dis/rcv_%s-0_7-f.json", exid) == 'f');

      // dispatch for the rcv_

      free(name);
      name = flu_sprintf("rcv_%s-0_7-f.json", exid);

      r = flon_dispatch(name);
      expect(r i== 2);

      sleep(1);

      s = flu_readall("var/archive/%s/exe.log", fep);
      //printf("exe.log >>>\n%s\n<<<\n", s);
      expect(s != NULL);
      expect(s >== "reject node not found, ");

      // check that rcv_ got rejected (no execution going on)

      expect(flu_fstat("var/spool/rejected/rcv_%s-0_7-f.json", exid) == 'f');
    }

//    it "writes down the executor pid in var/run/{exid}.pid"
//    {
//      int r;
//      exid = flon_generate_exid("d_test.pid");
//      name = flu_sprintf("exe_%s.json", exid);
//
//      r = flu_writeall(
//        "var/spool/dis/exe_%s.json", exid,
//        "{"
//          "point: execute\n"
//          "tree:"
//            "[ sequence {} [ [ sequence {} [ [ trace { _0: a } [] ] ] ] ] ]\n"
//          "exid: %s\n"
//          "payload: {\n"
//            "hello: d_test.pid\n"
//          "}\n"
//        "}", exid
//      );
//      expect(r i== 1);
//
//      r = flon_dispatch(name);
//      expect(r i== 0);
//
//      // too fast, already gone...
//    }

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

      flu_writeall("var/run/%s.pid", exid, "%i", getpid());
      //flu_system("cat var/run/%s.pid", exid);
        //
        // pass the current pid (existing process to prevent executor from
        // being forked...

      r = flon_dispatch(name);
      expect(r i== 2);

      //printf("var/run/%s\n", fep);
      expect(flu_fstat("var/run/%s", fep) == 0);
      expect(flu_fstat("var/archive/%s", fep) == 0);
        // execution never ran
    }
  }
}

