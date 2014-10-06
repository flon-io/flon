
//
// specifying flon-invoker
//
// Fri Oct  3 11:24:25 JST 2014
//

#include "flutil.h"
#include "fl_common.h"
#include "fl_invoker.h"


context "flon-invoker"
{
  before each
  {
    flon_configure_j(fdja_o(""
      "invoker: {\n"
      "  dir: ../tst/\n"
      "}\n"
    ));
  }

  describe "flon_invoke_j()"
  {
    it "invokes"
    {
      char *invid = flon_generate_id();

      fdja_value *j = fdja_o(""
        "invocation: [ stamp, {}, [] ]\n"
        "payload: {\n"
          "hello: world\n"
        "}\n"
      );
      fdja_pset(j, "payload._invocation_id", fdja_v(invid));

      //puts(fdja_to_json(j));
      flon_invoke_j(j);

      sleep(1);

      char *s = flu_readall("../tst/var/spool/in/inv_%s_ret.json", invid);
      //printf(">>>\n%s<<<\n", s);
      expect(strstr(s, ",\"stamp\":\"") != NULL);

      s = flu_readall("../tst/var/log/invocations/%s.txt", invid);
      //printf(">>>\n%s<<<\n", s);
      expect(strstr(s, " stamp.rb over.") != NULL);
    }
  }
}

