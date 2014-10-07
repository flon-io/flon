
//
// specifying flon-dispatcher
//
// Mon Oct  6 16:13:47 JST 2014
//

#include "flutil.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


context "flon-dispatcher"
{
  before each
  {
    flon_configure_j(fdja_c(
      "dispatcher: {\n"
      "  bindir: ../bin/\n"
      "}\n"
    ));
  }

  describe "flon_dispatch_j()"
  {
    it "dispatches invocations"
    {
      char *invid = flon_generate_id();
      char *path = flu_sprintf("inv_%s.json", invid);

      fdja_value *j = fdja_c(
        "invocation: [ stamp, {}, [] ]\n"
        "payload: {\n"
          "_invocation_id: %s\n"
          "hello: world\n"
        "}\n"
        "_path: %s\n",
        invid, path
      );

      flon_dispatch_j(j);

      sleep(2);

      char *s = flu_readall("../tst/var/spool/in/inv_%s_ret.json", invid);
      //printf(">>>\n%s<<<\n", s);
      expect(s != NULL);
      expect(strstr(s, ",\"stamp\":\"") != NULL);

      s = flu_readall("../tst/var/log/invocations/%s.txt", invid);
      //printf(">>>\n%s<<<\n", s);
      expect(s != NULL);
      expect(strstr(s, " stamp.rb over.") != NULL);
    }

    it "discards files it doesn't understand"
  }
}

