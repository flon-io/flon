
//
// specifying flon-invoker
//
// Fri Oct  3 11:24:25 JST 2014
//

#include "fl_invoker.h"


context "flon-invoker"
{
  describe "flon_invoke_j()"
  {
    it "invokes"
    {
      flon_invoke_j(fdja_o(""
        "line:"
          "[ stamp, {}, [] ]\n"
        "task: {\n"
          "hello: world\n"
        "}\n"
      ));

      pending("2fork of stamp/invoke.sh");
    }
  }
}

