
//
// specifying flon-dispatcher
//
// Mon Oct  6 16:13:47 JST 2014
//

#include "fl_common.h"
#include "fl_dispatcher.h"


context "flon-dispatcher"
{
  before each
  {
    flon_configure_j(fdja_o(""
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

      fdja_value *j = fdja_o(""
        "invocation: [ stamp, {}, [] ]\n"
        "payload: {\n"
          "hello: world\n"
        "}\n"
      );
      fdja_pset(j, "payload._invocation_id", fdja_v(invid));

      flon_dispatch_j("./inv_xxx.json", j);

      //sleep(2);
    }

    it "discards files it doesn't understand"
  }
}

