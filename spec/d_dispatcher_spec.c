
//
// specifying flon-dispatcher
//
// Mon Oct  6 16:13:47 JST 2014
//

#include "flutil.h"
#include "gajeta.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


context "flon-dispatcher"
{
  before each
  {
    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->params = stderr;

    flon_configure("../tst");

    char *id = NULL;
    char *path = NULL;
    char *s = NULL;
  }
  after each
  {
    if (id) free(id);
    if (path) free(path);
    if (s) free(s);
  }

  describe "flon_dispatch()"
  {
    it "dispatches invocations"
    {
      id = flon_generate_id();
      path = flu_sprintf("../tst/var/spool/in/inv_%s.json", id);

      int r = flu_writeall(
        path,
        "{"
          "invocation: [ stamp, {}, [] ]\n"
          "payload: {\n"
            "_invocation_id: %s\n"
            "hello: world\n"
          "}\n"
        "}", id
      );
      expect(r == 1);

      r = flon_dispatch(path);
      expect(r == 1);

      sleep(2);

      s = flu_readall("../tst/var/spool/in/inv_%s_ret.json", id);
      //printf(">>>\n%s<<<\n", s);
      expect(s != NULL);
      expect(strstr(s, ",\"stamp\":\"") != NULL);

      s = flu_readall("../tst/var/log/invocations/%s.txt", id);
      //printf(">>>\n%s<<<\n", s);
      expect(s != NULL);
      expect(strstr(s, " stamp.rb over.") != NULL);

      flu_unlink("../tst/var/spool/in/inv_%s_ret.json", id);
      flu_unlink("../tst/var/log/invocations/%s.txt", id);
    }

    it "rejects files it doesn't understand"
    {
      id = flon_generate_id();
      path = flu_sprintf("../tst/var/spool/in/inv_%s.json", id);

      int r = flu_writeall(path, "NADA");
      expect(r == 1);

      flon_dispatch(path);

      s = flu_readall("../tst/var/spool/rejected/inv_%s.json", id);
      expect(s === "NADA");

      flu_unlink("../tst/var/spool/rejected/inv_%s.json", id);
    }
  }
}

