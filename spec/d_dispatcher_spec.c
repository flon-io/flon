
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

    char *id = NULL;
    char *name = NULL;
    char *path = NULL;
    char *s = NULL;
  }
  after each
  {
    if (id) free(id);
    if (name) free(name);
    if (path) free(path);
    if (s) free(s);
  }

  describe "flon_dispatch()"
  {
    it "dispatches invocations"
    {
      id = flon_generate_exid("dtest.inv");
      name = flu_sprintf("inv_%s.json", id);
      path = flu_sprintf("var/spool/dis/%s", name);

      int r = flu_writeall(
        path,
        "{"
          "invoke: [ stamp, {}, [] ]\n"
          "id: %s\n"
          "payload: {\n"
            "hello: world\n"
          "}\n"
        "}",
        id
      );
      expect(r == 1);

      r = flon_dispatch(name);
      expect(r == 0);

      sleep(1);

      s = flu_readall("var/spool/dis/ret_%s.json", id);
      //printf(">>>\n%s<<<\n", s);
      expect(s != NULL);
      expect(strstr(s, ",\"stamp\":\"") != NULL);
      free(s);

      s = flu_readall("var/log/inv/inv_%s.txt", id);
      //printf(">>>\n%s<<<\n", s);
      expect(s != NULL);
      expect(strstr(s, " invoked >ruby stamp.rb<") != NULL);
      expect(strstr(s, " stamp.rb over.") != NULL);

      expect(flu_fstat("var/spool/dis_%s.json", id) == 0);

      flu_unlink("var/spool/dis/inv_%s_ret.json", id);
      flu_unlink("var/log/inv/%s.txt", id);
    }

    it "rejects files it doesn't understand"
    {
      id = flon_generate_exid("dtest.rju");
      name = flu_sprintf("inv_%s.json", id);
      path = flu_sprintf("var/spool/dis/%s", name);

      int r = flu_writeall(path, "NADA");
      expect(r == 1);

      r = flon_dispatch(name);
      expect(r == 1);

      s = flu_readall("var/spool/rejected/inv_%s.json", id);
      expect(s === "NADA");

      flu_unlink("var/spool/rejected/inv_%s.json", id);
    }

    it "rejects files it doesn't know how to dispatch"
    {
      id = flon_generate_exid("dtest.rjk");
      name = flu_sprintf("inv_%s.json", id);
      path = flu_sprintf("var/spool/dis/%s", name);

      int r = flu_writeall(
        path,
        "{"
          "nada: [ stamp, {}, [] ]\n"
          "id: %s\n"
          "payload: {\n"
            "hello: world\n"
          "}\n"
        "}",
        id
      );
      expect(r == 1);

      r = flon_dispatch(name);
      expect(r i== 1);

      s = flu_readall("var/spool/rejected/inv_%s.json", id);
      expect(s ^== "{nada: [ stamp");

      flu_unlink("var/spool/rejected/inv_%s.json", id);
    }
  }
}

