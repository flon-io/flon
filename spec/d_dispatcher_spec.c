
//
// specifying flon-dispatcher
//
// Mon Oct  6 16:13:47 JST 2014
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
    char *nid = NULL;
    char *name = NULL;
    char *path = NULL;
    char *s = NULL;
  }
  after each
  {
    if (exid) free(exid);
    if (nid) free(nid);
    if (name) free(name);
    if (path) free(path);
    if (s) free(s);
  }

  describe "flon_dispatch()"
  {
    it "dispatches invocations"
    {
      exid = flon_generate_exid("dtest.inv");
      name = flu_sprintf("inv_%s-0_2.json", exid);

      int r = flu_writeall(
        "var/spool/dis/inv_%s-0_2.json", exid,
        "{"
          "invoke: [ stamp, {}, [] ]\n"
          "exid: %s\n"
          "nid: 0_2\n"
          "payload: {\n"
            "hello: world\n"
          "}\n"
        "}", exid
      );
      expect(r == 1);

      r = flon_dispatch(name);
      expect(r == 0);

      sleep(1);

      fdja_value *v = fdja_parse_f("var/spool/dis/ret_%s-0_2.json", exid);
      expect(v != NULL);
      //puts(fdja_to_pretty_djan(v));
      expect(fdja_l(v, "stamp", NULL) != NULL);
      fdja_free(v);

      s = flu_readall("var/log/inv/%s-0_2.txt", exid);
      //printf(">>>\n%s<<<\n", s);
      expect(s != NULL);
      expect(strstr(s, " invoked >ruby stamp.rb<") != NULL);
      expect(strstr(s, " stamp.rb over.") != NULL);

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
      expect(r == 1);

      r = flon_dispatch(name);
      expect(r == 1);

      s = flu_readall("var/spool/rejected/inv_%s.json", exid);
      expect(s === "NADA");

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
      expect(r i== 1);

      s = flu_readall("var/spool/rejected/inv_%s.json", exid);
      expect(s ^== "{nada: [ stamp");

      flu_unlink("var/spool/rejected/inv_%s.json", exid);
    }

    it "receives invocation returns"
    {
      int r = -1;
      exid = flon_generate_exid("dtest.rir");
      name = flu_sprintf("ret_%s-0_7-f.json", exid);

      r = flu_writeall(
        "var/spool/inv/inv_%s-0_7-f.json", exid,
        "{"
          "invoke: [ stamp, {}, [] ]\n"
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
          "return: true\n"
        "}"
      );
      expect(r i== 1);

      // dispatch for the ret_

      r = flon_dispatch(name);
      expect(r i== 0);

      sleep(1);

      expect(flu_fstat("var/spool/inv/inv_%s-0_7-f.json", exid) == 0);
      expect(flu_fstat("var/spool/dis/ret_%s-0_7-f.json", exid) == 0);

      expect(flu_fstat("var/spool/dis/rcv_%s-0_7-f.json", exid) == 'f');

      // dispatch for the rcv_

      free(name);
      name = flu_sprintf("rcv_%s-0_7-f.json", exid);

      r = flon_dispatch(name);
      expect(r i== 0);

      sleep(1);

      expect(flu_fstat("var/log/exe/%s.txt", exid) == 'f');

      // TODO
    }
  }
}

