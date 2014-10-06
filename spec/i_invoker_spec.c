
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
      flon_invoke_j(fdja_o(""
        "invocation: [ stamp, {}, [] ]\n"
        "payload: {\n"
          "_invocation_id: 123456\n"
          "hello: world\n"
        "}\n"
      ));

      sleep(1);

      char *s = flu_readall("../tst/var/spool/in/inv_123456_ret.json");
      //printf(">>>\n%s<<<\n", s);
      expect(strstr(s, ",\"stamp\":\"") != NULL);

      s = flu_readall("../tst/var/log/invocations/123456.txt");
      //printf(">>>\n%s<<<\n", s);
      expect(strstr(s, " stamp.rb over.") != NULL);
    }
  }
}

