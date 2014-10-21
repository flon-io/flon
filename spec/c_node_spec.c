
//
// specifying fl_node.[ch]
//
// Mon Oct 20 16:58:10 JST 2014
//

//#include <stdio.h>

//#include "flutil.h"
#include "fl_node.h"


context "fl_node"
{
  describe "flon_node_tree()"
  {
    before each
    {
      fdja_value *exe = fdja_c(
        "exid: xtest.pn-u0-20141020.0754.chipeduzuba\n"
        "nodes: {\n"
          "0: { tree:\n"
          "  [ sequence, {}, [\n"
          "    [ invoke, { _0: stamp, color: red }, [] ]\n"
          "    [ concurrence, {}, [\n"
          "      [ invoke, { _0: stamp, color: green }, [] ]\n"
          "      [ invoke, { _0: stamp, color: blue }, [] ]\n"
          "    ] ]\n"
          "  ] ]\n"
          "}\n"
          "0_0: {}\n"
          "0_1: {}\n"
          "0_1_0: {}\n"
        "}");
    }
    after each
    {
      if (exe) fdja_free(exe);
    }

    it "looks up the root tree"
    {
      fdja_value *t = flon_node_tree(exe, "0");

      expect(t != NULL);

      expect(fdja_to_json(t) ^==f ""
        "[\"sequence\",{},[");
    }

    it "looks up a sub tree"
    {
      fdja_value *t = flon_node_tree(exe, "0_1_0");

      expect(t != NULL);

      expect(fdja_to_json(t) ^==f ""
        "[\"invoke\",{\"_0\":\"stamp\",\"color\":\"green\"},[]]");
    }
  }
}

