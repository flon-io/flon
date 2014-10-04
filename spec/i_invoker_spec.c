
//
// specifying flon-invoker
//
// Fri Oct  3 11:24:25 JST 2014
//

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
        "invocation:"
          "[ stamp, {}, [] ]\n"
        "task: {\n"
          "hello: world\n"
        "}\n"
      ));
    }
  }
}

